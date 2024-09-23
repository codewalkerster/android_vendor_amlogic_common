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
#define LOG_TAG "ErrorMonitorHal"
#include <inttypes.h>
#include <log/log.h>
#include <string>
#include <utils/String8.h>
#include "ErrorMonitorHal.h"

namespace vendor {
namespace amlogic {
namespace hardware {
namespace errormonitor {
namespace V1_0 {
namespace implementation {
using ::android::Mutex;

ErrorMonitorHal::ErrorMonitorHal(ErrorMonitorService* service)
        : mErrorMonitorService(service),
          mDeathRecipient(new DeathRecipient(this)) {
    ALOGI("ErrorMonitorHal construct");
    service->setListener(this);
}

ErrorMonitorHal::~ErrorMonitorHal() { ALOGI("~ErrorMonitorHal "); }

Return<void> ErrorMonitorHal::setCallback(const sp<IErrorMonitorCallback>& callback) {
    if (callback != nullptr) {
        ALOGI("setCallback ");
        mCallBack = callback;
        if (mErrorMsg.size() > 0)
            mCallBack->onError(mErrorMsg);
    }
    return Void();
}

Return<Result> ErrorMonitorHal::startErrorMonitor(const hidl_string& monitorConfig) {
    Mutex::Autolock autoLock(mLock);
    std::string config = monitorConfig;
    return mErrorMonitorService->startErrorMonitor(config) ? Result::OK : Result::FAIL;
}

Return<void> ErrorMonitorHal::stopErrorMonitor() {
    Mutex::Autolock autoLock(mLock);
    if (mErrorMonitorService) {
        mErrorMonitorService->stopErrorMonitor();
    }
    mCallBack = nullptr;
    return Void();
}

Return<void> ErrorMonitorHal::getMonitorConfig(getMonitorConfig_cb _hidl_cb) {
    Mutex::Autolock autoLock(mLock);
    std::string config;
    if (mErrorMonitorService && mErrorMonitorService->getMonitorConfig(&config)) {
        _hidl_cb(Result::OK, config);
        ALOGI("[%s %d]  config : %s", __FUNCTION__, __LINE__, config.c_str());
    } else {
        _hidl_cb(Result::FAIL, config);
    }
    return Void();
}

Return<Result> ErrorMonitorHal::updateMonitorConfig(const hidl_string& monitorConfig) {
    Mutex::Autolock autoLock(mLock);
    std::string config = monitorConfig;
    return mErrorMonitorService ? mErrorMonitorService->updateMonitorConfig(config) ? Result::OK : Result::FAIL
                                : Result::FAIL;
}

Return<Result> ErrorMonitorHal::setLogLevel(const hidl_string& levelConfig) {
    Mutex::Autolock autoLock(mLock);
    std::string config = levelConfig;
    return mErrorMonitorService ? mErrorMonitorService->setLogLevel(config) ? Result::OK : Result::FAIL : Result::FAIL;
}

Return<void> ErrorMonitorHal::getLogLevel(getLogLevel_cb _hidl_cb) {
    Mutex::Autolock autoLock(mLock);
    std::string config;
    if (mErrorMonitorService && mErrorMonitorService->getLogLevel(&config)) {
        _hidl_cb(Result::OK, config);
        ALOGI("[%s %d]  config : %s", __FUNCTION__, __LINE__, config.c_str());
    } else {
        _hidl_cb(Result::FAIL, config);
    }
    return Void();
}

Return<void> ErrorMonitorHal::notifyErrorInfo(int32_t subModule, int32_t level, int32_t logType, int32_t errorType,
                                              const hidl_string& msg) {
    Mutex::Autolock autoLock(mLock);
    if (mErrorMonitorService)
        mErrorMonitorService->notifyErrorInfo(subModule, level, logType, errorType, msg.c_str());
    return Void();
}

void ErrorMonitorHal::onReport(int32_t mainModule, int32_t subModule, int32_t level, int32_t logType, int64_t events,
                               const char* msg) {
    std::string msgString = msg;
    if (mCallBack) {
        ALOGI("[%s %d]  msgString : %s", __FUNCTION__, __LINE__, msgString.c_str());
        Return<void> ret = mCallBack->onReport(mainModule, subModule, level, logType, events, msgString);
    }
}

void ErrorMonitorHal::onError(const std::string& msg) {
    ALOGI("onError msg=%s", msg.c_str());
    mErrorMsg.clear();
    mErrorMsg = msg;
    if (mCallBack) {
        mCallBack->onError(mErrorMsg);
    }
}

void ErrorMonitorHal::handleServiceDeath(uint32_t cookie) {
    Mutex::Autolock autoLock(mLock);
    ALOGE("ErrorMonitorService handleServiceDeath cookie:%d", (int)cookie);
    mCallBack = nullptr;
}
ErrorMonitorHal::DeathRecipient::DeathRecipient(sp<ErrorMonitorHal> sch)
        : mErrorMonitorlHal(std::move(sch)) {}

void ErrorMonitorHal::DeathRecipient::serviceDied(uint64_t cookie,
                                                  const ::android::wp<::android::hidl::base::V1_0::IBase>&) {
    ALOGE("ErrorMonitorService daemon client died cookie:%d", (int)cookie);
    uint32_t type = static_cast<uint32_t>(cookie);
    mErrorMonitorlHal->handleServiceDeath(type);
}

} // namespace implementation
} // namespace V1_0
} // namespace errormonitor
} // namespace hardware
} // namespace amlogic
} // namespace vendor
