// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/aura/input_state_lookup_win.h"

#include <windows.h>
#include <winuser.h>

namespace aura {

// static
scoped_ptr<InputStateLookup> InputStateLookup::Create() {
  return make_scoped_ptr(new InputStateLookupWin);
}

InputStateLookupWin::InputStateLookupWin() {
}

InputStateLookupWin::~InputStateLookupWin() {
}

bool InputStateLookupWin::IsMouseButtonDown() const {
  return (GetKeyState(VK_LBUTTON) & 0x80) ||
    (GetKeyState(VK_RBUTTON) & 0x80) ||
    (GetKeyState(VK_MBUTTON) & 0x80) ||
    (GetKeyState(VK_XBUTTON1) & 0x80) ||
    (GetKeyState(VK_XBUTTON2) & 0x80);
}

}  // namespace aura
