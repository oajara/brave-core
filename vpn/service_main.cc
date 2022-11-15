// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

// This macro is used in <wrl/module.h>. Since only the COM functionality is
// used here (while WinRT isn't being used), define this macro to optimize
// compilation of <wrl/module.h> for COM-only.
#ifndef __WRL_CLASSIC_COM_STRICT__
#define __WRL_CLASSIC_COM_STRICT__
#endif  // __WRL_CLASSIC_COM_STRICT__

#include "brave/vpn/service_main.h"

#include <atlsecurity.h>
#include <iphlpapi.h>
#include <netioapi.h>
#include <sddl.h>
#include <wrl/module.h>
#include <type_traits>

#include "base/callback.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/run_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/single_thread_task_executor.h"
#include "base/threading/thread_restrictions.h"
#include "base/win/scoped_com_initializer.h"
#include "brave/vpn/constants.h"
#include "brave/vpn/vpn_dns_handler.h"
#include "brave/vpn/vpn_service.h"
#include "chrome/install_static/install_util.h"

namespace brave_vpn {

namespace {

// Command line switch "--console" runs the service interactively for
// debugging purposes.
constexpr char kConsoleSwitchName[] = "console";

}  // namespace

ServiceMain* ServiceMain::GetInstance() {
  static base::NoDestructor<ServiceMain> instance;
  return instance.get();
}

bool ServiceMain::InitWithCommandLine(const base::CommandLine* command_line) {
  const base::CommandLine::StringVector args = command_line->GetArgs();
  if (!args.empty()) {
    LOG(ERROR) << "No positional parameters expected.";
    return false;
  }

  // Run interactively if needed.
  if (command_line->HasSwitch(kConsoleSwitchName))
    run_routine_ = &ServiceMain::RunInteractive;

  return true;
}

// Start() is the entry point called by WinMain.
int ServiceMain::Start() {
  return (this->*run_routine_)();
}

void ServiceMain::CreateWRLModule() {
  Microsoft::WRL::Module<Microsoft::WRL::OutOfProc>::Create(
      this, &ServiceMain::SignalExit);
}

// When _ServiceMain gets called, it initializes COM, and then calls Run().
// Run() initializes security, then calls RegisterClassObject().
HRESULT ServiceMain::RegisterClassObject() {
  LOG(ERROR) << __func__;
  auto& module = Microsoft::WRL::Module<Microsoft::WRL::OutOfProc>::GetModule();

  // We hand-register a unique CLSID for each Chrome channel.
  Microsoft::WRL::ComPtr<IUnknown> factory;
  unsigned int flags = Microsoft::WRL::ModuleType::OutOfProc;

  HRESULT hr = Microsoft::WRL::Details::CreateClassFactory<
      Microsoft::WRL::SimpleClassFactory<VpnService>>(
      &flags, nullptr, __uuidof(IClassFactory), &factory);
  if (FAILED(hr)) {
    LOG(ERROR) << "Factory creation failed; hr: " << hr;
    return hr;
  }

  Microsoft::WRL::ComPtr<IClassFactory> class_factory;
  hr = factory.As(&class_factory);
  if (FAILED(hr)) {
    LOG(ERROR) << "IClassFactory object creation failed; hr: " << hr;
    return hr;
  }

  // The pointer in this array is unowned. Do not release it.
  IClassFactory* class_factories[] = {class_factory.Get()};
  static_assert(std::extent<decltype(cookies_)>() == std::size(class_factories),
                "Arrays cookies_ and class_factories must be the same size.");

  IID class_ids[] = {install_static::GetVpnServiceClsid()};
  // IID class_ids[] = {brave_vpn::GetVpnServiceIid()};

  DCHECK_EQ(std::size(cookies_), std::size(class_ids));
  static_assert(std::extent<decltype(cookies_)>() == std::size(class_ids),
                "Arrays cookies_ and class_ids must be the same size.");

  hr = module.RegisterCOMObject(nullptr, class_ids, class_factories, cookies_,
                                std::size(cookies_));
  if (FAILED(hr)) {
    LOG(ERROR) << "RegisterCOMObject failed; hr: " << std::hex << hr;
    return hr;
  }
  LOG(ERROR) << __func__ << ":" << std::hex << hr;
  return hr;
}

void ServiceMain::UnregisterClassObject() {
  LOG(ERROR) << __func__;
  auto& module = Microsoft::WRL::Module<Microsoft::WRL::OutOfProc>::GetModule();
  const HRESULT hr =
      module.UnregisterCOMObject(nullptr, cookies_, std::size(cookies_));
  if (FAILED(hr))
    LOG(ERROR) << "UnregisterCOMObject failed; hr: " << hr;
}

bool ServiceMain::IsExitSignaled() {
  LOG(ERROR) << __func__;
  return false;  // exit_signal_.IsSignaled();
}

ServiceMain::ServiceMain()
    : run_routine_(&ServiceMain::RunAsService),
      service_status_handle_(nullptr),
      service_status_(),
      cookies_(),
      exit_signal_(base::WaitableEvent::ResetPolicy::MANUAL,
                   base::WaitableEvent::InitialState::NOT_SIGNALED) {
  service_status_.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  service_status_.dwCurrentState = SERVICE_STOPPED;
  service_status_.dwControlsAccepted = SERVICE_ACCEPT_STOP;
}

ServiceMain::~ServiceMain() = default;

int ServiceMain::RunAsService() {
  const std::wstring& service_name(install_static::GetVpnServiceName());
  const SERVICE_TABLE_ENTRY dispatch_table[] = {
      {const_cast<LPTSTR>(service_name.c_str()),
       &ServiceMain::ServiceMainEntry},
      {nullptr, nullptr}};

  if (!::StartServiceCtrlDispatcher(dispatch_table)) {
    service_status_.dwWin32ExitCode = ::GetLastError();
    LOG(ERROR) << "Failed to connect to the service control manager:"
               << service_status_.dwWin32ExitCode;
  }

  return service_status_.dwWin32ExitCode;
}

void ServiceMain::ServiceMainImpl() {
  LOG(ERROR) << __func__ << " BraveVPN Service started";
  service_status_handle_ =
      ::RegisterServiceCtrlHandler(install_static::GetVpnServiceName().c_str(),
                                   &ServiceMain::ServiceControlHandler);
  if (service_status_handle_ == nullptr) {
    LOG(ERROR) << "RegisterServiceCtrlHandler failed";
    return;
  }
  SetServiceStatus(SERVICE_RUNNING);

  service_status_.dwWin32ExitCode = ERROR_SUCCESS;
  service_status_.dwCheckPoint = 0;
  service_status_.dwWaitHint = 0;

  // Initialize COM for the current thread.
  base::win::ScopedCOMInitializer com_initializer(
      base::win::ScopedCOMInitializer::kMTA);
  if (!com_initializer.Succeeded()) {
    LOG(ERROR) << "Failed to initialize COM";
    SetServiceStatus(SERVICE_STOPPED);
    return;
  }

  // When the Run function returns, the service has stopped.
  const HRESULT hr = Run();
  if (FAILED(hr)) {
    service_status_.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    service_status_.dwServiceSpecificExitCode = hr;
  }

  SetServiceStatus(SERVICE_STOPPED);
}

int ServiceMain::RunInteractive() {
  return Run();
}

// static
void ServiceMain::ServiceControlHandler(DWORD control) {
  ServiceMain* self = ServiceMain::GetInstance();
  LOG(ERROR) << __func__ << ":" << control;
  switch (control) {
    case SERVICE_CONTROL_STOP:

      self->SignalExit();
      break;

    default:
      break;
  }
}

// static
void WINAPI ServiceMain::ServiceMainEntry(DWORD argc, wchar_t* argv[]) {
  ServiceMain::GetInstance()->ServiceMainImpl();
}

void ServiceMain::SetServiceStatus(DWORD state) {
  ::InterlockedExchange(&service_status_.dwCurrentState, state);
  ::SetServiceStatus(service_status_handle_, &service_status_);
}

HRESULT ServiceMain::Run() {
  LOG(ERROR) << __func__ << " BraveVPN Service";
  HRESULT hr = InitializeComSecurity();
  if (FAILED(hr))
    return hr;

  CreateWRLModule();
  hr = RegisterClassObject();
  if (SUCCEEDED(hr)) {
    LOG(ERROR) << __func__ << " WaitForExitSignal";
    LOG(ERROR) << "Ready";
    WaitForExitSignal();
    LOG(ERROR) << "Stopping....";
    UnregisterClassObject();
  }

  return hr;
}

// static
HRESULT ServiceMain::InitializeComSecurity() {
  LOG(ERROR) << __func__;
  CDacl dacl;
  constexpr BYTE com_rights_execute_local =
      COM_RIGHTS_EXECUTE | COM_RIGHTS_EXECUTE_LOCAL;
  dacl.AddAllowedAce(Sids::System(), com_rights_execute_local);
  dacl.AddAllowedAce(Sids::Admins(), com_rights_execute_local);
  dacl.AddAllowedAce(Sids::Interactive(), com_rights_execute_local);

  CSecurityDesc sd;
  sd.SetDacl(dacl);
  sd.MakeAbsolute();
  sd.SetOwner(Sids::Admins());
  sd.SetGroup(Sids::Admins());

  return ::CoInitializeSecurity(
      const_cast<SECURITY_DESCRIPTOR*>(sd.GetPSECURITY_DESCRIPTOR()), -1,
      nullptr, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IDENTIFY,
      nullptr, EOAC_DYNAMIC_CLOAKING | EOAC_NO_CUSTOM_MARSHAL, nullptr);
}

void ServiceMain::WaitForExitSignal() {
  LOG(ERROR) << this << ":" << __func__;

  dns_handler_ = std::make_unique<brave_vpn::VpnDnsHandler>();
  // Make and keep live(while the service is live) a COM instance to control
  // service lifetime manually.
  auto service = Microsoft::WRL::Make<VpnService>();

  exit_signal_.Wait();
}

void ServiceMain::SignalExit() {
  exit_signal_.Signal();
}

VpnDnsHandler* ServiceMain::GetDNSHandler() {
  return dns_handler_.get();
}

}  // namespace brave_vpn
