// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/vpn/constants.h"

namespace {
/* UUID of WFP sublayer used by all instances of openvpn
 * 23e10e29-eb83-4d2c-9d77-f6e9b547f39c */
constexpr GUID kVpnDnsSublayerGUID = {
    0x23e10e29,
    0xeb83,
    0x4d2c,
    {0x9d, 0x77, 0xf6, 0xe9, 0xb5, 0x47, 0xf3, 0x9c}};

}  // namespace

namespace brave_vpn {

const GUID GetVpnDnsSublayerGUID() {
  return kVpnDnsSublayerGUID;
}

}  // namespace brave_vpn
