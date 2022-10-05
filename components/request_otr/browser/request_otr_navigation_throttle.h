/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_REQUEST_OTR_NAVIGATION_THROTTLE_H_
#define BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_REQUEST_OTR_NAVIGATION_THROTTLE_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "brave/components/brave_shields/browser/brave_shields_util.h"
#include "content/public/browser/navigation_throttle.h"
#include "url/gurl.h"

namespace content {
class NavigationHandle;
class WebContents;
}  // namespace content

namespace ephemeral_storage {
class EphemeralStorageService;
}  // namespace ephemeral_storage

namespace request_otr {

class RequestOTRService;

class RequestOTRNavigationThrottle : public content::NavigationThrottle {
 public:
  explicit RequestOTRNavigationThrottle(
      content::NavigationHandle* navigation_handle,
      RequestOTRService* request_otr_service,
      ephemeral_storage::EphemeralStorageService* ephemeral_storage_service,
      const std::string& locale);
  ~RequestOTRNavigationThrottle() override;

  RequestOTRNavigationThrottle(const RequestOTRNavigationThrottle&) = delete;
  RequestOTRNavigationThrottle& operator=(const RequestOTRNavigationThrottle&) =
      delete;

  static std::unique_ptr<RequestOTRNavigationThrottle> MaybeCreateThrottleFor(
      content::NavigationHandle* navigation_handle,
      RequestOTRService* request_otr_service,
      ephemeral_storage::EphemeralStorageService* ephemeral_storage_service,
      const std::string& locale);

  // content::NavigationThrottle implementation:
  content::NavigationThrottle::ThrottleCheckResult WillStartRequest() override;
  content::NavigationThrottle::ThrottleCheckResult WillRedirectRequest()
      override;
  content::NavigationThrottle::ThrottleCheckResult WillProcessResponse()
      override;
  const char* GetNameForLogging() override;

 private:
  content::NavigationThrottle::ThrottleCheckResult ShowInterstitial();
  void Enable1PESAndResume();

  RequestOTRService* request_otr_service_ = nullptr;  // not owned
  ephemeral_storage::EphemeralStorageService* ephemeral_storage_service_ =
      nullptr;  // not owned
  std::string locale_;

  base::WeakPtrFactory<RequestOTRNavigationThrottle> weak_ptr_factory_{this};
};

}  // namespace request_otr

#endif  // BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_REQUEST_OTR_NAVIGATION_THROTTLE_H_
