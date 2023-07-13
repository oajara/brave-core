/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "src/components/flags_ui/flags_state_unittest.cc"

#include "base/ranges/algorithm.h"

namespace flags_ui {

TEST_F(FlagsStateTest, ShowDefaultState) {
  // Choose a non-default option for kFlags8.
  flags_state_->SetFeatureEntryEnabled(&flags_storage_,
                                       std::string(kFlags8).append("@1"), true);

  // Disable a feature that is mapped to kFlags10.
  auto feature_list = std::make_unique<base::FeatureList>();
  base::test::ScopedFeatureList scoped_feature_list;
  scoped_feature_list.InitAndDisableFeature(kTestFeature2);

  // Get flag feature entries.
  base::Value::List supported_entries;
  base::Value::List unsupported_entries;
  flags_state_->GetFlagFeatureEntries(&flags_storage_, kGeneralAccessFlagsOnly,
                                      supported_entries, unsupported_entries,
                                      base::BindRepeating(&SkipFeatureEntry));
  ASSERT_EQ(11u, supported_entries.size());

  auto get_default_option_description = [&](base::StringPiece name) {
    SCOPED_TRACE(name);
    auto entry_it =
        base::ranges::find_if(supported_entries, [&](const base::Value& entry) {
          return *entry.GetDict().FindString("internal_name") == name;
        });
    EXPECT_TRUE(entry_it != supported_entries.end());
    auto* options = entry_it->GetDict().FindList("options");
    EXPECT_TRUE(options);
    auto* description = (*options)[0].GetDict().FindString("description");
    EXPECT_TRUE(description);
    return *description;
  };

  EXPECT_EQ("Default (Enabled)", get_default_option_description(kFlags7));
  EXPECT_EQ("Default", get_default_option_description(kFlags8));
  EXPECT_EQ("Default (Enabled)", get_default_option_description(kFlags9));
  EXPECT_EQ("Default (Disabled*)", get_default_option_description(kFlags10));
  EXPECT_EQ("Default (Disabled)", get_default_option_description(kFlags12));
}

}  // namespace flags_ui
