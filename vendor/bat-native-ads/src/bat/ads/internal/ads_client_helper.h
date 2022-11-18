/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_CLIENT_HELPER_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_CLIENT_HELPER_H_

#include "bat/ads/ads_client.h"

namespace ads {

class AdsClientObserver;

class AdsClientHelper final {
 public:
  explicit AdsClientHelper(AdsClient* ads_client);

  AdsClientHelper(const AdsClientHelper& other) = delete;
  AdsClientHelper& operator=(const AdsClientHelper& other) = delete;

  AdsClientHelper(AdsClientHelper&& other) noexcept = delete;
  AdsClientHelper& operator=(AdsClientHelper&& other) noexcept = delete;

  ~AdsClientHelper();

  static AdsClient* GetInstance();

  static bool HasInstance();

  static void AddObserver(AdsClientObserver* observer);
  static void RemoveObserver(AdsClientObserver* observer);
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_CLIENT_HELPER_H_
