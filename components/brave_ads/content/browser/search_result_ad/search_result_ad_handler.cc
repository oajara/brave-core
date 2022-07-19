/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/content/browser/search_result_ad/search_result_ad_handler.h"

#include <memory>
#include <string>

#include "brave/components/brave_ads/browser/ads_service.h"
#include "brave/components/brave_ads/content/browser/search_result_ad/search_result_ad_service.h"
#include "brave/components/brave_ads/core/browser/search_result_ad/search_result_ad_parsing.h"
#include "content/public/browser/render_frame_host.h"
#include "services/service_manager/public/cpp/interface_provider.h"
#include "url/gurl.h"

namespace brave_ads {

SearchResultAdHandler::AdViewedEventCallbackInfo::AdViewedEventCallbackInfo() = default;
SearchResultAdHandler::AdViewedEventCallbackInfo::AdViewedEventCallbackInfo(
    AdViewedEventCallbackInfo&& info) = default;
SearchResultAdHandler::AdViewedEventCallbackInfo&
SearchResultAdHandler::AdViewedEventCallbackInfo::operator=(
    AdViewedEventCallbackInfo&& info) = default;
SearchResultAdHandler::AdViewedEventCallbackInfo::~AdViewedEventCallbackInfo() = default;

SearchResultAdHandler::SearchResultAdHandler(
    AdsService* ads_service,
    SearchResultAdService* search_result_ad_service)
    : ads_service_(ads_service),
      search_result_ad_service_(search_result_ad_service) {
  DCHECK(ads_service_);
  DCHECK(search_result_ad_service_);
}

SearchResultAdHandler::~SearchResultAdHandler() = default;

void SearchResultAdHandler::MaybeRetrieveSearchResultAd(
    content::RenderFrameHost* render_frame_host,
    bool should_trigger_viewed_event) {
  DCHECK(render_frame_host);

  if (!ads_service_ || !search_result_ad_service_) {
    return;
  }

  search_result_ads_ = SearchResultAdMap();

  const GURL url = render_frame_host->GetLastCommittedURL();
  if (!search_result_ad_service_->ShouldRetrieveSearchResultAd(url)) {
    if (metadata_request_finished_callback_for_testing_) {
      std::move(metadata_request_finished_callback_for_testing_).Run();
    }
    RunAdViewedEventPendingCallbacks(/* ads_fetched */ false);
    return;
  }

  mojo::Remote<blink::mojom::DocumentMetadata> document_metadata;
  render_frame_host->GetRemoteInterfaces()->GetInterface(
      document_metadata.BindNewPipeAndPassReceiver());
  DCHECK(document_metadata.is_bound());
  document_metadata.reset_on_disconnect();

  blink::mojom::DocumentMetadata* raw_document_metadata =
      document_metadata.get();
  raw_document_metadata->GetEntities(
      base::BindOnce(&SearchResultAdHandler::OnRetrieveSearchResultAdEntities,
                     weak_factory_.GetWeakPtr(), std::move(document_metadata),
                     should_trigger_viewed_event));
}

void SearchResultAdHandler::MaybeTriggerSearchResultAdViewedEvent(
    const std::string& creative_instance_id,
    base::OnceCallback<void(bool)> callback) {
  DCHECK(!creative_instance_id.empty());

  // Check if search result ad JSON-LD wasn't processed yet.
  if (!search_result_ads_) {
    // // Check if OnDidFinishNavigation was called for tab_id.
    // if (!base::Contains(ad_viewed_event_pending_callbacks_, tab_id)) {
    //   std::move(callback).Run(/* event_triggered */ false);
    //   return;
    // }

    AdViewedEventCallbackInfo callback_info;
    callback_info.creative_instance_id = creative_instance_id;
    callback_info.callback = std::move(callback);
    ad_viewed_event_pending_callbacks_.push_back(std::move(callback_info));
    return;
  }

  const bool event_triggered =
      QueueSearchResultAdViewedEvent(creative_instance_id);
  std::move(callback).Run(event_triggered);
}

absl::optional<GURL>
SearchResultAdHandler::MaybeTriggerSearchResultAdClickedEvent(
    const std::string& creative_instance_id) {
  DCHECK(!creative_instance_id.empty());

  if (!ads_service_->IsEnabled()) {
    return absl::nullopt;
  }

  if (!search_result_ads_) {
    return absl::nullopt;
  }

  //const SearchResultAdMap& tab_ads = ads_it->second;
  auto it = search_result_ads_->find(creative_instance_id);
  if (it == search_result_ads_->end()) {
    return absl::nullopt;
  }

  if (it->second.state != SearchResultAdState::kReadyForClick) {
    return absl::nullopt;
  }

  const ads::mojom::SearchResultAdPtr& search_result_ad = it->second.ad;
  DCHECK(search_result_ad);
  DCHECK(search_result_ad->target_url.is_valid() &&
         search_result_ad->target_url.SchemeIs(url::kHttpsScheme));

  search_result_ad_service_->TriggerSearchResultAdClickedEvent(
      search_result_ad->Clone());

  return search_result_ad->target_url;
}

void SearchResultAdHandler::OnRetrieveSearchResultAdEntities(
    mojo::Remote<blink::mojom::DocumentMetadata> document_metadata,
    bool should_trigger_viewed_event,
    blink::mojom::WebPagePtr web_page) {
  if (metadata_request_finished_callback_for_testing_) {
    std::move(metadata_request_finished_callback_for_testing_).Run();
  }

  if (!web_page) {
    RunAdViewedEventPendingCallbacks(/* ads_fetched */ false);
    return;
  }

  search_result_ads_ = ParseWebPageEntities(
      std::move(web_page), should_trigger_viewed_event
                               ? SearchResultAdState::kReadyForView
                               : SearchResultAdState::kNotCountView);

  RunAdViewedEventPendingCallbacks(/* ads_fetched */ true);
}

void SearchResultAdHandler::RunAdViewedEventPendingCallbacks(bool ads_fetched) {
  for (auto& callback_info : ad_viewed_event_pending_callbacks_) {
    bool event_triggered = false;
    if (ads_fetched) {
      event_triggered = QueueSearchResultAdViewedEvent(
          callback_info.creative_instance_id);
    }
    if (event_triggered) {
      DVLOG(1) << "Triggered search result ad viewed event for "
               << callback_info.creative_instance_id;
    } else {
      DVLOG(1) << "Failed to trigger search result ad viewed event for "
               << callback_info.creative_instance_id;
    }
    std::move(callback_info.callback).Run(event_triggered);
  }
  ad_viewed_event_pending_callbacks_.clear();
}

bool SearchResultAdHandler::QueueSearchResultAdViewedEvent(
    const std::string& creative_instance_id) {
  DCHECK(!creative_instance_id.empty());
  DCHECK(search_result_ad_service_);
  DCHECK(search_result_ads_);

  auto it = search_result_ads_->find(creative_instance_id);
  if (it == search_result_ads_->end()) {
    return false;
  }

  if (it->second.state == SearchResultAdState::kNotReady ||
      it->second.state == SearchResultAdState::kReadyForClick) {
    return false;
  }

  const ads::mojom::SearchResultAdPtr& search_result_ad = it->second.ad;
  DCHECK(search_result_ad);

  if (it->second.state == SearchResultAdState::kReadyForView) {
    search_result_ad_service_->AddSearchResultAdViewedEventToQueue(
        search_result_ad->Clone());
    search_result_ad_service_->TriggerSearchResultAdViewedEventFromQueue();
  }
  it->second.state = SearchResultAdState::kReadyForClick;

  return true;
}

// void SearchResultAdService::ResetState(SessionID tab_id) {
//   DCHECK(tab_id.is_valid());

//   for (auto& callback_info : ad_viewed_event_pending_callbacks_[tab_id]) {
//     std::move(callback_info.callback).Run(false);
//   }
//   ad_viewed_event_pending_callbacks_.erase(tab_id);
//   search_result_ads_.erase(tab_id);
// }

}  // namespace brave_ads
