/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/google_sign_in/common/google_sign_in_util.h"

#include <iostream>
#include <utility>
#include <vector>
#include "base/callback_helpers.h"
#include "brave/components/constants/pref_names.h"
#include "brave/components/google_sign_in/common/features.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "extensions/common/url_pattern.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"

#include "third_party/blink/public/mojom/permissions/permission_status.mojom-shared.h"
#include "ui/base/page_transition_types.h"
#include "ui/base/window_open_disposition.h"

namespace google_sign_in {

namespace {
constexpr char kGoogleAuthPattern[] =
    "https://accounts.google.com/o/oauth2/auth*";
constexpr char kFirebaseContentSettingsPattern[] =
    "https://[*.]firebaseapp.com/__/auth*";
// ContentSettingsPattern accepts [*.] as wildcard for subdomains, while
// URLPattern takes *.
constexpr char kFirebaseUrlPattern[] = "https://*.firebaseapp.com/__/auth*";
// We need this to delete cookies based on domain
// constexpr char kGoogleAuthDomain[] = "accounts.google.com";
// constexpr char kFirebaseAuthDomain[] = "firebaseapp.com";

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
    bool redo_request,
    const GURL& target_url,
    const content::Referrer& referrer,
    WindowOpenDisposition disposition,
    // content::RenderFrameHost* opener,
    content::WebContents* contents,
    const std::vector<blink::mojom::PermissionStatus>& permission_statuses) {
  DCHECK_EQ(1u, permission_statuses.size());
  auto permission_status = permission_statuses[0];
  auto embedding_pattern =
      ContentSettingsPattern::FromURLNoWildcard(request_initiator_url);

  bool granted = permission_status == blink::mojom::PermissionStatus::GRANTED;

  if (granted) {
    std::cout << "Setting 3p policy for " << request_initiator_url.spec()
              << " to be GRANTED" << std::endl;

    // Add 3p exception for request_initiator_url for auth patterns
    Set3pCookieException(content_settings.get(), embedding_pattern,
                         CONTENT_SETTING_ALLOW);
  } else if (permission_status == blink::mojom::PermissionStatus::DENIED) {
    std::cout << "Setting 3p policy for " << request_initiator_url.spec()
              << " to be DENIED" << std::endl;
    // Remove 3p exception for request_initiator_url for auth patterns
    Set3pCookieException(content_settings.get(), embedding_pattern,
                         CONTENT_SETTING_BLOCK);
  }
  if (granted && redo_request) {
    std::cout << "Redoing request" << std::endl;
    // print pending request
    std::cout << "Target URL: " << target_url << "\n";
    // referrer, frametreenodeid, disposition
    // although should the disposition be current tab?
    // could we just take the renderframehost opener?
    std::cout << "window open disposition is " << static_cast<int>(disposition)
              << "\n";
    content::OpenURLParams params(
        target_url, content::Referrer::SanitizeForRequest(target_url, referrer),
        contents->GetPrimaryMainFrame()->GetFrameTreeNodeId(), disposition,
        ui::PAGE_TRANSITION_AUTO_TOPLEVEL, false);
    base::SequencedTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::BindOnce(
                       [](base::WeakPtr<content::WebContents> web_contents,
                          const content::OpenURLParams& params) {
                         if (!web_contents)
                           return;
                         web_contents->OpenURL(params);
                       },
                       contents->GetWeakPtr(), std::move(params)));

    // contents->OpenURL(params); // probably need to do this in a task
  }
  return;
}

}  // namespace google_sign_in
