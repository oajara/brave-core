/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_IPFS_PIN_IPFS_BASE_PIN_SERVICE_H_
#define BRAVE_COMPONENTS_IPFS_PIN_IPFS_BASE_PIN_SERVICE_H_

#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "brave/components/ipfs/ipfs_service.h"
#include "components/prefs/pref_change_registrar.h"
#include "components/prefs/pref_service.h"

namespace ipfs {

class IpfsBaseJob {
 public:
  IpfsBaseJob();
  virtual ~IpfsBaseJob();
  virtual void Start() = 0;
};

class IpfsBasePinService : public IpfsServiceObserver {
 public:
  IpfsBasePinService(PrefService* pref_service, IpfsService* service);
  ~IpfsBasePinService() override;

  void AddJob(std::unique_ptr<IpfsBaseJob> job);
  void OnJobDone(bool result);

  void OnIpfsShutdown() override;
  void OnGetConnectedPeers(bool succes,
                           const std::vector<std::string>& peers) override;

 private:
  bool IsDaemonReady();
  void MaybeStartDaemon();
  void OnDaemonStarted();
  void DoNextJob();

  bool daemon_ready_ = false;
  PrefService* pref_service_;
  IpfsService* ipfs_service_;
  PrefChangeRegistrar pref_change_registrar_;
  std::unique_ptr<IpfsBaseJob> current_job_;
  std::queue<std::unique_ptr<IpfsBaseJob>> jobs_;
};

}  // namespace ipfs

#endif  // BRAVE_COMPONENTS_IPFS_PIN_IPFS_BASE_PIN_SERVICE_H_
