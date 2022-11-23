/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_VIEWS_BOOKMARKS_BOOKMARK_CONTEXT_MENU_H_
#define BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_VIEWS_BOOKMARKS_BOOKMARK_CONTEXT_MENU_H_

#include "brave/browser/ui/toolbar/brave_bookmark_context_menu_controller.h"

#define BookmarkContextMenuController         \
  BraveBookmarkContextMenuController > dummy; \
  friend class BraveBookmarkContextMenu;      \
  std::unique_ptr < BraveBookmarkContextMenuController
#include "src/chrome/browser/ui/views/bookmarks/bookmark_context_menu.h"
#undef BookmarkContextMenuController

#endif  // BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_VIEWS_BOOKMARKS_BOOKMARK_CONTEXT_MENU_H_
