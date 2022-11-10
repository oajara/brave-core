/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/google_sign_in/common/google_sign_in_util.h"

#include <iostream>
#include <utility>
#include <vector>
#include "brave/components/constants/pref_names.h"
#include "brave/components/google_sign_in/common/features.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "extensions/common/url_pattern.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"

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

bool IsGoogleAuthRelatedRequest(const GURL& request_url,
                                const GURL& request_initiator_url) {
  return IsGoogleAuthUrl(request_url, false) &&
         !IsGoogleAuthUrl(request_initiator_url, true);
}

bool IsGoogleSignInFeatureEnabled() {
  return base::FeatureList::IsEnabled(
      google_sign_in::features::kBraveGoogleSignIn);
}

bool IsGoogleSignInPrefEnabled(PrefService* prefs) {
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
  // First we clear all content settings for the embedding pattern

  // embedding_pattern is the website that is embedding the google sign in
  // auth pattern is auth.google.com or firebaseapp.com
  for (const auto& auth_url_pattern : auth_content_settings_patterns) {
    auto auth_pattern = ContentSettingsPattern::FromString(auth_url_pattern);
    std::cout << "auth_pattern: " << auth_pattern.ToString() << std::endl;
    std::cout << "embedding_pattern: " << embedding_pattern.ToString()
              << std::endl;

    GURL primary_url("https://accounts.google.com");
    GURL secondary_url("https://www.joinhoney.com");

    std::cout << "primary_url: " << primary_url << std::endl;
    std::cout << "secondary_url: " << secondary_url << std::endl;

    std::cout << "Current content setting for these URLs is: "
              << content_settings->GetContentSetting(
                     primary_url, secondary_url,
                     ContentSettingsType::BRAVE_COOKIES)
              << "\n\n";
    ContentSettingsForOneType host_settings;

    content_settings->GetSettingsForOneType(ContentSettingsType::BRAVE_COOKIES,
                                            &host_settings);
    std::cout << "In Set3pCookieException... now let's print out all host "
                 "content settings for this setting"
              << "\n";
    for (auto setting : host_settings) {
      std::cout << "primary: " << setting.primary_pattern.ToString()
                << " secondary: " << setting.secondary_pattern.ToString()
                << " setting: " << setting.GetContentSetting() << "\n";
    }
    content_settings->SetContentSettingCustomScope(
        auth_pattern, embedding_pattern, ContentSettingsType::BRAVE_COOKIES,
        content_setting);
  }
}

// Sets 3p cookie exception for Google Auth-related URLs, for a particular URL
// If the status == ASK i.e. user dismisses the prompt so we don't do anything
void HandleBraveGoogleSignInPermissionStatus(
    content::BrowserContext* context,
    const GURL& request_initiator_url,
    scoped_refptr<HostContentSettingsMap> content_settings,
    const std::vector<blink::mojom::PermissionStatus>& permission_statuses) {
  DCHECK_EQ(1u, permission_statuses.size());
  auto permission_status = permission_statuses[0];
  auto embedding_pattern =
      ContentSettingsPattern::FromURLNoWildcard(request_initiator_url);
  if (permission_status == blink::mojom::PermissionStatus::GRANTED) {
    // Add 3p exception for request_initiator_url for auth patterns
    std::cout << "Adding 3p exception for " << request_initiator_url.spec()
              << std::endl;
    Set3pCookieException(content_settings.get(), embedding_pattern,
                         CONTENT_SETTING_ALLOW);
  } else if (permission_status == blink::mojom::PermissionStatus::DENIED) {
    // Remove 3p exception for request_initiator_url for auth patterns
    Set3pCookieException(content_settings.get(), embedding_pattern,
                         CONTENT_SETTING_BLOCK);
    // Delete existing 3p cookies
    // auto filter = network::mojom::CookieDeletionFilter::New();
    // filter->excluding_domains = std::vector<std::string>();
    // filter->excluding_domains->push_back(embedding_pattern.GetHost());
    // context->GetDefaultStoragePartition()
    //     ->GetCookieManagerForBrowserProcess()
    //     ->DeleteCookies(std::move(filter), base::NullCallback());
  }
  return;
}

}  // namespace google_sign_in
