/*
 **
 ** Copyright 2019 The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#ifndef AMLOGIC_EERRORMONITOR_RESMANWRAPPER_H
#define AMLOGIC_EERRORMONITOR_RESMANWRAPPER_H

#include <stdint.h>
#include <utils/RefBase.h>
#include <utils/Singleton.h>
#include "resourcemanage.h"

namespace android {

typedef bool (*DResman_support)(void);
typedef int (*DResman_init)(const char* appname, int type);
typedef int (*DResman_close)(int handle);
typedef int (*DResman_register)(int fd, void (*handler)(void*), void* opaque);
typedef int (*DResman_add_debug_callback)(int fd, void (*debug)(void*, const char*, int), void* opaque);
typedef int (*DResman_add_error_info_callback)(int fd, void (*notify_error_info)(void*, const char*, int),
                                               void* opaque);
typedef int (*DResman_set_debug_info)(int fd, const char* info);

class ResManage : public Singleton<ResManage> {

public:
    ResManage();
    ~ResManage();
    bool libHandlValid() { return !!resManLibHandle; }

    DResman_init resman_init;
    DResman_support resman_support;
    DResman_close resman_close;
    DResman_register resman_register;
    DResman_add_debug_callback resman_add_debug_callback;
    DResman_add_error_info_callback resman_add_error_info_callback;
    DResman_set_debug_info resman_set_debug_info;

private:
    void* resManLibHandle;
};

class ResManWrapper {

public:
    ResManWrapper(const char* appname, void* user);
    ~ResManWrapper();

    bool valid() { return mfd >= 0; }
    bool supportRM() { return mSupportRM; }
    bool addDebugCallback(void (*debug)(void*, const char*, int));
    bool addErrorInfoCallback(void (*notify_error_info)(void*, const char*, int));
    bool setDebugInfo(const char* info);

private:
    int mfd;
    ResManage* resManHandle;
    bool mSupportRM;
    void* mUser;
};

} // namespace android

#endif /* AMLOGIC_EERRORMONITOR_RESMANWRAPPER_H */
