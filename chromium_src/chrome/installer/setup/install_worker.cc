/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <shlobj.h>

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/version.h"
#include "base/win/windows_types.h"
#include "chrome/install_static/install_util.h"
#include "chrome/installer/setup/setup_util.h"
#include "chrome/installer/util/callback_work_item.h"
#include "chrome/installer/util/install_service_work_item.h"
#include "chrome/installer/util/work_item_list.h"

#if defined(OFFICIAL_BUILD)
#include "chrome/install_static/buildflags.h"
#include "chrome/install_static/install_constants.h"
#undef BUILDFLAG_INTERNAL_USE_GOOGLE_UPDATE_INTEGRATION
#define BUILDFLAG_INTERNAL_USE_GOOGLE_UPDATE_INTEGRATION() (1)
#endif
namespace {
const wchar_t kBraveVPNServicePath[] = L"vpn.exe";
const wchar_t kVpnServiceRegistryStoragePath[] =
    L"Software\\BraveSoftware\\Brave\\Vpn";

base::FilePath GetBraveVPNServicePath(const base::FilePath& target_path,
                                      const base::Version& version) {
  return target_path.AppendASCII(version.GetString())
      .Append(kBraveVPNServicePath);
}

bool ConfigureServiceAutoRestart(const CallbackWorkItem& item) {
  SC_HANDLE scm = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
  if (!scm)
    return false;
  SC_HANDLE service = ::OpenService(
      scm, install_static::GetVpnServiceName().c_str(), SERVICE_ALL_ACCESS);
  if (!service) {
    return ::CloseServiceHandle(scm) != FALSE;
  }

  SC_ACTION failActions[] = {
      {SC_ACTION_RESTART, 1}, {SC_ACTION_RESTART, 1}, {SC_ACTION_RESTART, 1}};
  // The time after which to reset the failure count to zero if there are no
  // failures, in seconds. 86400 in sec = 1 day.
  SERVICE_FAILURE_ACTIONS servFailActions = {
      .dwResetPeriod = 86400,
      .lpRebootMsg = NULL,
      .lpCommand = NULL,
      .cActions = sizeof(failActions) / sizeof(SC_ACTION),
      .lpsaActions = failActions};
  auto result = ChangeServiceConfig2(service, SERVICE_CONFIG_FAILURE_ACTIONS,
                                     &servFailActions);
  if (!result) {
    LOG(ERROR) << "ChangeServiceConfig2 failed:" << GetLastError();
  }
  if (!::CloseServiceHandle(service)) {
    LOG(ERROR) << "CloseServiceHandle service failed:" << GetLastError();
  }
  if (!::CloseServiceHandle(scm)) {
    LOG(ERROR) << "CloseServiceHandle scm failed:" << GetLastError();
  }
  return result;
}
// Adds work items to register the Elevation Service with Windows. Only for
// system level installs.
void AddBraveVPNServiceWorkItems(const base::FilePath& vpn_service_path,
                                 WorkItemList* list) {
  DCHECK(::IsUserAnAdmin());

  if (vpn_service_path.empty()) {
    LOG(DFATAL) << "The path to vpn.exe is invalid.";
    return;
  }

  WorkItem* install_service_work_item = new installer::InstallServiceWorkItem(
      install_static::GetVpnServiceName(),
      install_static::GetVpnServiceDisplayName(), SERVICE_DEMAND_START,
      base::CommandLine(vpn_service_path),
      base::CommandLine(base::CommandLine::NO_PROGRAM),
      kVpnServiceRegistryStoragePath, {install_static::GetVpnServiceClsid()},
      {install_static::GetVpnServiceIid()});
  install_service_work_item->set_best_effort(true);
  list->AddWorkItem(install_service_work_item);
  list->AddCallbackWorkItem(base::BindOnce(&ConfigureServiceAutoRestart),
                            base::NullCallback());
}

}  // namespace

#define GetElevationServicePath GetElevationServicePath(target_path, new_version), install_list); \
  AddBraveVPNServiceWorkItems(GetBraveVPNServicePath
#include "src/chrome/installer/setup/install_worker.cc"
#undef GetElevationServicePath

#if defined(OFFICIAL_BUILD)
#undef BUILDFLAG_INTERNAL_USE_GOOGLE_UPDATE_INTEGRATION
#endif
