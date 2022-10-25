// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_VPN_VPN_SERVICE_H_
#define BRAVE_VPN_VPN_SERVICE_H_

#include <windows.h>
#include <wrl/implements.h>
#include <wrl/module.h>
#include <string>

#include "base/callback.h"
#include "base/gtest_prod_util.h"
#include "base/win/windows_types.h"
#include "brave/vpn/vpn_service_idl.h"

namespace brave_vpn {

class VpnService
    : public Microsoft::WRL::RuntimeClass<
          Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
          IVpnService> {
 public:
  VpnService();

  VpnService(const VpnService&) = delete;
  VpnService& operator=(const VpnService&) = delete;

  IFACEMETHODIMP EnableDNSFilters(const BSTR connection_name,
                                  /* [out] */ DWORD* error_code) override;
  IFACEMETHODIMP DisableDNSFilters(const BSTR connection_name,
                                   /* [out] */ DWORD* error_code) override;

 private:
  FRIEND_TEST_ALL_PREFIXES(VpnServiceTest, StringHandlingTest);
  ~VpnService() override;
};

// Returns a singleton application object bound to this COM server.
scoped_refptr<VpnService> VpnServiceSingletonInstance();

}  // namespace brave_vpn

#endif  // BRAVE_VPN_VPN_SERVICE_H_
