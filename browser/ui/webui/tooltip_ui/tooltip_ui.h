/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_WEBUI_TOOLTIP_UI_TOOLTIP_UI_H_
#define BRAVE_BROWSER_UI_WEBUI_TOOLTIP_UI_TOOLTIP_UI_H_

#include <memory>

#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "ui/webui/mojo_bubble_web_ui_controller.h"

class Browser;
class Profile;

class TooltipUI : public ui::MojoBubbleWebUIController {
 public:
  explicit TooltipUI(content::WebUI* web_ui);
  TooltipUI(const TooltipUI&) = delete;
  TooltipUI& operator=(const TooltipUI&) = delete;
  ~TooltipUI() override;

 private:
  raw_ptr<Profile> profile_ = nullptr;
  raw_ptr<Browser> browser_ = nullptr;

  WEB_UI_CONTROLLER_TYPE_DECL();
};

#endif  // BRAVE_BROWSER_UI_WEBUI_TOOLTIP_UI_TOOLTIP_UI_H_
