/* Copyright 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/google_sign_in/browser/google_sign_in_throttle.h"

#include <iostream>
#include <utility>
#include <vector>

#include "brave/components/google_sign_in/common/google_sign_in_util.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/permission_controller_delegate.h"
#include "content/public/browser/web_contents.h"
#include "extensions/common/url_pattern.h"
#include "services/network/public/cpp/resource_request.h"
#include "third_party/blink/public/common/permissions/permission_utils.h"
#include "third_party/blink/public/mojom/loader/resource_load_info.mojom-shared.h"

#include "url/gurl.h"
#include "url/origin.h"

using blink::URLLoaderThrottle;

namespace google_sign_in {

GoogleSignInThrottle::GoogleSignInThrottle(
    const content::WebContents::Getter& wc_getter,
    const GURL initial_url,
    scoped_refptr<HostContentSettingsMap> settings_map)
    : wc_getter_(wc_getter),
      initial_url_(initial_url),
      settings_map_(settings_map) {}

void OnPermissionRequestStatus(
    content::NavigationEntry* pending_entry,
    content::WebContents* contents,
    const GURL& request_initiator_url,
    scoped_refptr<HostContentSettingsMap> content_settings,
    URLLoaderThrottle::Delegate* delegate,
    const std::vector<blink::mojom::PermissionStatus>& permission_statuses) {
  std::cout << "In OnPermissionRequestStatus" << std::endl;
  std::cout << "1" << std::endl;
  HandleBraveGoogleSignInPermissionStatus(
      request_initiator_url, content_settings, permission_statuses);
  std::cout << "2" << std::endl;
  DCHECK_EQ(1u, permission_statuses.size());
  const auto status = permission_statuses[0];
  std::cout << "status is " << status << std::endl;
  std::cout << "request_initiator_url is " << request_initiator_url
            << std::endl;
  // Check if current pending navigation is the one we started out with.
  // This is done to prevent us from accessing deleted Delegate, in case
  // the user navigated away while the prompt was still up, or closed the
  // window
  if (pending_entry != contents->GetController().GetPendingEntry()) {
    std::cout << "Current pending navigation is not the one we started out with"
              << std::endl;
    // print out the one we started out with
    std::cout << "The one we started out with: "
              << pending_entry->GetVirtualURL() << std::endl;
    // print out current pending navigation
    std::cout << "Current pending navigation: "
              << contents->GetController().GetPendingEntry()->GetVirtualURL()
              << std::endl;
    return;
  }
  std::cout << "3" << std::endl;
  if (status == blink::mojom::PermissionStatus::GRANTED) {
    std::cout << "4" << std::endl;
    delegate->Resume();
  } else if (status == blink::mojom::PermissionStatus::DENIED) {
    std::cout << "5" << std::endl;
    delegate->CancelWithError(net::ERR_BLOCKED_BY_CLIENT);
  }
  // In case of ASK, we need to be careful because delegate may be deleted
  return;
}

void GetPermissionAndMaybeCreatePrompt(
    bool* defer,
    content::WebContents* contents,
    const GURL& request_initiator_url,
    scoped_refptr<HostContentSettingsMap> content_settings,
    URLLoaderThrottle::Delegate* delegate) {
  std::cout << "In GetPermissionAndMaybeCreatePrompt" << std::endl;
  // Check kGoogleLoginControlType pref and cancel request if false
  // NOTE: This means that if the kGoogleLoginControlType permission in
  // brave://settings/socialMediaBlocking is turned off, all requests to
  // kGoogleAuthPattern and kFirebaseUrlPattern will be disallowed
  PrefService* prefs =
      user_prefs::UserPrefs::Get(contents->GetBrowserContext());
  if (!IsGoogleSignInPrefEnabled(prefs)) {
    delegate->CancelWithError(net::ERR_BLOCKED_BY_CLIENT);
    return;
  }

  content::PermissionControllerDelegate* permission_controller =
      contents->GetBrowserContext()->GetPermissionControllerDelegate();

  // Check current permission status
  auto current_status = GetCurrentGoogleSignInPermissionStatus(
      permission_controller, contents, request_initiator_url);

  std::cout << "Current status: " << current_status << std::endl;

  if (current_status == blink::mojom::PermissionStatus::ASK) {
    *defer = true;
    // Create permission request
    permission_controller->RequestPermissionsForOrigin(
        {blink::PermissionType::BRAVE_GOOGLE_SIGN_IN},
        contents->GetPrimaryMainFrame(), request_initiator_url, true,
        base::BindOnce(&OnPermissionRequestStatus,
                       contents->GetController().GetPendingEntry(), contents,
                       request_initiator_url, content_settings, delegate));
  } else if (current_status == blink::mojom::PermissionStatus::DENIED) {
    // Permission denied, cancel request
    delegate->CancelWithError(net::ERR_BLOCKED_BY_CLIENT);
  }
  // If GRANTED, allow request to continue without deferring load
  return;
}

std::unique_ptr<blink::URLLoaderThrottle>
GoogleSignInThrottle::MaybeCreateThrottleFor(
    const network::ResourceRequest& request,
    const content::WebContents::Getter& wc_getter,
    HostContentSettingsMap* content_settings) {
  if (!IsGoogleSignInFeatureEnabled()) {
    return nullptr;
  }

  if (request.resource_type !=
      static_cast<int>(blink::mojom::ResourceType::kMainFrame)) {
    return nullptr;
  }

  const auto request_url = request.url;
  const auto request_initiator_url =
      request.request_initiator.value_or(url::Origin()).GetURL();

  std::cout << "In GoogleSignInThrottle::MaybeCreateThrottleFor, request_url: "
            << request_url << std::endl;
  std::cout << "request_url: " << request_url << std::endl;
  std::cout << "request_initiator_url: " << request_initiator_url << std::endl;

  // We might not have a request_initiator_url if the request is coming from
  // a top-level navigation and it's a redirect. We still want to create the
  // throttle.
  if (!request_url.SchemeIsHTTPOrHTTPS()) {
    return nullptr;
  }

  std::cout << "Creating GoogleSignInThrottle" << std::endl;

  return std::make_unique<GoogleSignInThrottle>(
      wc_getter, request_url,
      base::WrapRefCounted<HostContentSettingsMap>(content_settings));
}

GoogleSignInThrottle::~GoogleSignInThrottle() = default;

void GoogleSignInThrottle::DetachFromCurrentSequence() {}

void GoogleSignInThrottle::WillStartRequest(network::ResourceRequest* request,
                                            bool* defer) {
  std::cout << "In GoogleSignInThrottle::WillStartRequest" << std::endl;
  const GURL request_url = request->url;
  std::cout << "request_url: " << request_url << std::endl;
  const auto request_initiator_url =
      request->request_initiator.value_or(url::Origin()).GetURL();

  std::cout << "WillStartRequest request_url: " << request_url
            << " request_initiator_url: " << request_initiator_url << std::endl;

  if (!IsGoogleAuthRelatedRequest(request_url, request_initiator_url)) {
    return;
  }

  auto* contents = wc_getter_.Run();

  if (!contents)
    return;

  GetPermissionAndMaybeCreatePrompt(defer, contents, request_initiator_url,
                                    settings_map_, delegate_);
}

void GoogleSignInThrottle::BeforeWillRedirectRequest(
    net::RedirectInfo* redirect_info,
    const network::mojom::URLResponseHead& response_head,
    bool* defer,
    std::vector<std::string>* to_be_removed_request_headers,
    net::HttpRequestHeaders* modified_request_headers,
    net::HttpRequestHeaders* modified_cors_exempt_request_headers) {
  std::cout << "In GoogleSignInThrottle::BeforeWillRedirectRequest"
            << std::endl;

  // We should detect if we're in a new window and being redirected
  // In this case, we get and set the Google Sign In permission
  // on the opening window (last committed URL)

  auto* contents = wc_getter_.Run();

  if (!contents) {
    return;
  }

  const auto request_url = redirect_info->new_url;
  std::cout << "initial url: " << initial_url_ << std::endl;
  std::cout << "new location: " << request_url << std::endl;
  std::cout << "last committed URL: " << contents->GetLastCommittedURL()
            << std::endl;
  std::cout << "visible URL: " << contents->GetVisibleURL() << std::endl;

  if (!IsGoogleAuthRelatedRequest(request_url, initial_url_)) {
    return;
  }

  GetPermissionAndMaybeCreatePrompt(defer, contents, initial_url_,
                                    settings_map_, delegate_);
}

}  // namespace google_sign_in
