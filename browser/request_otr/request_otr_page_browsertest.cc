/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/path_service.h"
#include "base/scoped_observation.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/bind.h"
#include "base/test/scoped_feature_list.h"
#include "brave/browser/brave_browser_process.h"
#include "brave/browser/brave_content_browser_client.h"
#include "brave/browser/extensions/brave_base_local_data_files_browsertest.h"
#include "brave/components/brave_shields/browser/brave_shields_util.h"
#include "brave/components/brave_shields/common/features.h"
#include "brave/components/constants/brave_paths.h"
#include "brave/components/debounce/browser/debounce_component_installer.h"
#include "brave/components/debounce/common/features.h"
#include "chrome/browser/extensions/extension_browsertest.h"
#include "chrome/browser/interstitials/security_interstitial_page_test_utils.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/testing_browser_process.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/security_interstitials/content/security_interstitial_tab_helper.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_navigation_observer.h"
#include "url/gurl.h"

namespace {
const char kTestDataDirectory[] = "request-otr-data";
}  // namespace

using request_otr::features::kBraveRequestOTR;

namespace request_otr {

class RequestOTRComponentInstallerWaiter
    : public RequestOTRComponentInstaller::Observer {
 public:
  explicit RequestOTRComponentInstallerWaiter(
      RequestOTRComponentInstaller* component_installer)
      : component_installer_(component_installer), scoped_observer_(this) {
    scoped_observer_.Observe(component_installer_);
  }
  RequestOTRComponentInstallerWaiter(
      const RequestOTRComponentInstallerWaiter&) = delete;
  RequestOTRComponentInstallerWaiter& operator=(
      const RequestOTRComponentInstallerWaiter&) = delete;
  ~RequestOTRComponentInstallerWaiter() override = default;

  void Wait() { run_loop_.Run(); }

 private:
  // RequestOTRComponentInstaller::Observer:
  void OnRulesReady(
      RequestOTRComponentInstaller* component_installer) override {
    run_loop_.QuitWhenIdle();
  }

  RequestOTRComponentInstaller* const component_installer_;
  base::RunLoop run_loop_;
  base::ScopedObservation<RequestOTRComponentInstaller,
                          RequestOTRComponentInstaller::Observer>
      scoped_observer_{this};
};

class RequestOTRBrowserTestBase : public BaseLocalDataFilesBrowserTest {
 public:
  // BaseLocalDataFilesBrowserTest overrides
  const char* test_data_directory() override { return kTestDataDirectory; }
  const char* embedded_test_server_directory() override { return ""; }
  LocalDataFilesObserver* service() override {
    return g_brave_browser_process->request_otr_component_installer();
  }

  void WaitForService() override {
    // Wait for request-otr component installer to load and parse its
    // configuration file.
    request_otr::RequestOTRComponentInstaller* component_installer =
        g_brave_browser_process->request_otr_component_installer();
    RequestOTRComponentInstallerWaiter(component_installer).Wait();
  }

  content::WebContents* web_contents() {
    return browser()->tab_strip_model()->GetActiveWebContents();
  }

  bool IsShowingInterstitial() {
    return chrome_browser_interstitials::IsShowingInterstitial(web_contents());
  }

  void NavigateTo(const GURL& url) {
    ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
    content::RenderFrameHost* frame = web_contents()->GetPrimaryMainFrame();
    ASSERT_TRUE(WaitForRenderFrameReady(frame));
  }

  void Click(const std::string& id) {
    content::RenderFrameHost* frame = web_contents()->GetPrimaryMainFrame();
    frame->ExecuteJavaScriptForTests(
        base::ASCIIToUTF16("document.getElementById('" + id + "').click();\n"),
        base::NullCallback());
  }

  void ClickAndWaitForNavigation(const std::string& id) {
    content::TestNavigationObserver observer(web_contents());
    Click(id);
    observer.WaitForNavigationFinished();
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

class RequestOTRBrowserTest : public RequestOTRBrowserTestBase {
 public:
  RequestOTRBrowserTest() { feature_list_.InitAndEnableFeature(kBraveRequestOTR); }

 private:
  base::test::ScopedFeatureList feature_list_;
};

class RequestOTRDisabledBrowserTest : public RequestOTRBrowserTestBase {
 public:
  RequestOTRDisabledTest() {
    feature_list_.InitAndDisableFeature(kBraveRequestOTR);
  }

 private:
  base::test::ScopedFeatureList feature_list_;
};

IN_PROC_BROWSER_TEST_F(RequestOTRBrowserTest, ShowInterstitial) {
  ASSERT_TRUE(InstallMockExtension());
}

}  // namespace request_otr
