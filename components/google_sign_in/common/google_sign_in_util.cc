/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/google_sign_in/common/google_sign_in_util.h"

#include <vector>

#include "brave/components/constants/pref_names.h"
#include "extensions/common/url_pattern.h"
#include "third_party/blink/public/mojom/permissions/permission_status.mojom-shared.h"

namespace google_sign_in {

namespace {
constexpr char kGoogleAuthPattern[] =
    "https://accounts.google.com/o/oauth2/auth*";
constexpr char kFirebaseContentSettingsPattern[] =
    "https://[*.]firebaseapp.com/__/auth*";
constexpr char kFirebaseUrlPattern[] = "https://*.firebaseapp.com/__/auth*";

bool IsGoogleAuthUrl(const GURL& gurl, const bool check_origin_only) {
  const std::vector<URLPattern> auth_login_patterns({
      URLPattern(URLPattern::SCHEME_HTTPS, kGoogleAuthPattern),
      URLPattern(URLPattern::SCHEME_HTTPS, kFirebaseUrlPattern),
  });
  return std::any_of(auth_login_patterns.begin(), auth_login_patterns.end(),
                     [&gurl, check_origin_only](URLPattern pattern) {
                       if (check_origin_only) {
                         return pattern.MatchesSecurityOrigin(gurl);
                       }
                       return pattern.MatchesURL(gurl);
                     });
}
}  // namespace

bool ShouldCheckGoogleSignInPermission(const GURL& request_url,
                                       const GURL& request_initiator_url) {
  return IsGoogleAuthUrl(request_url, false) &&
         !IsGoogleAuthUrl(request_initiator_url, true);
}

bool IsGoogleSignInEnabled(PrefService* prefs) {
  return prefs->FindPreference(kGoogleLoginControlType) &&
         prefs->GetBoolean(kGoogleLoginControlType);
}

blink::mojom::PermissionStatus GetCurrentGoogleSignInPermissionStatus(
    content::PermissionControllerDelegate* permission_controller,
    content::WebContents* contents,
    const GURL& request_initiator_url) {
  return permission_controller->GetPermissionStatusForOrigin(
      blink::PermissionType::BRAVE_GOOGLE_SIGN_IN,
      contents->GetPrimaryMainFrame(), request_initiator_url);
}

void Set3pCookieException(HostContentSettingsMap* content_settings,
                          const ContentSettingsPattern& embedding_pattern,
                          const ContentSetting& content_setting) {
  const std::vector<std::string> auth_content_settings_patterns(
      {kGoogleAuthPattern, kFirebaseContentSettingsPattern});
  for (const auto& auth_url_pattern : auth_content_settings_patterns) {
    auto auth_pattern = ContentSettingsPattern::FromString(auth_url_pattern);
    content_settings->SetContentSettingCustomScope(
        auth_pattern, embedding_pattern, ContentSettingsType::BRAVE_COOKIES,
        content_setting);
  }
}

// Sets 3p cookie exception for Google Auth-related URLs, for a particular URL
// If the status == ASK i.e. user dismisses the prompt so we don't do anything
void HandleBraveGoogleSignInPermissionStatus(
    const GURL& request_initiator_url,
    scoped_refptr<HostContentSettingsMap> content_settings,
    const std::vector<blink::mojom::PermissionStatus>& permission_statuses) {
  DCHECK_EQ(1u, permission_statuses.size());
  auto permission_status = permission_statuses[0];
  auto embedding_pattern =
      ContentSettingsPattern::FromURL(request_initiator_url);
  if (permission_status == blink::mojom::PermissionStatus::GRANTED) {
    // Add 3p exception for request_initiator_url for auth patterns
    Set3pCookieException(content_settings.get(), embedding_pattern,
                         CONTENT_SETTING_ALLOW);
  } else if (permission_status == blink::mojom::PermissionStatus::DENIED) {
    // Remove 3p exception for request_initiator_url for auth patterns
    Set3pCookieException(content_settings.get(), embedding_pattern,
                         CONTENT_SETTING_BLOCK);
  }
  return;
}

}  // namespace google_sign_in
