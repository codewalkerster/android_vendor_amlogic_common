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
#define LOG_TAG "ErrorMonitorService"
#include <binder/IBinder.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>
#include <hidl/HidlLazyUtils.h>
#include <utils/Log.h>
#include <utils/String16.h>
#include "ErrorMonitorHal.h"
#include "ErrorMonitorService.h"

using android::hardware::LazyServiceRegistrar;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::amlogic::hardware::errormonitor::V1_0::implementation::ErrorMonitorHal;
namespace android {

ErrorMonitorService::ErrorMonitorService() {
    ALOGI("ErrorMonitorService construct");
    mLogSetter = std::make_unique<LogLevelSetterImpl1>();
    mMonitor = std::make_unique<Monitor>();
    mMonitor->init();
}

ErrorMonitorService::~ErrorMonitorService() {
    ALOGI("~ErrorMonitorService");
}
ErrorMonitorService* ErrorMonitorService::getInstance() {
    ErrorMonitorService* service = new ErrorMonitorService();
    return service;
}

void ErrorMonitorService::setListener(const sp<ErrorMonitorNotify>& listener) {
    mNotifyListener = listener;
}

bool ErrorMonitorService::startErrorMonitor(std::string& monitorConfig) {
    ALOGI("[%s %d]  monitorConfig : %s", __FUNCTION__, __LINE__, monitorConfig.c_str());
    Mutex::Autolock autoLock(mLock);
    if (!mMonitor)
        return false;
    mMonitor->setListener(this);
    if (!mMonitor->start()) {
        ALOGE("[%s %d] mMonitor start fail !!", __FUNCTION__, __LINE__);
        return false;
    }

    if (!mMonitor->setMonitorConfig(monitorConfig.c_str())) {
        ALOGE("[%s %d] set monitor config fail !!", __FUNCTION__, __LINE__);
        mMonitor->stop();
        return false;
    }
    return true;
}

void ErrorMonitorService::stopErrorMonitor() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    Mutex::Autolock autoLock(mLock);
    if (mMonitor) {
        mMonitor->stop();
        mMonitor->setListener(nullptr);
    }
}

bool ErrorMonitorService::updateMonitorConfig(std::string& monitorConfig) {
    ALOGI("[%s %d]  monitorConfig : %s", __FUNCTION__, __LINE__, monitorConfig.c_str());
    Mutex::Autolock autoLock(mLock);
    return (mMonitor && mMonitor->setMonitorConfig(monitorConfig.c_str())) ? true : false;
}

bool ErrorMonitorService::getMonitorConfig(std::string* monitorConfig) {
    Mutex::Autolock autoLock(mLock);
    bool ret = false;
    if (!mMonitor)
        return ret;
    char* config = mMonitor->getMonitorConfig();
    if (config) {
        *monitorConfig = std::string(config);
        ret = true;
    }
    return ret;
}

bool ErrorMonitorService::setLogLevel(std::string& levelConfig) {
    ALOGI("[%s %d]  logLevelConfig : %s", __FUNCTION__, __LINE__, levelConfig.c_str());
    Mutex::Autolock autoLock(mLock);
    return mLogSetter ? mLogSetter->setLogLevel(levelConfig.c_str()) : false;
}

bool ErrorMonitorService::getLogLevel(std::string* levelConfig) {
    Mutex::Autolock autoLock(mLock);
    if (!mLogSetter)
        return false;
    char* config = mLogSetter->getLogLevel();
    if (!config)
        return false;
    *levelConfig = std::string(config);
    return true;
}

void ErrorMonitorService::notifyErrorInfo(int32_t subModule, int32_t level, int32_t logType, int32_t errorType,
                                          const char* msg) {
    ALOGI("[%s %d] subModule: %d,level: %d,logType: %d,errorType:%d,msg: %s ", __FUNCTION__, __LINE__, subModule, level,
          logType, errorType, msg);
    Mutex::Autolock autoLock(mLock);
    if (mMonitor)
        mMonitor->notifyError(subModule, level, logType, errorType, msg);
}

void ErrorMonitorService::onDataAvailable(std::unique_ptr<ErrorData>& data) {
    ALOGI("[%s %d]  %s", __FUNCTION__, __LINE__, data->toString().c_str());
    sp<ErrorMonitorNotify> cb = mNotifyListener.promote();
    if (cb) {
        char* msg = (char*)malloc(strlen(data->msg));
        memcpy(msg, data->msg, strlen(data->msg));
        cb->onReport(data->mainModule, data->subModule, data->level, data->logType, data->events, msg);
        free(msg);
    }
}

void ErrorMonitorService::onError(const std::string& msg) {
    ALOGI("onError msg=%s", msg.c_str());
    sp<ErrorMonitorNotify> cb = mNotifyListener.promote();
    if (cb)
        cb->onError(msg);
}

}; // namespace android