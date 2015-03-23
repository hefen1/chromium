// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// A RendererPreconnect instance is maintained for each RenderThread.
// URL strings are typically added to the embedded queue during rendering.
// The first addition to the queue (transitioning from empty to having
// some names) causes a processing task to be added to the Renderer Thread.
// The processing task gathers all buffered URLs, and send them via IPC
// to the browser.
// This class counts repeated requests for the same URL and requests that
// number of connections be preconnected. If multiple requests are sent
// separately the net stack will just keep re-using the first pending
// connection so we allow for the time between the parsing of the tags and
// when the task is scheduled to accumulate multiple requests.

#ifndef COMPONENTS_NETWORK_HINTS_RENDERER_RENDERER_PRECONNECT_H_
#define COMPONENTS_NETWORK_HINTS_RENDERER_RENDERER_PRECONNECT_H_

#include <map>

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "url/gurl.h"

namespace network_hints {

// An internal interface to the network_hints component for efficiently sending
// DNS prefetch requests to the net stack.
class RendererPreconnect {
 public:
  RendererPreconnect();
  ~RendererPreconnect();

  // Push a name into the queue to be preconnected.
  void Preconnect(const GURL &url);

  // SubmitPreconnect processes the buffered URLs, and submits them for
  // preconnecting.  Multiple requests for the same URL will increase the number
  // of connections requested.
  void SubmitPreconnect();

 private:
  // url_request_count_map_ contains (for each URL) a count for the number of
  // times it was requested.
  typedef std::map<GURL, int> UrlRequestCountMap;
  UrlRequestCountMap url_request_count_map_;

  base::WeakPtrFactory<RendererPreconnect> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(RendererPreconnect);
};  // class RendererPreconnect

}  // namespace network_hints

#endif  // COMPONENTS_NETWORK_HINTS_RENDERER_RENDERER_PRECONNECT_H_
