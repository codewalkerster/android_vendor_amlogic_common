/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *  @author   Tellen Yu
 *  @version  2.0
 *  @date     2014/10/23
 *  @par function description:
 *  - 1 set display mode
 */
#define LOG_TAG "SystemControl"
#define LOG_NDEBUG 0
#include "Rect.h"
#include "common.h"
#include "SysWrite.h"
#include "DisplayMode.h"
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#define VIDEO_AXIS "/sys/class/video/device_resolution"
using namespace droidlogic;
Rect::Rect():left(0),top(0),right(0),bottom(0) {
}
Rect::Rect(unsigned int l,unsigned int t, unsigned int r, unsigned int b):left(l),top(t),right(r),bottom(b){}
bool Rect::inside(Rect& rect) {
    ALOGE("inScreen inside %d %d %d %d",rect.left,rect.top,rect.right, rect.bottom);
    return left > rect.left && top > rect.top && right < rect.right && bottom < rect.bottom;
}
bool Rect::inscreen() {
    unsigned int w = 0;
    unsigned int h = 0;
    char axis[MAX_STR_LEN] = {0};
    SysWrite* pSysWrite = new SysWrite();
    pSysWrite->readSysfsOriginal(VIDEO_AXIS, axis);
    ALOGD("axis is %s",axis);
    std::string axisStr = axis;
    std::size_t pos = axisStr.find("x");
    if (pos > 0) {
        std::string WStr = axisStr.substr(0,pos);
        std::string HStr = axisStr.substr(pos+1);
        w = atoi(WStr.c_str());
        h = atoi(HStr.c_str());
    }else {
        ALOGE("[init]mWindowAxis.stored fail.\n");
        //return false;
        w = 3840;
        h = 2160;
    }
    ALOGE("inScreen  %d %d [ %d %d %d %d]",w,h,left,top,right,bottom);
    delete pSysWrite;
    return (right-left) > w*2/3 && (bottom - top) > h*2/3;
}
