// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media/webrtc/media_capture_devices_dispatcher.h"

MediaCaptureDevicesDispatcher* MediaCaptureDevicesDispatcher::GetInstance() {
  return base::Singleton<MediaCaptureDevicesDispatcher>::get();
}

MediaCaptureDevicesDispatcher::MediaCaptureDevicesDispatcher() {}

MediaCaptureDevicesDispatcher::~MediaCaptureDevicesDispatcher() {}

bool MediaCaptureDevicesDispatcher::IsInsecureCapturingInProgress(
    int render_process_id,
    int render_frame_id) {
  return false;
}
