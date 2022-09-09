/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/base/url/url_util.h"

#include "base/ranges/algorithm.h"
#include "base/strings/pattern.h"
#include "base/strings/strcat.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "url/gurl.h"
#include "url/url_constants.h"

namespace ads {

namespace {

constexpr char kBraveScheme[] = "brave";
constexpr char kChromeScheme[] = "chrome";

constexpr char kRewardsHostName[] = "rewards";
constexpr char kSettingsHostName[] = "settings";
constexpr char kSyncHostName[] = "sync";
constexpr char kWalletHostName[] = "wallet";

constexpr char kSearchEnginesPath[] = "/searchEngines";
constexpr char kSearchPath[] = "/search";

GURL ReplaceBraveHostWithChromeHostForUrl(const GURL& url) {
  if (!url.SchemeIs(kBraveScheme)) {
    return url;
  }

  GURL::Replacements replacements;
  replacements.SetSchemeStr(kChromeScheme);
  return url.ReplaceComponents(replacements);
}

}  // namespace

GURL GetUrlWithEmptyQuery(const GURL& url) {
  return GURL(base::StrCat(
      {url.scheme(), url::kStandardSchemeSeparator, url.host(), url.path()}));
}

bool SchemeIsSupportedForUrl(const GURL& url) {
  if (url.SchemeIs(url::kHttpsScheme)) {
    return true;
  }

  // We must replace the brave:// host with chrome:// due to GURL not correctly
  // parsing brave:// hosts.
  const GURL modified_url = ReplaceBraveHostWithChromeHostForUrl(url);

  if (!modified_url.SchemeIs(kChromeScheme)) {
    return false;
  }

  const std::string host_name = modified_url.host();
  if (host_name == kRewardsHostName || host_name == kSyncHostName ||
      host_name == kWalletHostName) {
    return true;
  }

  if (host_name == kSettingsHostName) {
    if (modified_url.path() == kSearchEnginesPath) {
      return true;
    }

    if (modified_url.path() == kSearchPath) {
      return true;
    }
  }

  return false;
}

bool MatchUrlPattern(const GURL& url, const std::string& pattern) {
  if (!url.is_valid() || pattern.empty()) {
    return false;
  }

  return base::MatchPattern(url.spec(), pattern);
}

bool SameHostForUrl(const GURL& lhs, const GURL& rhs) {
  return lhs.host() == rhs.host();
}

bool HostForUrlExists(const std::vector<GURL>& lhs, const GURL& rhs) {
  return base::ranges::any_of(
      lhs, [&rhs](const GURL& url) { return SameHostForUrl(rhs, url); });
}

bool SameDomainOrHostForUrl(const GURL& lhs, const GURL& rhs) {
  return net::registry_controlled_domains::SameDomainOrHost(
      lhs, rhs, net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
}

bool DomainOrHostForUrlExists(const std::vector<GURL>& lhs, const GURL& rhs) {
  return base::ranges::any_of(lhs, [&rhs](const GURL& url) {
    return SameDomainOrHostForUrl(rhs, url);
  });
}

}  // namespace ads
