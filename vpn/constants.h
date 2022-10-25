// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_VPN_CONSTANTS_H_
#define BRAVE_VPN_CONSTANTS_H_

#include <guiddef.h>
#include <string>

namespace brave_vpn {

constexpr wchar_t kBraveVPNServiceName[] = L"BraveVPNService";
const wchar_t kVpnServiceRegistryStoragePath[] =
    L"Software\\BraveSoftware\\Brave\\Vpn";

enum ErrorCodes {
  OPERATION_SUCCESS = 0,
  FILTERS_ALREADY_ACTIVATED,
  ADAPTER_NOT_FOUND,
  FILTERS_NOT_FOUND,
  REMOVE_FILTERS_ERROR,
  ADD_FILTERS_ERROR,
  INTERNAL_ERROR,
  SERVICE_IS_NULL,
  DNS_HANLDER_IS_NULL,
};

const IID& GetVpnServiceIid();
const CLSID& GetVpnServiceClsid();
const std::wstring GetVpnServiceDisplayName();
const GUID GetVpnDnsSublayerGUID();

}  // namespace brave_vpn

#endif  // BRAVE_VPN_CONSTANTS_H_
