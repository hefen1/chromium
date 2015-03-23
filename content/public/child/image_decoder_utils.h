// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_UTILITY_IMAGE_DECODER_UTILS_H_
#define CONTENT_PUBLIC_UTILITY_IMAGE_DECODER_UTILS_H_

#include "base/basictypes.h"
#include "content/common/content_export.h"
#include "ui/gfx/geometry/size.h"

class SkBitmap;

namespace content {

// Helper function to decode the image using the data passed in.
// On success returns the decoded image.
// On failure returns an empty bitmap.
CONTENT_EXPORT SkBitmap DecodeImage(const unsigned char* data,
                                    const gfx::Size& desired_image_size,
                                    size_t size);
}  // namespace content

#endif  // CONTENT_PUBLIC_UTILITY_IMAGE_DECODER_UTILS_H_
