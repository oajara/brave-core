/* Copyright 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_GOOGLE_SIGN_IN_BROWSER_GOOGLE_SIGN_IN_THROTTLE_H_
#define BRAVE_COMPONENTS_GOOGLE_SIGN_IN_BROWSER_GOOGLE_SIGN_IN_THROTTLE_H_

#include <memory>
#include <string>

#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "third_party/blink/public/common/loader/url_loader_throttle.h"

namespace blink {
class WebURLRequest;
}  // namespace blink

namespace google_sign_in {

class GoogleSignInThrottle : public blink::URLLoaderThrottle {
 public:
  explicit GoogleSignInThrottle(
      const content::WebContents::Getter& wc_getter,
      scoped_refptr<HostContentSettingsMap> settings_map);
  ~GoogleSignInThrottle() override;

  static std::unique_ptr<blink::URLLoaderThrottle> MaybeCreateThrottleFor(
      const network::ResourceRequest& request,
      const content::WebContents::Getter& wc_getter,
      HostContentSettingsMap* settings_map);

  GoogleSignInThrottle(const GoogleSignInThrottle&) = delete;
  GoogleSignInThrottle& operator=(const GoogleSignInThrottle&) = delete;

  // Implements blink::URLLoaderThrottle:
  void DetachFromCurrentSequence() override;
  void WillStartRequest(network::ResourceRequest* request,
                        bool* defer) override;

 private:
  const content::WebContents::Getter& wc_getter_;
  scoped_refptr<HostContentSettingsMap> settings_map_;
  base::WeakPtrFactory<GoogleSignInThrottle> weak_factory_{this};
};

}  // namespace google_sign_in

#endif  // BRAVE_COMPONENTS_GOOGLE_SIGN_IN_BROWSER_GOOGLE_SIGN_IN_THROTTLE_H_
