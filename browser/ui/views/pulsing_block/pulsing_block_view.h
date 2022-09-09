// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_UI_VIEWS_PULSING_BLOCK_PULSING_BLOCK_VIEW_H_
#define BRAVE_BROWSER_UI_VIEWS_PULSING_BLOCK_PULSING_BLOCK_VIEW_H_

#include <memory>

#include "base/compiler_specific.h"
#include "base/timer/timer.h"
#include "brave/browser/ui/views/pulsing_block/tooltip_webui_bubble_view.h"
#include "chrome/browser/ui/browser.h"
#include "ui/views/view.h"

namespace gfx {
class Size;
}

namespace views {
// PulsingBlockView shows a pulsing white circle via layer animation.
class PulsingBlockView : public views::View {
 public:
  explicit PulsingBlockView(Browser* browser);

  PulsingBlockView(const PulsingBlockView&) = delete;
  PulsingBlockView& operator=(const PulsingBlockView&) = delete;

  ~PulsingBlockView() override;

 private:
  // views::View overrides:
  void OnPaint(gfx::Canvas* canvas) override;
  void AddedToWidget() override;

  base::OneShotTimer start_delay_timer_;
  raw_ptr<Browser> browser_;
  std::unique_ptr<TooltipWebUIBubbleManager> tooltip_webui_bubble_manager_;
};

}  // namespace views

#endif  // BRAVE_BROWSER_UI_VIEWS_PULSING_BLOCK_PULSING_BLOCK_VIEW_H_
