// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/browser/ui/views/pulsing_block/tooltip_webui_bubble_view.h"

#include <algorithm>
#include <utility>

// #include "brave/browser/ui/views/sidebar/bubble_border_with_arrow.h"
#include "brave/browser/ui/webui/private_new_tab_page/brave_private_new_tab_ui.h"
#include "brave/components/constants/webui_url_constants.h"
#include "chrome/browser/profiles/profile.h"
#include "components/grit/brave_components_strings.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/ui_base_types.h"
#include "ui/base/window_open_disposition.h"
#include "ui/display/display.h"
#include "ui/display/screen.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/scoped_canvas.h"
#include "ui/views/accessibility/accessibility_paint_checks.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/bubble/bubble_border.h"
#include "ui/views/bubble/bubble_border_arrow_utils.h"
#include "ui/views/bubble/bubble_frame_view.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/window/caption_button_layout_constants.h"

namespace {
// interface paints a border with arrow
class BorderWithArrow : public views::BubbleBorder {
 public:
  explicit BorderWithArrow(Arrow arrow, Shadow shadow, ui::ColorId color_id)
      : views::BubbleBorder(arrow, shadow, color_id) {
    set_visible_arrow(true);
  }

  BorderWithArrow(const BorderWithArrow&) = delete;
  BorderWithArrow& operator=(const BorderWithArrow&) = delete;
  ~BorderWithArrow() override = default;
  void Paint(const views::View& view, gfx::Canvas* canvas) override {
    // views::BubbleBorder::Paint(view, canvas);
    // somehow dont trigger PaintNoShadowLegacy at all or override so it doesn't
    // draw ugly border
    PaintVisibleArrow(view, canvas);
  }
  enum BubbleArrowPart { kFill, kBorder };

 private:
  // we're copying this from upstream because we only need arrows
  void PaintVisibleArrow(const views::View& view, gfx::Canvas* canvas) {
    // we need ArrowRect* for bounds
    gfx::Point arrow_origin = GetVisibibleArrowRectForTesting().origin();

    views::View::ConvertPointFromScreen(&view, &arrow_origin);
    const gfx::Rect arrow_bounds(arrow_origin,
                                 GetVisibibleArrowRectForTesting().size());

    // Clip the canvas to a box that's big enough to hold the shadow in every
    // dimension but won't overlap the bubble itself.
    gfx::ScopedCanvas scoped(canvas);
    gfx::Rect clip_rect = arrow_bounds;
    const views::BubbleArrowSide side = GetBubbleArrowSide(arrow());
    clip_rect.Inset(
        gfx::Insets::TLBR(side == views::BubbleArrowSide::kBottom ? 0 : -2,
                          side == views::BubbleArrowSide::kRight ? 0 : -2,
                          side == views::BubbleArrowSide::kTop ? 0 : -2,
                          side == views::BubbleArrowSide::kLeft ? 0 : -2));
    canvas->ClipRect(clip_rect);

    cc::PaintFlags flags;
    flags.setStrokeCap(cc::PaintFlags::kRound_Cap);

    flags.setColor(view.GetColorProvider()->GetColor(
        ui::kColorBubbleBorderWhenShadowPresent));
    flags.setStyle(cc::PaintFlags::kStroke_Style);
    flags.setStrokeWidth(1.2);
    flags.setAntiAlias(true);
    canvas->DrawPath(
        GetVisibleArrowPath(arrow(), arrow_bounds, BubbleArrowPart::kBorder),
        flags);

    flags.setColor(color());
    flags.setStyle(cc::PaintFlags::kFill_Style);
    flags.setStrokeWidth(1.0);
    flags.setAntiAlias(true);
    canvas->DrawPath(
        GetVisibleArrowPath(arrow(), arrow_bounds, BubbleArrowPart::kFill),
        flags);
  }
  SkPath GetVisibleArrowPath(BubbleBorder::Arrow arrow,
                             const gfx::Rect& bounds,
                             BubbleArrowPart part) {
    constexpr size_t kNumPoints = 4;
    gfx::RectF bounds_f(bounds);
    SkPoint points[kNumPoints];
    switch (GetBubbleArrowSide(arrow)) {
      case views::BubbleArrowSide::kRight:
        points[0] = {bounds_f.x(), bounds_f.y()};
        points[1] = {bounds_f.right(),
                     bounds_f.y() + BubbleBorder::kVisibleArrowRadius - 1};
        points[2] = {bounds_f.right(),
                     bounds_f.y() + BubbleBorder::kVisibleArrowRadius};
        points[3] = {bounds_f.x(), bounds_f.bottom() - 1};
        break;
      case views::BubbleArrowSide::kLeft:
        points[0] = {bounds_f.right(), bounds_f.bottom() - 1};
        points[1] = {bounds_f.x(),
                     bounds_f.y() + BubbleBorder::kVisibleArrowRadius};
        points[2] = {bounds_f.x(),
                     bounds_f.y() + BubbleBorder::kVisibleArrowRadius - 1};
        points[3] = {bounds_f.right(), bounds_f.y()};
        break;
      case views::BubbleArrowSide::kTop:
        points[0] = {bounds_f.x(), bounds_f.bottom()};
        points[1] = {bounds_f.x() + BubbleBorder::kVisibleArrowRadius - 1,
                     bounds_f.y()};
        points[2] = {bounds_f.x() + BubbleBorder::kVisibleArrowRadius,
                     bounds_f.y()};
        points[3] = {bounds_f.right() - 1, bounds_f.bottom()};
        break;
      case views::BubbleArrowSide::kBottom:
        points[0] = {bounds_f.right() - 1, bounds_f.y()};
        points[1] = {bounds_f.x() + BubbleBorder::kVisibleArrowRadius,
                     bounds_f.bottom()};
        points[2] = {bounds_f.x() + BubbleBorder::kVisibleArrowRadius - 1,
                     bounds_f.bottom()};
        points[3] = {bounds_f.x(), bounds_f.y()};
        break;
    }

    return SkPath::Polygon(points, kNumPoints, part == BubbleArrowPart::kFill);
  }
};
}  // namespace

namespace views {
TooltipWebUIBubbleManager::TooltipWebUIBubbleManager(View* anchor_view,
                                                     Profile* profile,
                                                     const GURL& webui_url,
                                                     int task_manager_string_id)
    : WebUIBubbleManagerT<TooltipUI>(anchor_view,
                                     profile,
                                     webui_url,
                                     task_manager_string_id),
      anchor_view_(anchor_view),
      profile_(profile),
      webui_url_(webui_url),
      task_manager_string_id_(task_manager_string_id) {}

TooltipWebUIBubbleManager::~TooltipWebUIBubbleManager() = default;

base::WeakPtr<WebUIBubbleDialogView>
TooltipWebUIBubbleManager::CreateWebUIBubbleDialog(
    const absl::optional<gfx::Rect>& anchor) {
  BubbleContentsWrapper* contents_wrapper = nullptr;

  auto* service =
      BubbleContentsWrapperServiceFactory::GetForProfile(profile_, true);
  if (service && base::FeatureList::IsEnabled(
                     features::kWebUIBubblePerProfilePersistence)) {
    set_bubble_using_cached_web_contents(true);

    contents_wrapper = service->GetBubbleContentsWrapperFromURL(webui_url_);
    DCHECK(contents_wrapper);

    if (contents_wrapper->GetHost())
      contents_wrapper->CloseUI();
    DCHECK(!contents_wrapper->GetHost());

    if (contents_wrapper->web_contents()->IsCrashed())
      contents_wrapper->ReloadWebContents();
  } else {
    set_bubble_using_cached_web_contents(!!cached_contents_wrapper());

    if (!cached_contents_wrapper()) {
      set_cached_contents_wrapper(
          std::make_unique<BubbleContentsWrapperT<TooltipUI>>(
              webui_url_, profile_, task_manager_string_id_));
      cached_contents_wrapper()->ReloadWebContents();
    }

    contents_wrapper = cached_contents_wrapper();
  }

  // we need a custom WebUIBubbleDialog impelmentation here because it draws
  // customer border with arrow and no shadows
  auto bubble_view =
      std::make_unique<TooltipWebUIBubbleView>(anchor_view_, contents_wrapper);
  auto weak_ptr = bubble_view->GetWeakPtr();

  bubble_view->set_shadow(BubbleBorder::Shadow::NO_SHADOW_LEGACY);
  bubble_view->set_corner_radius(10);
  bubble_view->set_color(SkColorSetARGB(0xFF, 0x20, 0x4A, 0xE3));
  contents_wrapper->web_contents()->SetPageBaseBackgroundColor(
      SK_ColorTRANSPARENT);

  BubbleDialogDelegateView::CreateBubble(std::move(bubble_view));
  return weak_ptr;
}
}  // namespace views

namespace views {
TooltipWebUIBubbleView::TooltipWebUIBubbleView(
    View* anchor_view,
    BubbleContentsWrapper* contents_wrapper)
    : WebUIBubbleDialogView(anchor_view, contents_wrapper) {}

TooltipWebUIBubbleView::~TooltipWebUIBubbleView() = default;

std::unique_ptr<NonClientFrameView>
TooltipWebUIBubbleView::CreateNonClientFrameView(Widget* widget) {
  std::unique_ptr<NonClientFrameView> frame =
      BubbleDialogDelegate::CreateNonClientFrameView(widget);

  std::unique_ptr<BorderWithArrow> border =
      std::make_unique<BorderWithArrow>(arrow(), GetShadow(), color());
  border->SetColor(color());

  // we dont need to set this anymore as we're copying the logic to draw arraw
  // from upstream the issue with depending on upstream is it paints the arrow
  // with a border around the whole widget's rect and there's no way to override
  border->set_visible_arrow(true);

  // we dont need to check if custom shadows are supported
  // because this will always be flat
  if (GetParams().round_corners)
    border->SetCornerRadius(GetCornerRadius());

  // is it safe to downcast a unique ptr?
  static_cast<BubbleFrameView*>(frame.get())
      ->SetBubbleBorder(std::move(border));

  return frame;
}

void TooltipWebUIBubbleView::AddedToWidget() {
  // We always show the widget by default
  GetWidget()->Show();
}
}  // namespace views
