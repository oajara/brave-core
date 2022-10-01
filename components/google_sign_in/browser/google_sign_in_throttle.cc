/* Copyright 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/google_sign_in/browser/google_sign_in_throttle.h"

#include <utility>
#include <vector>

#include "brave/components/google_sign_in/common/google_sign_in_util.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/permission_controller_delegate.h"
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
    scoped_refptr<HostContentSettingsMap> settings_map)
    : wc_getter_(wc_getter), settings_map_(settings_map) {}

void OnPermissionRequestStatus(
    const GURL& request_initiator_url,
    scoped_refptr<HostContentSettingsMap> content_settings,
    URLLoaderThrottle::Delegate* delegate,
    const std::vector<blink::mojom::PermissionStatus>& permission_statuses) {
  HandleBraveGoogleSignInPermissionStatus(
      request_initiator_url, content_settings, permission_statuses);
  DCHECK_EQ(1u, permission_statuses.size());
  if (permission_statuses[0] == blink::mojom::PermissionStatus::GRANTED) {
    delegate->Resume();
  } else {
    // if DENIED or ASK (i.e. user has not yet made a decision), we cancel the
    // request
    delegate->CancelWithError(net::ERR_BLOCKED_BY_CLIENT);
  }
}

std::unique_ptr<blink::URLLoaderThrottle>
GoogleSignInThrottle::MaybeCreateThrottleFor(
    const network::ResourceRequest& request,
    const content::WebContents::Getter& wc_getter,
    HostContentSettingsMap* content_settings) {
  if (request.resource_type !=
      static_cast<int>(blink::mojom::ResourceType::kMainFrame)) {
    return nullptr;
  }

  const auto request_url = request.url;
  const auto request_initiator_url =
      request.request_initiator.value_or(url::Origin()).GetURL();

  if (!request_initiator_url.is_valid() || !request_url.is_valid()) {
    return nullptr;
  }

  if (!ShouldCheckGoogleSignInPermission(request_url, request_initiator_url)) {
    // We don't want to prompt the user to add a permission for
    // accounts.google.com to access accounts.google.com!
    return nullptr;
  }

  return std::make_unique<GoogleSignInThrottle>(
      wc_getter,
      base::WrapRefCounted<HostContentSettingsMap>(content_settings));
}

GoogleSignInThrottle::~GoogleSignInThrottle() = default;

void GoogleSignInThrottle::DetachFromCurrentSequence() {}

void GoogleSignInThrottle::WillStartRequest(network::ResourceRequest* request,
                                            bool* defer) {
  const GURL request_url = request->url;
  const GURL request_initiator_url = request->request_initiator->GetURL();

  DCHECK(request);
  DCHECK(request->request_initiator);

  auto* contents = wc_getter_.Run();

  if (!contents)
    return;

  // Check kGoogleLoginControlType pref and cancel request if false
  // NOTE: This means that if the kGoogleLoginControlType permission in
  // brave://settings/socialMediaBlocking is turned off, all requests to
  // kGoogleAuthPattern and kFirebaseUrlPattern will be disallowed
  PrefService* prefs =
      user_prefs::UserPrefs::Get(contents->GetBrowserContext());
  if (!IsGoogleSignInEnabled(prefs)) {
    delegate_->CancelWithError(net::ERR_BLOCKED_BY_CLIENT);
    return;
  }

  content::PermissionControllerDelegate* permission_controller =
      contents->GetBrowserContext()->GetPermissionControllerDelegate();

  // Check current permission status
  auto current_status = GetCurrentGoogleSignInPermissionStatus(
      permission_controller, contents, request_initiator_url);

  if (current_status == blink::mojom::PermissionStatus::ASK) {
    *defer = true;
    // Create permission request
    permission_controller->RequestPermissionsForOrigin(
        {blink::PermissionType::BRAVE_GOOGLE_SIGN_IN},
        contents->GetPrimaryMainFrame(), request_initiator_url, true,
        base::BindOnce(&OnPermissionRequestStatus, request_initiator_url,
                       settings_map_, delegate_));
  } else if (current_status == blink::mojom::PermissionStatus::DENIED) {
    // Permission denied, cancel request
    delegate_->CancelWithError(net::ERR_BLOCKED_BY_CLIENT);
  }
  // If GRANTED, allow request to continue without deferring load
  return;
}

}  // namespace google_sign_in
