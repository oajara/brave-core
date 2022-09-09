// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_UI_VIEWS_PULSING_BLOCK_TOOLTIP_WEBUI_BUBBLE_VIEW_H_
#define BRAVE_BROWSER_UI_VIEWS_PULSING_BLOCK_TOOLTIP_WEBUI_BUBBLE_VIEW_H_

#include <memory>

#include "base/component_export.h"
#include "brave/browser/ui/webui/tooltip_ui/tooltip_ui.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/views/bubble/bubble_contents_wrapper.h"
#include "chrome/browser/ui/views/bubble/webui_bubble_dialog_view.h"
#include "chrome/browser/ui/views/bubble/webui_bubble_manager.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/bubble/bubble_border.h"
#include "ui/views/bubble/bubble_dialog_delegate_view.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/widget/widget_delegate.h"

namespace views {
// custom impl of WebUIBubbleManager that creates a dialog with the new border
// manages TooltipWebUIBubbleView and is owned by PulsingBlockView
class TooltipWebUIBubbleManager : public WebUIBubbleManagerT<TooltipUI> {
 public:
  explicit TooltipWebUIBubbleManager(View* anchor_view,
                                     Profile* profile,
                                     const GURL& webui_url,
                                     int task_manager_string_id);
  TooltipWebUIBubbleManager(const TooltipWebUIBubbleManager&) = delete;
  TooltipWebUIBubbleManager& operator=(const TooltipWebUIBubbleManager&) =
      delete;
  ~TooltipWebUIBubbleManager() override;

  base::WeakPtr<WebUIBubbleDialogView> CreateWebUIBubbleDialog(
      const absl::optional<gfx::Rect>& anchor) override;

 private:
  const raw_ptr<View> anchor_view_;
  const raw_ptr<Profile> profile_;
  const GURL webui_url_;
  const int task_manager_string_id_;
};

// This view anchors the pulsing block view and will always be placed on top of
// WebView
class TooltipWebUIBubbleView : public WebUIBubbleDialogView {
 public:
  explicit TooltipWebUIBubbleView(View* anchor_view,
                                  BubbleContentsWrapper* contents_wrapper);
  TooltipWebUIBubbleView(const TooltipWebUIBubbleView&) = delete;
  TooltipWebUIBubbleView& operator=(const TooltipWebUIBubbleView&) = delete;
  ~TooltipWebUIBubbleView() override;

  // views::BubbleDialogDelegate
  std::unique_ptr<NonClientFrameView> CreateNonClientFrameView(
      Widget* widget) override;
  void AddedToWidget() override;
};
}  // namespace views

#endif  // BRAVE_BROWSER_UI_VIEWS_PULSING_BLOCK_TOOLTIP_WEBUI_BUBBLE_VIEW_H_
