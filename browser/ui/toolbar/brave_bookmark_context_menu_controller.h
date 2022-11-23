/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_TOOLBAR_BRAVE_BOOKMARK_CONTEXT_MENU_CONTROLLER_H_
#define BRAVE_BROWSER_UI_TOOLBAR_BRAVE_BOOKMARK_CONTEXT_MENU_CONTROLLER_H_

#include <memory>
#include <vector>

#include "base/callback_forward.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "brave/browser/ui/toolbar/bookmark_bar_sub_menu_model.h"
#include "chrome/browser/ui/bookmarks/bookmark_context_menu_controller.h"
#include "chrome/browser/ui/bookmarks/bookmark_stats.h"
#include "components/bookmarks/browser/base_bookmark_model_observer.h"
#include "ui/base/models/simple_menu_model.h"
#include "ui/gfx/native_widget_types.h"

class BraveBookmarkContextMenuController
    : public BookmarkContextMenuController {
 public:
  BraveBookmarkContextMenuController(
      gfx::NativeWindow parent_window,
      BookmarkContextMenuControllerDelegate* delegate,
      Browser* browser,
      Profile* profile,
      BookmarkLaunchLocation opened_from,
      const bookmarks::BookmarkNode* parent,
      const std::vector<const bookmarks::BookmarkNode*>& selection);

  BraveBookmarkContextMenuController(
      const BraveBookmarkContextMenuController&) = delete;
  BraveBookmarkContextMenuController& operator=(
      const BraveBookmarkContextMenuController&) = delete;

  ~BraveBookmarkContextMenuController() override;

  // ui::SimpleMenuModel::Delegate implementation:
  bool IsCommandIdChecked(int command_id) const override;
  bool IsCommandIdEnabled(int command_id) const override;
  bool IsCommandIdVisible(int command_id) const override;
  void ExecuteCommand(int command_id, int event_flags) override;
  bool IsItemForCommandIdDynamic(int command_id) const override;
  std::u16string GetLabelForCommandId(int command_id) const override;

  void AddBraveBookmarksSubmenu(Profile* profile);

  BookmarkBarSubMenuModel* GetBookmarkSubmenuModel();

  std::unique_ptr<BookmarkBarSubMenuModel> brave_bookmarks_submenu_model_;
};

#endif  // BRAVE_BROWSER_UI_TOOLBAR_BRAVE_BOOKMARK_CONTEXT_MENU_CONTROLLER_H_
