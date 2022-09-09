// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/browser/ui/webui/tooltip_ui/tooltip_ui.h"

#include <utility>

#include "base/bind.h"
#include "brave/browser/ui/brave_browser_window.h"
#include "brave/components/constants/webui_url_constants.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/webui/favicon_source.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "components/favicon_base/favicon_url_parser.h"
#include "components/grit/brave_components_resources.h"
#include "content/public/browser/web_ui.h"

// Cache active Browser instance's TabStripModel to give
// to ShieldsPanelDataHandler when this is created because
// CreatePanelHandler() is run in async.
TooltipUI::TooltipUI(content::WebUI* web_ui)
    : ui::MojoBubbleWebUIController(web_ui, true),
      profile_(Profile::FromWebUI(web_ui)) {
  browser_ = chrome::FindLastActiveWithProfile(profile_);

  content::WebUIDataSource* source =
      content::WebUIDataSource::Create(kTooltipHost);

  const webui::ResourcePath kStubGenerated[] = {};

  content::URLDataSource::Add(
      profile_, std::make_unique<FaviconSource>(
                    profile_, chrome::FaviconUrlFormat::kFavicon2));

  webui::SetupWebUIDataSource(source, base::make_span(kStubGenerated, 0),
                              IDR_TOOLTIP_UI_HTML);

  content::WebUIDataSource::Add(web_ui->GetWebContents()->GetBrowserContext(),
                                source);
}

TooltipUI::~TooltipUI() = default;

WEB_UI_CONTROLLER_TYPE_IMPL(TooltipUI)
