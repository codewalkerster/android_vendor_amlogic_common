/*
 * Copyright (C) 2006 The Android Open Source Project
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
 */
#define LOG_NDEBUG 0
#define LOG_TAG "MonitorResManWrapper"
#include <dlfcn.h>
#include <utils/Log.h>
#include "ResManWrapper.h"

#define RESPRIO10 (10)

namespace android {

ANDROID_SINGLETON_STATIC_INSTANCE(ResManage);

ResManage::ResManage()
        : resManLibHandle(nullptr) {
    resManLibHandle = dlopen("libmediahal_resman.so", RTLD_NOW);
    if (resManLibHandle == NULL)
        resManLibHandle = dlopen("libmediahal_resman.system.so", RTLD_NOW);
    if (resManLibHandle) {
        resman_support = (DResman_support)dlsym(resManLibHandle, "resman_support");
        resman_init = (DResman_init)dlsym(resManLibHandle, "resman_init");
        resman_close = (DResman_close)dlsym(resManLibHandle, "resman_close");
        resman_register = (DResman_register)dlsym(resManLibHandle, "resman_register");
        resman_add_debug_callback = (DResman_add_debug_callback)dlsym(resManLibHandle, "resman_add_debug_callback");
        resman_add_error_info_callback =
            (DResman_add_error_info_callback)dlsym(resManLibHandle, "resman_add_error_info_callback");
        resman_set_debug_info = (DResman_set_debug_info)dlsym(resManLibHandle, "resman_set_debug_info");
        if (!resman_init || !resman_register || !resman_add_debug_callback || !resman_add_error_info_callback ||
            !resman_set_debug_info) {
            ALOGE("dlsym error:%s", dlerror());
            dlclose(resManLibHandle);
            resManLibHandle = nullptr;
        }
    } else
        ALOGW("dlopen libmediahal_resman.so error:%s", dlerror());
}

ResManage::~ResManage() {
    if (resManLibHandle)
        dlclose(resManLibHandle);
}

ResManWrapper::ResManWrapper(const char* appname, void* user)
        : mUser(user) {
    resManHandle = &ResManage::getInstance();
    if (resManHandle && resManHandle->libHandlValid() && resManHandle->resman_support) {
        mSupportRM = resManHandle->resman_support();
        if (resManHandle->resman_init)
            mfd = resManHandle->resman_init(appname, RESMAN_APP_OTHER);
        ALOGD("%s %s init, fd = %d", __FUNCTION__, appname, mfd);
    } else {
        mSupportRM = false;
        mfd = -1;
        ALOGE("%s init fail!", __FUNCTION__);
    }
}

ResManWrapper::~ResManWrapper() {
    if (valid()) {
        ALOGD("%s fd = %d", __FUNCTION__, mfd);
        resManHandle->resman_close(mfd);
        mfd = -1;
    }
}

bool ResManWrapper::addDebugCallback(void (*debug)(void*, const char*, int)) {
    return (valid() && resManHandle->resman_add_debug_callback && debug)
               ? (resManHandle->resman_add_debug_callback(mfd, debug, mUser) == 0) ? true : false
               : false;
}

bool ResManWrapper::addErrorInfoCallback(void (*notify_error_info)(void*, const char*, int)) {
    return (valid() && resManHandle->resman_add_error_info_callback && notify_error_info)
               ? (resManHandle->resman_add_error_info_callback(mfd, notify_error_info, mUser) == 0) ? true : false
               : false;
}

bool ResManWrapper::setDebugInfo(const char* info) {
    ALOGI("ResManWrapper setDebugInfo info = %s", info);
    return (valid() && info) ? resManHandle->resman_set_debug_info(mfd, info) : false;
}

} // namespace android
