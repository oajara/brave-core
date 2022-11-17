/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/install_static/install_util.h"

#include "build/branding_buildflags.h"
#include "chrome/chrome_elf/nt_registry/nt_registry.h"
#include "chrome/install_static/buildflags.h"
#include "chrome/install_static/install_details.h"
#include "chrome/install_static/install_modes.h"
#include "chrome/install_static/policy_path_parser.h"
#include "chrome/install_static/user_data_dir.h"
#include "components/nacl/common/buildflags.h"
#include "components/version_info/channel.h"

#if defined(OFFICIAL_BUILD)
#undef BUILDFLAG_INTERNAL_GOOGLE_CHROME_BRANDING
#define BUILDFLAG_INTERNAL_GOOGLE_CHROME_BRANDING() (1)
#undef BUILDFLAG_INTERNAL_USE_GOOGLE_UPDATE_INTEGRATION
#define BUILDFLAG_INTERNAL_USE_GOOGLE_UPDATE_INTEGRATION() (1)
#endif

namespace {
constexpr IID kVpnServiceIID = {
    0xA949CB4E,
    0xC4F9,
    0x44C4,
    {0xB2, 0x13, 0x6B, 0xF8, 0xAA, 0x9A, 0xC6, 0x9A}};

constexpr CLSID kVpnServiceCLSID = {
    0x0014D784,
    0x7012,
    0x4A79,
    {0x8A, 0xB6, 0xAD, 0xDB, 0x81, 0x93, 0xA0, 0x6A}};

const wchar_t kVpnServiceName[] = L"BraveVPNService";

}  // namespace

#define GetChromeChannel GetChromeChannel_ChromiumImpl

#include "src/chrome/install_static/install_util.cc"

#undef GetChromeChannel

#if defined(OFFICIAL_BUILD)
#undef BUILDFLAG_INTERNAL_GOOGLE_CHROME_BRANDING
#undef BUILDFLAG_INTERNAL_USE_GOOGLE_UPDATE_INTEGRATION
#endif

namespace install_static {
const IID& GetVpnServiceIid() {
  return kVpnServiceIID;
}

const CLSID& GetVpnServiceClsid() {
  return kVpnServiceCLSID;
}

std::wstring GetVpnServiceName() {
  return kVpnServiceName;
}

std::wstring GetVpnServiceDisplayName() {
  static constexpr wchar_t kBraveVpnServiceDisplayName[] = L" Vpn Service";
  return GetBaseAppName() + kBraveVpnServiceDisplayName;
}

version_info::Channel GetChromeChannel() {
#if defined(OFFICIAL_BUILD)
  std::wstring channel_name(
      GetChromeChannelName(/*with_extended_stable=*/false));
  if (channel_name == L"nightly")
    return version_info::Channel::CANARY;
  return GetChromeChannel_ChromiumImpl();
#else
  return version_info::Channel::UNKNOWN;
#endif
}

}  // namespace install_static
