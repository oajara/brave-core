// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/vpn/vpn_service.h"

#include "base/logging.h"
#include "base/win/win_util.h"
#include "brave/vpn/constants.h"
#include "brave/vpn/service_main.h"
#include "brave/vpn/vpn_dns_handler.h"

namespace brave_vpn {

VpnService::VpnService() = default;
VpnService::~VpnService() = default;

HRESULT VpnService::EnableDNSFilters(const BSTR connection_name,
                                     DWORD* error_code) {
  LOG(ERROR) << this << " " << __func__ << ":" << connection_name;
  std::wstring name(connection_name, SysStringLen(connection_name));

  ServiceMain* self = ServiceMain::GetInstance();
  if (!self) {
    LOG(ERROR) << "Service not found";
    *error_code = brave_vpn::ErrorCodes::SERVICE_IS_NULL;
    return S_OK;
  }
  if (!self->GetDNSHandler()) {
    LOG(ERROR) << "DNS handler not found";
    *error_code = brave_vpn::ErrorCodes::DNS_HANLDER_IS_NULL;
    return S_OK;
  }

  *error_code = self->GetDNSHandler()->SetFilters(name);
  return S_OK;
}

HRESULT VpnService::DisableDNSFilters(const BSTR connection_name,
                                      DWORD* error_code) {
  LOG(ERROR) << this << " " << __func__ << ":" << connection_name;
  ServiceMain* self = ServiceMain::GetInstance();
  if (!self) {
    LOG(ERROR) << "Service not found";
    *error_code = brave_vpn::ErrorCodes::SERVICE_IS_NULL;
    return S_OK;
  }
  if (!self->GetDNSHandler()) {
    LOG(ERROR) << "DNS handler not found";
    *error_code = brave_vpn::ErrorCodes::DNS_HANLDER_IS_NULL;
    return S_OK;
  }
  std::wstring name(connection_name, SysStringLen(connection_name));
  *error_code = self->GetDNSHandler()->RemoveFilters(name);
  LOG(ERROR) << __func__ << ":"
             << "SignalExit";
  self->SignalExit();
  return S_OK;
}

}  // namespace brave_vpn
