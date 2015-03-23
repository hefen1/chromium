// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_TEST_TEST_WEB_CONTENTS_H_
#define CONTENT_TEST_TEST_WEB_CONTENTS_H_

#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/test/web_contents_tester.h"
#include "content/test/test_render_frame_host.h"
#include "content/test/test_render_view_host.h"
#include "ui/base/page_transition_types.h"

class SiteInstanceImpl;

namespace content {

class RenderViewHost;
class TestRenderViewHost;
class WebContentsTester;

// Subclass WebContentsImpl to ensure it creates TestRenderViewHosts
// and does not do anything involving views.
class TestWebContents : public WebContentsImpl, public WebContentsTester {
 public:
  ~TestWebContents() override;

  static TestWebContents* Create(BrowserContext* browser_context,
                                 SiteInstance* instance);

  // WebContentsImpl overrides (returning the same values, but in Test* types)
  TestRenderFrameHost* GetMainFrame() override;
  TestRenderViewHost* GetRenderViewHost() const override;

  // WebContentsTester implementation.
  void CommitPendingNavigation() override;
  TestRenderFrameHost* GetPendingMainFrame() const override;
  void NavigateAndCommit(const GURL& url) override;
  void TestSetIsLoading(bool value) override;
  void ProceedWithCrossSiteNavigation() override;
  void TestDidNavigate(RenderFrameHost* render_frame_host,
                       int page_id,
                       const GURL& url,
                       ui::PageTransition transition) override;
  void TestDidNavigateWithReferrer(RenderFrameHost* render_frame_host,
                                   int page_id,
                                   const GURL& url,
                                   const Referrer& referrer,
                                   ui::PageTransition transition) override;

  // State accessor.
  bool cross_navigation_pending() {
    return GetRenderManager()->cross_navigation_pending_;
  }

  // Prevent interaction with views.
  bool CreateRenderViewForRenderManager(RenderViewHost* render_view_host,
                                        int opener_route_id,
                                        int proxy_routing_id,
                                        bool for_main_frame) override;
  void UpdateRenderViewSizeForRenderManager() override {}

  // Returns a clone of this TestWebContents. The returned object is also a
  // TestWebContents. The caller owns the returned object.
  WebContents* Clone() override;

  // Allow mocking of the RenderViewHostDelegateView.
  RenderViewHostDelegateView* GetDelegateView() override;
  void set_delegate_view(RenderViewHostDelegateView* view) {
    delegate_view_override_ = view;
  }

  // Allows us to simulate this tab having an opener.
  void SetOpener(TestWebContents* opener);

  // Allows us to simulate that a contents was created via CreateNewWindow.
  void AddPendingContents(TestWebContents* contents);

  // Establish expected arguments for |SetHistoryOffsetAndLength()|. When
  // |SetHistoryOffsetAndLength()| is called, the arguments are compared
  // with the expected arguments specified here.
  void ExpectSetHistoryOffsetAndLength(int history_offset,
                                       int history_length);

  // Compares the arguments passed in with the expected arguments passed in
  // to |ExpectSetHistoryOffsetAndLength()|.
  void SetHistoryOffsetAndLength(int history_offset,
                                 int history_length) override;

  void TestDidFinishLoad(const GURL& url);
  void TestDidFailLoadWithError(const GURL& url,
                                int error_code,
                                const base::string16& error_description);

 protected:
  // The deprecated WebContentsTester still needs to subclass this.
  explicit TestWebContents(BrowserContext* browser_context);

 private:
  // WebContentsImpl overrides
  void CreateNewWindow(
      int render_process_id,
      int route_id,
      int main_frame_route_id,
      const ViewHostMsg_CreateWindow_Params& params,
      SessionStorageNamespace* session_storage_namespace) override;
  void CreateNewWidget(int render_process_id,
                       int route_id,
                       blink::WebPopupType popup_type) override;
  void CreateNewFullscreenWidget(int render_process_id, int route_id) override;
  void ShowCreatedWindow(int route_id,
                         WindowOpenDisposition disposition,
                         const gfx::Rect& initial_rect,
                         bool user_gesture) override;
  void ShowCreatedWidget(int route_id, const gfx::Rect& initial_rect) override;
  void ShowCreatedFullscreenWidget(int route_id) override;

  RenderViewHostDelegateView* delegate_view_override_;

  // Expectations for arguments of |SetHistoryOffsetAndLength()|.
  bool expect_set_history_offset_and_length_;
  int expect_set_history_offset_and_length_history_offset_;
  int expect_set_history_offset_and_length_history_length_;
};

}  // namespace content

#endif  // CONTENT_TEST_TEST_WEB_CONTENTS_H_
