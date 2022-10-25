// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_VPN_VPN_DNS_HANDLER_H_
#define BRAVE_VPN_VPN_DNS_HANDLER_H_

#include <windows.h>

#include "base/memory/raw_ptr.h"
#include "brave/vpn/constants.h"

namespace brave_vpn {

class VpnDnsHandler {
 public:
  VpnDnsHandler();
  ~VpnDnsHandler();

  ErrorCodes SetFilters(const std::wstring& connection_name);
  ErrorCodes RemoveFilters(const std::wstring& connection_name);
  bool IsDNSFiltersActive() const;

 private:
  HANDLE engine_ = nullptr;
};

}  // namespace brave_vpn

#endif  // BRAVE_VPN_VPN_DNS_HANDLER_H_
