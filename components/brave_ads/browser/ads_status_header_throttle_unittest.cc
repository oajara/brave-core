/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>

#include "base/strings/strcat.h"
#include "brave/components/brave_ads/browser/ads_service.h"
#include "brave/components/brave_ads/browser/ads_status_header_throttle.h"
#include "brave/components/brave_ads/browser/mock_ads_service.h"
#include "services/network/public/cpp/resource_request.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::Return;

namespace {

constexpr char kAdsStatusHeader[] = "X-Brave-Ads-Enabled";
constexpr char kAdsEnabledStatusValue[] = "1";
constexpr const char* kAllowedHosts[] = {
    "search.brave.com",          "search.brave.software",
    "search.bravesoftware.com",  "safesearch.brave.com",
    "safesearch.brave.software", "safesearch.bravesoftware.com",
    "search-dev-local.brave.com"};
constexpr char kNotAllowedHost[] = "brave.com";
constexpr char kTestingHeaderName[] = "TestingHeaderName";
constexpr char kTestingHeaderValue[] = "TestingHeaderValue";

network::ResourceRequest BuildRequest(const char* host) {
  network::ResourceRequest request;
  const std::string url = base::StrCat({"https://", host, "/search"});
  request.url = GURL(url);
  request.is_outermost_main_frame = true;
  request.headers.SetHeader(kTestingHeaderName, kTestingHeaderValue);
  return request;
}

}  // namespace

namespace brave_ads {

class AdsStatusHeaderThrottleTest : public ::testing::Test {
 public:
  void SetUp() override {
    ON_CALL(mock_ads_service_, IsEnabled()).WillByDefault(Return(true));
  }

  const AdsService* GetAdsService() const { return &mock_ads_service_; }

 private:
  MockAdsService mock_ads_service_;
};

TEST_F(AdsStatusHeaderThrottleTest, AdsEnabledForAllowedHost) {
  for (const auto* allowed_host : kAllowedHosts) {
    network::ResourceRequest request = BuildRequest(allowed_host);
    auto throttle =
        AdsStatusHeaderThrottle::MaybeCreateThrottle(GetAdsService(), request);
    ASSERT_TRUE(throttle);
    bool defer = false;
    throttle->WillStartRequest(&request, &defer);
    EXPECT_FALSE(defer);
    std::string value;
    EXPECT_TRUE(request.headers.GetHeader(kAdsStatusHeader, &value));
    EXPECT_EQ(kAdsEnabledStatusValue, value);
    EXPECT_TRUE(request.headers.GetHeader(kTestingHeaderName, &value));
    EXPECT_EQ(kTestingHeaderValue, value);
  }
}

TEST_F(AdsStatusHeaderThrottleTest, AdsDisabledForAllowedHost) {
  const network::ResourceRequest request = BuildRequest(kAllowedHosts[0]);
  MockAdsService ads_service;
  EXPECT_CALL(ads_service, IsEnabled()).WillOnce(Return(false));
  auto throttle =
      AdsStatusHeaderThrottle::MaybeCreateThrottle(&ads_service, request);
  EXPECT_FALSE(throttle);
}

TEST_F(AdsStatusHeaderThrottleTest, IncognitoModeForAllowedHost) {
  const network::ResourceRequest request = BuildRequest(kAllowedHosts[0]);
  auto throttle =
      AdsStatusHeaderThrottle::MaybeCreateThrottle(nullptr, request);
  EXPECT_FALSE(throttle);
}

TEST_F(AdsStatusHeaderThrottleTest, AdsEnabledForNotAllowedHost) {
  network::ResourceRequest request = BuildRequest(kNotAllowedHost);
  auto throttle =
      AdsStatusHeaderThrottle::MaybeCreateThrottle(GetAdsService(), request);
  EXPECT_FALSE(throttle);
}

TEST_F(AdsStatusHeaderThrottleTest, NonOutermostMainFrameNavigation) {
  network::ResourceRequest request = BuildRequest(kAllowedHosts[0]);
  request.is_outermost_main_frame = false;
  auto throttle =
      AdsStatusHeaderThrottle::MaybeCreateThrottle(GetAdsService(), request);
  EXPECT_FALSE(throttle);
}

}  // namespace brave_ads
