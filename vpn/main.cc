// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

#include <windows.h>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/process/memory.h"
#include "base/win/process_startup_helper.h"
#include "base/win/scoped_com_initializer.h"
#include "brave/vpn/service_main.h"
#include "brave/vpn/vpn_utils.h"

int main(int argc, char* argv[]) {
  // Initialize the CommandLine singleton from the environment.
  base::CommandLine::Init(0, nullptr);

  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_ALL;
  settings.log_file_path = L"D:\\1\\vpn.log";
  logging::InitLogging(settings);

  // The exit manager is in charge of calling the dtors of singletons.
  base::AtExitManager exit_manager;

  // Make sure the process exits cleanly on unexpected errors.
  base::EnableTerminationOnHeapCorruption();
  base::EnableTerminationOnOutOfMemory();
  base::win::RegisterInvalidParamHandler();
  auto* command_line = base::CommandLine::ForCurrentProcess();
  base::win::SetupCRT(*command_line);

  // Initialize COM for the current thread.
  base::win::ScopedCOMInitializer com_initializer(
      base::win::ScopedCOMInitializer::kMTA);
  if (!com_initializer.Succeeded()) {
    PLOG(ERROR) << "Failed to initialize COM";
    return -1;
  }

  // Run the COM service.
  brave_vpn::ServiceMain* service = brave_vpn::ServiceMain::GetInstance();
  if (!service->InitWithCommandLine(command_line))
    return -1;

  return service->Start();
}
