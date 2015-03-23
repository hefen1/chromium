// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_RENDERER_CONTEXT_MENU_CONTEXT_MENU_CONTENT_TYPE_APP_MODE_H_
#define CHROME_BROWSER_RENDERER_CONTEXT_MENU_CONTEXT_MENU_CONTENT_TYPE_APP_MODE_H_

#include "components/renderer_context_menu/context_menu_content_type.h"

class ContextMenuContentTypeAppMode : public ContextMenuContentType {
 public:
  ~ContextMenuContentTypeAppMode() override;

  // ContextMenuContentType overrides.
  bool SupportsGroup(int group) override;

 protected:
  ContextMenuContentTypeAppMode(content::WebContents* web_contents,
                                const content::ContextMenuParams& params);

 private:
  friend class ContextMenuContentTypeFactory;

  DISALLOW_COPY_AND_ASSIGN(ContextMenuContentTypeAppMode);
};

#endif  // CHROME_BROWSER_RENDERER_CONTEXT_MENU_CONTEXT_MENU_CONTENT_TYPE_APP_MODE_H_
