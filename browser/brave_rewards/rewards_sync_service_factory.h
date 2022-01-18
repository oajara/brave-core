/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_BRAVE_REWARDS_REWARDS_SYNC_SERVICE_FACTORY_H_
#define BRAVE_BROWSER_BRAVE_REWARDS_REWARDS_SYNC_SERVICE_FACTORY_H_

#include <memory>
#include <vector>

#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class Profile;

namespace base {
template <typename T>
struct DefaultSingletonTraits;
}  // namespace base

namespace syncer {
class SyncServiceImpl;
class SyncService;
}  // namespace syncer

class RewardsSyncServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  // Returns the SyncService for the given profile.
  static syncer::SyncService* GetForProfile(Profile* profile);
  // Returns the SyncServiceImpl for the given profile. DO NOT USE unless
  // absolutely necessary! Prefer GetForProfile instead.
  static syncer::SyncServiceImpl* GetAsSyncServiceImplForProfile(
      Profile* profile);

  RewardsSyncServiceFactory(const RewardsSyncServiceFactory&) = delete;
  RewardsSyncServiceFactory& operator=(const RewardsSyncServiceFactory&) =
      delete;

  // Returns whether a SyncService has already been created for the profile.
  // Note that GetForProfile will create the service if it doesn't exist yet.
  static bool HasSyncService(Profile* profile);

  // Checks whether sync is configurable by the user. Returns false if sync is
  // disallowed by the command line or controlled by configuration management.
  // |profile| must not be nullptr.
  static bool IsSyncAllowed(Profile* profile);

  static RewardsSyncServiceFactory* GetInstance();

  // Iterates over all profiles that have been loaded so far and extract their
  // SyncService if present. Returned pointers are guaranteed to be not null.
  static std::vector<const syncer::SyncService*> GetAllSyncServices();

  // Returns the default factory, useful in tests where it's null by default.
  static TestingFactory GetDefaultFactory();

 private:
  friend struct base::DefaultSingletonTraits<RewardsSyncServiceFactory>;

  RewardsSyncServiceFactory();
  ~RewardsSyncServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
  bool ServiceIsNULLWhileTesting() const override;
};

#endif  // BRAVE_BROWSER_BRAVE_REWARDS_REWARDS_SYNC_SERVICE_FACTORY_H_
