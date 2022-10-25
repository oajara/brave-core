// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_VPN_VPN_UTILS_H_
#define BRAVE_VPN_VPN_UTILS_H_

#include <windows.h>
#include <string>

#include "base/files/file_path.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace brave_vpn {
DWORD AddDnsFilters(HANDLE engine_handle, const std::string& name);
HANDLE OpenEngineSession();
DWORD CloseEngineSession(HANDLE engine);

}  // namespace brave_vpn

#endif  // BRAVE_VPN_VPN_UTILS_H_
