// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_VR_UI_SUPPORT_H_
#define CHROME_BROWSER_VR_UI_SUPPORT_H_

#include "chrome/browser/vr/vr_base_export.h"
#include "components/url_formatter/url_formatter.h"
#include "third_party/icu/source/common/unicode/uchar.h"
#include "third_party/icu/source/common/unicode/uscript.h"
#include "third_party/icu/source/common/unicode/utypes.h"
#include "ui/gfx/vector_icon_types.h"
#include "url/gurl.h"

namespace vr {

// Methods in this module are wrappers around methods that the UI should not
// utilitize directly.  If UI code is split to a feature module, then libraries
// such as base:: must either export their methods for use by the module, or VR
// must supply wrappers for use by the module.  For libraries that require only
// one or two methods, it's more efficient to make wrappers.

VR_BASE_EXPORT UScriptCode UScriptGetScript(UChar32 codepoint, UErrorCode* err);

VR_BASE_EXPORT base::string16 FormatUrlForVr(const GURL& gurl,
                                             url::Parsed* new_parsed);

enum VrIconId {
  kVrNoneIcon,
  kVrReloadIcon,
  kVrVideocamIcon,
  kVrBackArrowIcon,
  kVrInfoOutlineIcon,
  kVrScreenShareIcon,
  kVrCloseRoundedIcon,
  kVrForwardArrowIcon,
  kVrBluetoothConnectedIcon,
  kVrMicIcon,
  kVrMyLocationIcon,
  kVrRepositionIcon,
  kVrMoreVertIcon,
  kVrSadTabIcon,
  kVrRemoveCircleOutlineIcon,
  kVrDaydreamControllerAppButtonIcon,
  kVrDaydreamControllerHomeButtonIcon,
  kVrOpenInBrowserIcon,
  kVrUsbIcon,
  kVrMidiIcon,
};

VR_BASE_EXPORT const gfx::VectorIcon& GetVrIcon(VrIconId icon);

}  // namespace vr

#endif  // CHROME_BROWSER_VR_UI_SUPPORT_H_
