/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/request_otr/browser/request_otr_tab_storage.h"

// If user has requested going off-the-record in this tab, don't allow access to
// the history service. All callers null-check this because the upstream
// GetHistoryService function already returns NULL if the entire profile is
// off-the-record.
#define BRAVE_GET_HISTORY_SERVICE                                  \
  if (request_otr::RequestOTRTabStorage* request_otr_tab_storage = \
          request_otr::RequestOTRTabStorage::FromWebContents(      \
              web_contents())) {                                   \
    if (request_otr_tab_storage->RequestedOTR())                   \
      return NULL;                                                 \
  }

#include "src/chrome/browser/history/history_tab_helper.cc"

#undef BRAVE_GET_HISTORY_SERVICE
