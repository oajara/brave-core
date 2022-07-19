/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_ads/ads_tab_helper.h"

#include <memory>
#include <string>

#include "brave/browser/brave_ads/ads_service_factory.h"
#include "brave/browser/brave_ads/search_result_ad/search_result_ad_service_factory.h"
#include "brave/components/brave_ads/content/browser/search_result_ad/search_result_ad_handler.h"
#include "brave/components/brave_ads/content/browser/search_result_ad/search_result_ad_service.h"
#include "brave/components/brave_ads/core/browser/search_result_ad/search_result_ad_parsing.h"
#include "chrome/browser/profiles/profile.h"
#include "components/dom_distiller/content/browser/distiller_javascript_utils.h"
#include "components/dom_distiller/content/browser/distiller_page_web_contents.h"
#include "components/sessions/content/session_tab_helper.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "services/service_manager/public/cpp/interface_provider.h"
#include "ui/base/page_transition_types.h"
#include "ui/base/resource/resource_bundle.h"
#include "url/gurl.h"

#if !BUILDFLAG(IS_ANDROID)
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#endif

namespace brave_ads {

// AdsTabHelper::AdViewedEventCallbackInfo::AdViewedEventCallbackInfo() = default;
// AdsTabHelper::AdViewedEventCallbackInfo::AdViewedEventCallbackInfo(
//     AdViewedEventCallbackInfo&& info) = default;
// AdsTabHelper::AdViewedEventCallbackInfo&
// AdsTabHelper::AdViewedEventCallbackInfo::operator=(
//     AdViewedEventCallbackInfo&& info) = default;
// AdsTabHelper::AdViewedEventCallbackInfo::~AdViewedEventCallbackInfo() = default;

AdsTabHelper::AdsTabHelper(content::WebContents* web_contents)
    : WebContentsObserver(web_contents),
      content::WebContentsUserData<AdsTabHelper>(*web_contents),
      tab_id_(sessions::SessionTabHelper::IdForTab(web_contents)),
      weak_factory_(this) {
  if (!tab_id_.is_valid()) {
    return;
  }

  Profile* profile =
      Profile::FromBrowserContext(web_contents->GetBrowserContext());
  ads_service_ = AdsServiceFactory::GetForProfile(profile);
  search_result_ad_service_ =
      SearchResultAdServiceFactory::GetForProfile(profile);

#if !BUILDFLAG(IS_ANDROID)
  BrowserList::AddObserver(this);
  OnBrowserSetLastActive(BrowserList::GetInstance()->GetLastActive());
#endif
  OnVisibilityChanged(web_contents->GetVisibility());
}

AdsTabHelper::~AdsTabHelper() {
#if !BUILDFLAG(IS_ANDROID)
  BrowserList::RemoveObserver(this);
#endif
}

void AdsTabHelper::MaybeTriggerSearchResultAdViewedEvent(
    const std::string& creative_instance_id,
    base::OnceCallback<void(bool)> callback) {
  if (!ads_service_ || !ads_service_->IsEnabled() ||
      !search_result_ad_handler_) {
    std::move(callback).Run(/* event_triggered */ false);
    return;
  }

  search_result_ad_handler_->MaybeTriggerSearchResultAdViewedEvent(
      creative_instance_id, std::move(callback));
}

absl::optional<GURL>
AdsTabHelper::MaybeTriggerSearchResultAdClickedEvent(
    const std::string& creative_instance_id) {
  if (!ads_service_ || !ads_service_->IsEnabled() ||
      !search_result_ad_handler_) {
    return absl::nullopt;
  }

  return search_result_ad_handler_->MaybeTriggerSearchResultAdClickedEvent(
      creative_instance_id);
}

void AdsTabHelper::TabUpdated() {
  if (!ads_service_) {
    return;
  }

  ads_service_->OnTabUpdated(tab_id_, GURL(web_contents()->GetVisibleURL()),
                             is_active_, is_browser_active_);
}

void AdsTabHelper::RunIsolatedJavaScript(
    content::RenderFrameHost* render_frame_host) {
  DCHECK(render_frame_host);

  dom_distiller::RunIsolatedJavaScript(
      render_frame_host, "new XMLSerializer().serializeToString(document)",
      base::BindOnce(&AdsTabHelper::OnJavaScriptHtmlResult,
                     weak_factory_.GetWeakPtr()));

  dom_distiller::RunIsolatedJavaScript(
      render_frame_host, "document?.body?.innerText",
      base::BindOnce(&AdsTabHelper::OnJavaScriptTextResult,
                     weak_factory_.GetWeakPtr()));
}

void AdsTabHelper::OnJavaScriptHtmlResult(base::Value value) {
  if (!ads_service_) {
    return;
  }

  if (!value.is_string()) {
    return;
  }
  const std::string& html = value.GetString();
  ads_service_->OnHtmlLoaded(tab_id_, redirect_chain_, html);
}

void AdsTabHelper::OnJavaScriptTextResult(base::Value value) {
  if (!ads_service_) {
    return;
  }

  if (!value.is_string()) {
    return;
  }
  const std::string& text = value.GetString();
  ads_service_->OnTextLoaded(tab_id_, redirect_chain_, text);
}

void AdsTabHelper::DidFinishNavigation(
    content::NavigationHandle* navigation_handle) {
  DCHECK(navigation_handle);

  if (!ads_service_ || !navigation_handle->IsInMainFrame() ||
      !navigation_handle->HasCommitted() || !tab_id_.is_valid()) {
    return;
  }

  if (navigation_handle->HasUserGesture()) {
    const int32_t page_transition =
        static_cast<int32_t>(navigation_handle->GetPageTransition());

    ads_service_->OnUserGesture(page_transition);
  }

  redirect_chain_ = navigation_handle->GetRedirectChain();

  if (!navigation_handle->IsSameDocument()) {
    // if (search_result_ad_service_) {
    //   search_result_ad_service_->OnDidFinishNavigation(tab_id_);
    // }
    if (ads_service_ && ads_service_->IsEnabled() &&
        search_result_ad_service_) {
      search_result_ad_handler_ =
          std::make_unique<SearchResultAdHandler>(ads_service_,
                                                  search_result_ad_service_);
    }

    should_process_ = navigation_handle->GetRestoreType() ==
                      content::RestoreType::kNotRestored;
    return;
  }

  content::RenderFrameHost* render_frame_host =
      navigation_handle->GetRenderFrameHost();

  RunIsolatedJavaScript(render_frame_host);
}

void AdsTabHelper::DocumentOnLoadCompletedInPrimaryMainFrame() {
  content::RenderFrameHost* render_frame_host = web_contents()->GetMainFrame();
  if (search_result_ad_handler_) {
    search_result_ad_handler_->MaybeRetrieveSearchResultAd(render_frame_host,
                                                          should_process_);
  }

  if (!should_process_) {
    return;
  }

  RunIsolatedJavaScript(render_frame_host);
}

void AdsTabHelper::DidFinishLoad(content::RenderFrameHost* render_frame_host,
                                 const GURL& validated_url) {
  DCHECK(render_frame_host);

  if (render_frame_host->GetParent()) {
    return;
  }

  TabUpdated();
}

void AdsTabHelper::MediaStartedPlaying(const MediaPlayerInfo& video_type,
                                       const content::MediaPlayerId& id) {
  if (!ads_service_) {
    return;
  }

  ads_service_->OnMediaStart(tab_id_);
}

void AdsTabHelper::MediaStoppedPlaying(
    const MediaPlayerInfo& video_type,
    const content::MediaPlayerId& id,
    WebContentsObserver::MediaStoppedReason reason) {
  if (!ads_service_) {
    return;
  }

  ads_service_->OnMediaStop(tab_id_);
}

void AdsTabHelper::OnVisibilityChanged(content::Visibility visibility) {
  const bool old_is_active = is_active_;

  switch (visibility) {
    case content::Visibility::HIDDEN:
    case content::Visibility::OCCLUDED: {
      is_active_ = false;
      break;
    }

    case content::Visibility::VISIBLE: {
      is_active_ = true;
      break;
    }
  }

  if (old_is_active == is_active_) {
    return;
  }

  TabUpdated();
}

void AdsTabHelper::WebContentsDestroyed() {
  // if (search_result_ad_service_) {
  //   search_result_ad_service_->OnTabClosed(tab_id_);
  //   search_result_ad_service_ = nullptr;
  // }

  search_result_ad_handler_.reset();

  if (!ads_service_) {
    return;
  }

  ads_service_->OnTabClosed(tab_id_);
  ads_service_ = nullptr;
}

// void AdsTabHelper::MaybeTriggerSearchResultAdViewedEvent(
//     const std::string& creative_instance_id,
//     base::OnceCallback<void(bool)> callback) {
//   DCHECK(!creative_instance_id.empty());

//   if (!ads_service_ || !ads_service_->IsEnabled()) {
//     std::move(callback).Run(/* event_triggered */ false);
//     return;
//   }

//   // Check if search result ad JSON-LD wasn't processed yet.
//   if (!search_result_ads_) {
//     // Check if OnDidFinishNavigation was called for tab_id.
//     if (!base::Contains(ad_viewed_event_pending_callbacks_, tab_id)) {
//       std::move(callback).Run(/* event_triggered */ false);
//       return;
//     }

//     AdViewedEventCallbackInfo callback_info;
//     callback_info.creative_instance_id = creative_instance_id;
//     callback_info.callback = std::move(callback);
//     ad_viewed_event_pending_callbacks_.push_back(std::move(callback_info));
//     return;
//   }

//   const bool event_triggered =
//       QueueSearchResultAdViewedEvent(creative_instance_id);
//   std::move(callback).Run(event_triggered);
// }

// void AdsTabHelper::MaybeRetrieveSearchResultAd(
//     content::RenderFrameHost* render_frame_host,
//     bool should_trigger_viewed_event) {
//   DCHECK(render_frame_host);

//   if (!search_result_ad_service_) {
//     return;
//   }

//   search_result_ads_ = SearchResultAdMap();

//   const GURL url = render_frame_host->GetLastCommittedURL();
//   if (!search_result_ad_service_->ShouldRetrieveSearchResultAd(url)) {
//     if (metadata_request_finished_callback_for_testing_) {
//       std::move(metadata_request_finished_callback_for_testing_).Run();
//     }
//     RunAdViewedEventPendingCallbacks(/* ads_fetched */ false);
//     return;
//   }

//   mojo::Remote<blink::mojom::DocumentMetadata> document_metadata;
//   render_frame_host->GetRemoteInterfaces()->GetInterface(
//       document_metadata.BindNewPipeAndPassReceiver());
//   DCHECK(document_metadata.is_bound());
//   document_metadata.reset_on_disconnect();

//   blink::mojom::DocumentMetadata* raw_document_metadata =
//       document_metadata.get();
//   raw_document_metadata->GetEntities(
//       base::BindOnce(&AdsTabHelper::OnRetrieveSearchResultAdEntities,
//                      weak_factory_.GetWeakPtr(), std::move(document_metadata),
//                      should_trigger_viewed_event));
// }

// void AdsTabHelper::OnRetrieveSearchResultAdEntities(
//     mojo::Remote<blink::mojom::DocumentMetadata> document_metadata,
//     bool should_trigger_viewed_event,
//     blink::mojom::WebPagePtr web_page) {
//   if (metadata_request_finished_callback_for_testing_) {
//     std::move(metadata_request_finished_callback_for_testing_).Run();
//   }

//   if (!web_page) {
//     RunAdViewedEventPendingCallbacks(/* ads_fetched */ false);
//     return;
//   }

//   search_result_ads_ = ParseWebPageEntities(
//       std::move(web_page), should_trigger_viewed_event
//                                ? SearchResultAdState::kReadyForView
//                                : SearchResultAdState::kNotCountView);

//   RunAdViewedEventPendingCallbacks(/* ads_fetched */ true);
// }

// void AdsTabHelper::RunAdViewedEventPendingCallbacks(bool ads_fetched) {
//   for (auto& callback_info : ad_viewed_event_pending_callbacks_) {
//     bool event_triggered = false;
//     if (ads_fetched) {
//       event_triggered = QueueSearchResultAdViewedEvent(
//           callback_info.creative_instance_id);
//     }
//     if (event_triggered) {
//       DVLOG(1) << "Triggered search result ad viewed event for "
//                << callback_info.creative_instance_id;
//     } else {
//       DVLOG(1) << "Failed to trigger search result ad viewed event for "
//                << callback_info.creative_instance_id;
//     }
//     std::move(callback_info.callback).Run(event_triggered);
//   }
//   ad_viewed_event_pending_callbacks_.clear();
// }

// bool AdsTabHelper::QueueSearchResultAdViewedEvent(
//     const std::string& creative_instance_id) {
//   DCHECK(!creative_instance_id.empty());
//   DCHECK(search_result_ad_service_);
//   DCHECK(search_result_ads_);

//   auto it = search_result_ads_->find(creative_instance_id);
//   if (it == search_result_ads_->end()) {
//     return false;
//   }

//   if (it->second.state == SearchResultAdState::kNotReady ||
//       it->second.state == SearchResultAdState::kReadyForClick) {
//     return false;
//   }

//   const ads::mojom::SearchResultAdPtr& search_result_ad = it->second.ad;
//   DCHECK(search_result_ad);

//   if (it->second.state == SearchResultAdState::kReadyForView) {
//     search_result_ad_service_->AddSearchResultAdViewedEventToQueue(
//         search_result_ad->Clone());
//     search_result_ad_service_->TriggerSearchResultAdViewedEventFromQueue();
//   }
//   it->second.state = SearchResultAdState::kReadyForClick;

//   return true;
// }

// void SearchResultAdService::ResetState(SessionID tab_id) {
//   DCHECK(tab_id.is_valid());

//   for (auto& callback_info : ad_viewed_event_pending_callbacks_[tab_id]) {
//     std::move(callback_info.callback).Run(false);
//   }
//   ad_viewed_event_pending_callbacks_.erase(tab_id);
//   search_result_ads_.erase(tab_id);
// }

#if !BUILDFLAG(IS_ANDROID)
// components/brave_ads/browser/background_helper_android.cc handles Android
void AdsTabHelper::OnBrowserSetLastActive(Browser* browser) {
  if (!browser) {
    return;
  }

  const bool old_is_browser_active = is_browser_active_;

  if (browser->tab_strip_model()->GetIndexOfWebContents(web_contents()) !=
      TabStripModel::kNoTab) {
    is_browser_active_ = true;
  }

  if (old_is_browser_active == is_browser_active_) {
    return;
  }

  TabUpdated();
}

void AdsTabHelper::OnBrowserNoLongerActive(Browser* browser) {
  DCHECK(browser);

  const bool old_is_browser_active = is_browser_active_;

  if (browser->tab_strip_model()->GetIndexOfWebContents(web_contents()) !=
      TabStripModel::kNoTab) {
    is_browser_active_ = false;
  }

  if (old_is_browser_active == is_browser_active_) {
    return;
  }

  TabUpdated();
}
#endif

WEB_CONTENTS_USER_DATA_KEY_IMPL(AdsTabHelper);

}  // namespace brave_ads
