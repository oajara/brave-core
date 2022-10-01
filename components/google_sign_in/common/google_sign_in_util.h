/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_GOOGLE_SIGN_IN_COMMON_GOOGLE_SIGN_IN_UTIL_H_
#define BRAVE_COMPONENTS_GOOGLE_SIGN_IN_COMMON_GOOGLE_SIGN_IN_UTIL_H_

#include <string>
#include <vector>

#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/content_settings/core/common/content_settings_pattern.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/permission_controller_delegate.h"
#include "content/public/browser/web_contents.h"
#include "third_party/blink/public/common/permissions/permission_utils.h"

#include "url/gurl.h"

namespace google_sign_in {

bool ShouldCheckGoogleSignInPermission(const GURL& request_url,
                                       const GURL& request_initiator_url);
blink::mojom::PermissionStatus GetCurrentGoogleSignInPermissionStatus(
    content::PermissionControllerDelegate* permission_controller,
    content::WebContents* contents,
    const GURL& request_initiator_url);
bool IsGoogleSignInEnabled(PrefService* prefs);
void Set3pCookieException(HostContentSettingsMap* content_settings,
                          const ContentSettingsPattern& embedding_pattern,
                          const ContentSetting& content_setting);
void HandleBraveGoogleSignInPermissionStatus(
    const GURL& request_initiator_url,
    scoped_refptr<HostContentSettingsMap> content_settings,
    const std::vector<blink::mojom::PermissionStatus>& permission_statuses);

}  // namespace google_sign_in

#endif  // BRAVE_COMPONENTS_GOOGLE_SIGN_IN_COMMON_GOOGLE_SIGN_IN_UTIL_H_
