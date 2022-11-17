/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/ads_observer_manager.h"

#include <utility>

#include "base/check_op.h"

namespace ads {

namespace {
AdsObserverManager* g_observer_manager_instance = nullptr;
}  // namespace

AdsObserverManager::AdsObserverManager() {
  DCHECK(!g_observer_manager_instance);
  g_observer_manager_instance = this;
}

AdsObserverManager::~AdsObserverManager() {
  DCHECK_EQ(this, g_observer_manager_instance);
  g_observer_manager_instance = nullptr;
}

// static
AdsObserverManager* AdsObserverManager::GetInstance() {
  DCHECK(g_observer_manager_instance);
  return g_observer_manager_instance;
}

// static
bool AdsObserverManager::HasInstance() {
  return !!g_observer_manager_instance;
}

void AdsObserverManager::AddObserver(
    mojo::PendingRemote<bat_ads::mojom::BatAdsObserver> observer) {
  observers_.Add(std::move(observer));
}

void AdsObserverManager::NotifyStatementOfAccountsDidChange() const {
  for (const auto& observer : observers_) {
    observer->OnStatementOfAccountsDidChange();
  }
}

}  // namespace ads
