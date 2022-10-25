// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/vpn/vpn_dns_handler.h"

#include <string>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "brave/vpn/vpn_utils.h"

namespace brave_vpn {

VpnDnsHandler::VpnDnsHandler() = default;
VpnDnsHandler::~VpnDnsHandler() = default;

ErrorCodes VpnDnsHandler::SetFilters(const std::wstring& connection_name) {
  LOG(ERROR) << __func__ << ":" << connection_name;
  if (IsDNSFiltersActive()) {
    LOG(ERROR) << "Filters activated for:" << connection_name;
    return ErrorCodes::FILTERS_ALREADY_ACTIVATED;
  }

  engine_ = OpenEngineSession();
  if (!engine_) {
    return ErrorCodes::ADD_FILTERS_ERROR;
  }

  auto result = AddDnsFilters(engine_, base::WideToASCII(connection_name));
  if (result) {
    LOG(ERROR) << "AddDnsFilters error: " << std::hex << result;
    RemoveFilters(connection_name);
    return ErrorCodes::ADD_FILTERS_ERROR;
  }
  return ErrorCodes::OPERATION_SUCCESS;
}

bool VpnDnsHandler::IsDNSFiltersActive() const {
  return engine_ != nullptr;
}

ErrorCodes VpnDnsHandler::RemoveFilters(const std::wstring& connection_name) {
  LOG(ERROR) << __func__ << ":" << connection_name << ", engine:" << engine_;
  if (!IsDNSFiltersActive()) {
    LOG(ERROR) << "No active filters";
    return ErrorCodes::FILTERS_NOT_FOUND;
  }

  auto result = CloseEngineSession(engine_);
  engine_ = nullptr;
  return (result) ? ErrorCodes::REMOVE_FILTERS_ERROR
                  : ErrorCodes::OPERATION_SUCCESS;
}

}  // namespace brave_vpn
