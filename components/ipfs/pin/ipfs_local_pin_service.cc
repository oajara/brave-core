/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/ipfs/pin/ipfs_local_pin_service.h"

#include <memory>
#include <string>
#include <utility>

#include "brave/components/ipfs/pref_names.h"
#include "components/prefs/scoped_user_pref_update.h"

namespace ipfs {

namespace {
const char kRecursiveMode[] = "recursive";
}  // namespace

AddLocalPinJob::AddLocalPinJob(PrefService* prefs_service,
                               IpfsService* ipfs_service,
                               const std::string& prefix,
                               const std::vector<std::string>& cids,
                               AddPinCallback callback)
    : prefs_service_(prefs_service),
      ipfs_service_(ipfs_service),
      prefix_(prefix),
      cids_(cids),
      callback_(std::move(callback)) {}

AddLocalPinJob::~AddLocalPinJob() {}

void AddLocalPinJob::Start() {
  ipfs_service_->AddPin(
      cids_, true,
      base::BindOnce(&AddLocalPinJob::OnAddPinResult, base::Unretained(this)));
}

void AddLocalPinJob::OnAddPinResult(bool status,
                                    absl::optional<AddPinResult> result) {
  if (status && result) {
    for (const auto& cid : cids_) {
      if (std::find(result->pins.begin(), result->pins.end(), cid) ==
          result->pins.end()) {
        std::move(callback_).Run(false);
        return;
      }
    }

    {
      DictionaryPrefUpdate update(prefs_service_, kIPFSPinnedCids);
      base::Value::Dict& update_dict = update->GetDict();

      for (const auto& cid : cids_) {
        base::Value::List* list = update_dict.FindList(cid);
        if (!list) {
          update_dict.Set(cid, base::Value::List());
          list = update_dict.FindList(cid);
        }
        DCHECK(list);
        list->EraseValue(base::Value(prefix_));
        list->Append(base::Value(prefix_));
      }
    }
    std::move(callback_).Run(true);
  } else {
    std::move(callback_).Run(false);
  }
}

RemoveLocalPinJob::RemoveLocalPinJob(PrefService* prefs_service,
                                     const std::string& prefix,
                                     RemovePinCallback callback)
    : prefs_service_(prefs_service),
      prefix_(prefix),
      callback_(std::move(callback)) {}

RemoveLocalPinJob::~RemoveLocalPinJob() {}

void RemoveLocalPinJob::Start() {
  {
    DictionaryPrefUpdate update(prefs_service_, kIPFSPinnedCids);
    base::Value::Dict& update_dict = update->GetDict();

    std::vector<std::string> remove_list;
    for (auto pair : update_dict) {
      base::Value::List* list = pair.second.GetIfList();
      if (list) {
        list->EraseValue(base::Value(prefix_));
        if (list->empty()) {
          remove_list.push_back(pair.first);
        }
      }
    }
    for (const auto& cid : remove_list) {
      update_dict.Remove(cid);
    }
  }
  std::move(callback_).Run(true);
}

VerifyLocalPinJob::VerifyLocalPinJob(PrefService* prefs_service,
                                     IpfsService* ipfs_service,
                                     const std::string& prefix,
                                     const std::vector<std::string>& cids,
                                     ValidatePinsCallback callback)
    : prefs_service_(prefs_service),
      ipfs_service_(ipfs_service),
      prefix_(prefix),
      cids_(cids),
      callback_(std::move(callback)) {}

VerifyLocalPinJob::~VerifyLocalPinJob() {}

void VerifyLocalPinJob::Start() {
  ipfs_service_->GetPins(absl::nullopt, kRecursiveMode, true,
                         base::BindOnce(&VerifyLocalPinJob::OnGetPinsResult,
                                        base::Unretained(this)));
}

void VerifyLocalPinJob::OnGetPinsResult(bool status,
                                        absl::optional<GetPinsResult> result) {
  if (status && result) {
    DictionaryPrefUpdate update(prefs_service_, kIPFSPinnedCids);
    base::Value::Dict& update_dict = update->GetDict();

    bool verification_pased = true;
    for (const auto& cid : cids_) {
      base::Value::List* list = update_dict.FindList(cid);
      if (!list) {
        verification_pased = false;
      } else {
        if (result->find(cid) != result->end()) {
          list->EraseValue(base::Value(prefix_));
          list->Append(base::Value(prefix_));
        } else {
          verification_pased = false;
          list->EraseValue(base::Value(prefix_));
        }
        if (list->empty()) {
          update_dict.Remove(cid);
        }
      }
    }
    std::move(callback_).Run(verification_pased);
  } else {
    std::move(callback_).Run(absl::nullopt);
  }
}

GcJob::GcJob(PrefService* prefs_service,
             IpfsService* ipfs_service,
             GcCallback callback)
    : prefs_service_(prefs_service),
      ipfs_service_(ipfs_service),
      callback_(std::move(callback)) {}
GcJob::~GcJob() {}

void GcJob::Start() {
  ipfs_service_->GetPins(
      absl::nullopt, kRecursiveMode, true,
      base::BindOnce(&GcJob::OnGetPinsResult, base::Unretained(this)));
}

void GcJob::OnGetPinsResult(bool status, absl::optional<GetPinsResult> result) {
  std::vector<std::string> cids_to_delete;
  if (status && result) {
    const base::Value::Dict& dict = prefs_service_->GetDict(kIPFSPinnedCids);
    for (const auto& pair : result.value()) {
      const base::Value::List* list = dict.FindList(pair.first);
      if (!list || list->empty()) {
        cids_to_delete.push_back(pair.first);
      }
    }

    if (!cids_to_delete.empty()) {
      ipfs_service_->RemovePin(
          cids_to_delete,
          base::BindOnce(&GcJob::OnPinsRemovedResult, base::Unretained(this)));
    } else {
      std::move(callback_).Run(true);
    }
  } else {
    std::move(callback_).Run(false);
  }
}

void GcJob::OnPinsRemovedResult(bool status,
                                absl::optional<RemovePinResult> result) {
  if (status && result) {
    std::move(callback_).Run(true);
  } else {
    std::move(callback_).Run(false);
  }
}

IpfsLocalPinService::IpfsLocalPinService(PrefService* prefs_service,
                                         IpfsService* ipfs_service)
    : prefs_service_(prefs_service), ipfs_service_(ipfs_service) {
  ipfs_base_pin_service_ =
      std::make_unique<IpfsBasePinService>(prefs_service_, ipfs_service_);
  base::SequencedTaskRunnerHandle::Get()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&IpfsLocalPinService::AddGcTask, base::Unretained(this)),
      base::Minutes(1));
}

IpfsLocalPinService::IpfsLocalPinService() {}

IpfsLocalPinService::~IpfsLocalPinService() {}

void IpfsLocalPinService::AddPins(const std::string& prefix,
                                  const std::vector<std::string>& cids,
                                  AddPinCallback callback) {
  ipfs_base_pin_service_->AddJob(std::make_unique<AddLocalPinJob>(
      prefs_service_, ipfs_service_, prefix, cids,
      base::BindOnce(&IpfsLocalPinService::OnAddJobFinished,
                     base::Unretained(this), std::move(callback))));
}

void IpfsLocalPinService::RemovePins(const std::string& prefix,
                                     RemovePinCallback callback) {
  ipfs_base_pin_service_->AddJob(std::make_unique<RemoveLocalPinJob>(
      prefs_service_, prefix,
      base::BindOnce(&IpfsLocalPinService::OnRemovePinsFinished,
                     base::Unretained(this), std::move(callback))));
}

void IpfsLocalPinService::ValidatePins(const std::string& prefix,
                                       const std::vector<std::string>& cids,
                                       ValidatePinsCallback callback) {
  ipfs_base_pin_service_->AddJob(std::make_unique<VerifyLocalPinJob>(
      prefs_service_, ipfs_service_, prefix, cids,
      base::BindOnce(&IpfsLocalPinService::OnValidateJobFinished,
                     base::Unretained(this), std::move(callback))));
}

void IpfsLocalPinService::OnRemovePinsFinished(RemovePinCallback callback,
                                               bool status) {
  std::move(callback).Run(status);
  if (status) {
    base::SequencedTaskRunnerHandle::Get()->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(&IpfsLocalPinService::AddGcTask, base::Unretained(this)),
        base::Minutes(1));
  }
  ipfs_base_pin_service_->OnJobDone(status);
}

void IpfsLocalPinService::OnAddJobFinished(AddPinCallback callback,
                                           bool status) {
  std::move(callback).Run(status);
  ipfs_base_pin_service_->OnJobDone(status);
}

void IpfsLocalPinService::OnValidateJobFinished(ValidatePinsCallback callback,
                                                absl::optional<bool> status) {
  std::move(callback).Run(status);
  ipfs_base_pin_service_->OnJobDone(status.value_or(false));
}

void IpfsLocalPinService::AddGcTask() {
  if (gc_task_posted_) {
    return;
  }
  gc_task_posted_ = true;
  ipfs_base_pin_service_->AddJob(std::make_unique<GcJob>(
      prefs_service_, ipfs_service_,
      base::BindOnce(&IpfsLocalPinService::OnGcFinishedCallback,
                     base::Unretained(this))));
}

void IpfsLocalPinService::OnGcFinishedCallback(bool status) {
  gc_task_posted_ = false;
  ipfs_base_pin_service_->OnJobDone(status);
}

}  // namespace ipfs
