// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/browser/ui/views/pulsing_block/pulsing_block_view.h"

#include <stddef.h>
#include <vector>

#include "base/check_op.h"
#include "base/rand_util.h"
#include "cc/paint/paint_shader.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkPoint.h"
#include "ui/compositor/layer.h"
// #include "ui/compositor/layer_animation_element.h"
// #include "ui/compositor/layer_animation_sequence.h"
// #include "ui/compositor/layer_animator.h"
#include "ui/gfx/canvas.h"
// #include "ui/gfx/geometry/transform.h"
#include "ui/gfx/geometry/transform_util.h"
#include "ui/views/animation/animation_builder.h"
// #include "ui/views/animation/animation_sequence_block.h"
#include "brave/components/constants/webui_url_constants.h"
#include "chrome/browser/profiles/profile.h"
#include "components/grit/brave_components_strings.h"
#include "ui/views/background.h"
#include "ui/views/bubble/bubble_border.h"
#include "ui/views/bubble/bubble_dialog_delegate_view.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"

namespace {
const base::TimeDelta kPulsingDuration = base::Milliseconds(1000);

const SkColor kBlockColor = SkColorSetRGB(52, 172, 224);
SkColor4f colors[] = {{0.66, 0.10, 0.47, 1.f}, {0.22, 0.17, 0.81, 1.f}};
SkScalar positions[] = {0.0, 0.65, 1.0};
SkPoint pts[] = {{0, 0}, {60, 60}};
auto kBraveGradient = cc::PaintShader::MakeLinearGradient(pts,
                                                          colors,
                                                          positions,
                                                          3,
                                                          SkTileMode::kClamp);

void SchedulePulsingAnimation(ui::Layer* layer) {
  DCHECK(layer);

  const gfx::Rect local_bounds(layer->bounds().size());
  views::AnimationBuilder()
      .SetPreemptionStrategy(
          ui::LayerAnimator::IMMEDIATELY_ANIMATE_TO_NEW_TARGET)
      .Repeatedly()
      .SetDuration(kPulsingDuration)
      .SetTransform(layer,
                    gfx::GetScaleTransform(local_bounds.CenterPoint(), 0.7f),
                    gfx::Tween::EASE_IN)
      .At(kPulsingDuration)
      .SetDuration(kPulsingDuration)
      .SetTransform(layer,
                    gfx::GetScaleTransform(local_bounds.CenterPoint(), 1.0f),
                    gfx::Tween::EASE_OUT);
}
}  // namespace

namespace views {
PulsingBlockView::PulsingBlockView(Browser* browser) : browser_(browser) {
  SetPaintToLayer();
  layer()->SetFillsBoundsOpaquely(false);
  SetSize({60, 60});
  SchedulePulsingAnimation(layer());
}

PulsingBlockView::~PulsingBlockView() = default;

void PulsingBlockView::AddedToWidget() {
  if (!tooltip_webui_bubble_manager_) {
    tooltip_webui_bubble_manager_ = std::make_unique<TooltipWebUIBubbleManager>(
        this, browser_->profile(), GURL(kTooltipURL), IDS_BRAVE_SHIELDS);
  }

  tooltip_webui_bubble_manager_->ShowBubble();
}

void PulsingBlockView::OnPaint(gfx::Canvas* canvas) {
  cc::PaintFlags flags;
  flags.setColor(kBlockColor);
  flags.setAntiAlias(true);
  flags.setStyle(cc::PaintFlags::kStroke_Style);
  flags.setShader(kBraveGradient);
  flags.setStrokeWidth(2.f);
  canvas->DrawCircle(GetContentsBounds().CenterPoint(), 28.f, flags);
  flags.setStrokeWidth(5.f);
  canvas->DrawCircle(GetContentsBounds().CenterPoint(), 20.f, flags);
}
}  // namespace views
