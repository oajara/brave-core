/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CONTENT_BROWSER_SEARCH_RESULT_AD_SEARCH_RESULT_AD_HANDLER_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CONTENT_BROWSER_SEARCH_RESULT_AD_SEARCH_RESULT_AD_HANDLER_H_

#include <vector>

#include "base/callback.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "brave/components/brave_ads/core/browser/search_result_ad/search_result_ad_info.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/blink/public/mojom/document_metadata/document_metadata.mojom.h"

namespace content {
class RenderFrameHost;
}

namespace brave_ads {

class AdsService;
class SearchResultAdService;

class SearchResultAdHandler final {
 public:
  SearchResultAdHandler(AdsService* ads_service,
                        SearchResultAdService* search_result_ad_service);
  ~SearchResultAdHandler();

  SearchResultAdHandler(const SearchResultAdHandler&) = delete;
  SearchResultAdHandler& operator=(const SearchResultAdHandler&) = delete;

  void MaybeRetrieveSearchResultAd(
      content::RenderFrameHost* render_frame_host,
      bool should_trigger_viewed_event);

  void MaybeTriggerSearchResultAdViewedEvent(
      const std::string& creative_instance_id,
      base::OnceCallback<void(bool)> callback);

  absl::optional<GURL> MaybeTriggerSearchResultAdClickedEvent(
    const std::string& creative_instance_id);

 private:
  void OnRetrieveSearchResultAdEntities(
      mojo::Remote<blink::mojom::DocumentMetadata> document_metadata,
      bool should_trigger_viewed_event,
      blink::mojom::WebPagePtr web_page);
  void RunAdViewedEventPendingCallbacks(bool ads_fetched);
  bool QueueSearchResultAdViewedEvent(
    const std::string& creative_instance_id);

  struct AdViewedEventCallbackInfo {
    AdViewedEventCallbackInfo();
    AdViewedEventCallbackInfo(AdViewedEventCallbackInfo&& info);
    AdViewedEventCallbackInfo& operator=(AdViewedEventCallbackInfo&& info);
    ~AdViewedEventCallbackInfo();

    std::string creative_instance_id;
    base::OnceCallback<void(bool)> callback;
  };

  raw_ptr<AdsService> ads_service_ = nullptr;  // NOT OWNED
  raw_ptr<SearchResultAdService> search_result_ad_service_ =
      nullptr;  // NOT OWNED

  absl::optional<SearchResultAdMap> search_result_ads_;
  std::vector<AdViewedEventCallbackInfo> ad_viewed_event_pending_callbacks_;
  base::OnceClosure metadata_request_finished_callback_for_testing_;

  base::WeakPtrFactory<SearchResultAdHandler> weak_factory_{this};
};

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CONTENT_BROWSER_SEARCH_RESULT_AD_SEARCH_RESULT_AD_HANDLER_H_
