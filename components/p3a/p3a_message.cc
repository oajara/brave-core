/* Copyright 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/p3a/p3a_message.h"

#include <algorithm>
#include <array>
#include <vector>

#include "base/containers/flat_set.h"
#include "base/i18n/timezone.h"
#include "base/logging.h"
#include "base/strings/strcat.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "brave/components/brave_referrals/buildflags/buildflags.h"
#include "brave/components/brave_referrals/common/pref_names.h"
#include "brave/components/brave_stats/browser/brave_stats_updater_util.h"
#include "brave/components/p3a/brave_p3a_uploader.h"
#include "brave/components/version_info/version_info.h"

namespace brave {

namespace {
constexpr std::size_t kP3AStarAttributeCount = 8;
}  // namespace

MessageMetainfo::MessageMetainfo() = default;
MessageMetainfo::~MessageMetainfo() = default;

base::Value::Dict GenerateP3AMessageDict(base::StringPiece metric_name,
                                         uint64_t metric_value,
                                         const MessageMetainfo& meta,
                                         const std::string& upload_type) {
  base::Value::Dict result;

  // Fill basic meta.
  result.Set("platform", meta.platform);
  result.Set("channel", meta.channel);
  // Set the metric
  result.Set("metric_name", metric_name);
  result.Set("metric_value", static_cast<int>(metric_value));

  if (upload_type == kP3ACreativeUploadType) {
    return result;
  }

  // Find out years of install and survey.
  base::Time::Exploded exploded;
  meta.date_of_survey.LocalExplode(&exploded);
  DCHECK_GE(exploded.year, 999);
  result.Set("yos", exploded.year);

  meta.date_of_install.LocalExplode(&exploded);
  DCHECK_GE(exploded.year, 999);
  result.Set("yoi", exploded.year);

  // Fill meta.
  result.Set("country_code", meta.country_code);
  result.Set("version", meta.version);
  result.Set("woi", meta.woi);
  result.Set("wos", meta.wos);

  return result;
}

std::string GenerateP3AStarMessage(base::StringPiece metric_name,
                                   uint64_t metric_value,
                                   const MessageMetainfo& meta) {
  base::Time::Exploded exploded;
  meta.date_of_install.LocalExplode(&exploded);
  DCHECK_GE(exploded.year, 999);

  std::array<std::array<std::string, 2>, kP3AStarAttributeCount> attributes = {{
      {"metric_name", std::string(metric_name)},
      {"metric_value", base::NumberToString(metric_value)},
      {"version", meta.version},
      {"yoi", base::NumberToString(exploded.year)},
      {"channel", meta.channel},
      {"platform", meta.platform},
      {"country_code", meta.country_code},
      {"woi", base::NumberToString(meta.woi)},
  }};

  std::array<std::string, kP3AStarAttributeCount> serialized_attributes;

  std::transform(attributes.begin(), attributes.end(),
                 serialized_attributes.begin(), [](auto& attr) -> std::string {
                   return base::JoinString(attr,
                                           kP3AMessageStarKeyValueSeparator);
                 });

  return base::JoinString(serialized_attributes, kP3AMessageStarLayerSeparator);
}

void MessageMetainfo::Init(PrefService* local_state,
                           std::string channel_type,
                           std::string week_of_install) {
  platform = brave_stats::GetPlatformIdentifier();
  channel = channel_type;
  InitVersion();

  if (!week_of_install.empty()) {
    date_of_install = brave_stats::GetYMDAsDate(week_of_install);
  } else {
    date_of_install = base::Time::Now();
  }
  woi = brave_stats::GetIsoWeekNumber(date_of_install);

  country_code = base::ToUpperASCII(base::CountryCodeForCurrentTimezone());
#if BUILDFLAG(ENABLE_BRAVE_REFERRALS)
  refcode = local_state->GetString(kReferralPromoCode);
#endif
  MaybeStripRefcodeAndCountry();

  Update();

  VLOG(2) << "Message meta: " << platform << " " << channel << " " << version
          << " " << woi << " " << wos << " " << country_code << " " << refcode;
}

void MessageMetainfo::Update() {
  date_of_survey = base::Time::Now();
  wos = brave_stats::GetIsoWeekNumber(date_of_survey);
}

void MessageMetainfo::InitVersion() {
  std::string full_version =
      version_info::GetBraveVersionWithoutChromiumMajorVersion();
  std::vector<std::string> version_numbers = base::SplitString(
      full_version, ".", base::WhitespaceHandling::TRIM_WHITESPACE,
      base::SplitResult::SPLIT_WANT_ALL);
  if (version_numbers.size() <= 2) {
    version = full_version;
  } else {
    version = base::StrCat({version_numbers[0], ".", version_numbers[1]});
  }
}

void MessageMetainfo::MaybeStripRefcodeAndCountry() {
  const std::string& country = country_code;
  constexpr char kRefcodeNone[] = "none";
  constexpr char kCountryOther[] = "other";

  static base::flat_set<std::string> const kLinuxCountries(
      {"US", "FR", "DE", "GB", "IN", "BR", "PL", "NL", "ES", "CA", "IT", "AU",
       "MX", "CH", "RU", "ZA", "SE", "BE", "JP"});

  static base::flat_set<std::string> const kNotableCountries(
      {"US", "FR", "PH", "GB", "IN", "DE", "BR", "CA", "IT", "ES", "NL", "MX",
       "AU", "RU", "JP", "PL", "ID", "KR", "AR"});

  // Always strip the refcode.
  // We no longer need to partition P3A data with that key.
  refcode = kRefcodeNone;

  if (platform == "linux-bc") {
    // If we have more than 3/0.05 = 60 users in a country for
    // a week of install, we can send country.
    if (kLinuxCountries.count(country) == 0) {
      country_code = kCountryOther;
    }
  } else {
    // Now the minimum platform is MacOS at ~3%, so cut off for a group under
    // here becomes 3/(0.05*0.03) = 2000.
    if (kNotableCountries.count(country) == 0) {
      country_code = kCountryOther;
    }
  }
}

}  // namespace brave
