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
#define LOG_TAG "ErrorMonitorClient"
#include <inttypes.h>
#include <log/log.h>
#include <string>
#include <utils/String8.h>
#include "ErrorMonitorClient.h"
#include "JsonUtil.h"

using ::android::hardware::hidl_string;
using ::android::hardware::Void;
using ::vendor::amlogic::hardware::errormonitor::V1_0::Result;
namespace android {

ErrorMonitorClient* ErrorMonitorClient::mInstance = nullptr;

ErrorMonitorClient::ErrorMonitorClient()
        : isRunningThread(true) {
    sp<IErrorMonitor> ctrl = IErrorMonitor::tryGetService();
    while (ctrl == nullptr) {
        usleep(200 * 1000); // sleep 200ms
        ctrl = IErrorMonitor::tryGetService();
        ALOGE("tryGet screen control daemon Service");
    };
    mErrorMonitor = std::move(ctrl);
    mNotifyThread = std::thread(&ErrorMonitorClient::notifyThreadFunc, this);
}

ErrorMonitorClient::~ErrorMonitorClient() {
    ALOGD("~ErrorMonitorClient");
    if (mInstance)
        delete mInstance;
    std::unique_lock<std::mutex> lock(mNotifyLock);
    isRunningThread = false;
    mCondition.wait(lock);
    mNotifyThread.join();
    mErrorDataQueue.clear();
}

ErrorMonitorClient* ErrorMonitorClient::getInstance() {
    if (nullptr == mInstance)
        mInstance = new ErrorMonitorClient();
    return mInstance;
}

void ErrorMonitorClient::setErrorMonitorCallback(const sp<ErrorMonitorCallback>& f) {
    mCallBack = f;
}

int32_t ErrorMonitorClient::startErrorMonitor(std::string& monitorConfig) {
    ALOGI("[%s %d]  monitorConfig : %s", __FUNCTION__, __LINE__, monitorConfig.c_str());
    std::lock_guard<std::mutex> lock(mLock);
    mErrorMonitor->setCallback(this);
    return (Result::OK == mErrorMonitor->startErrorMonitor(monitorConfig)) ? 0 : -1;
}

int32_t ErrorMonitorClient::startErrorMonitor(std::list<std::shared_ptr<MonitorConfig>> configList) {
    std::string configString;
    if (!MonitConfig2Json(configList, &configString))
        return -1;
    ALOGI("[%s %d]  configString : %s", __FUNCTION__, __LINE__, configString.c_str());
    return !configString.empty() ? startErrorMonitor(configString) : -1;
}

void ErrorMonitorClient::stopErrorMonitor() {
    ALOGI("[%s %d] begin", __FUNCTION__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    mErrorMonitor->setCallback(nullptr);
    mErrorMonitor->stopErrorMonitor();
}

bool ErrorMonitorClient::getMonitorConfig(std::string& monitorConfig) {
    std::lock_guard<std::mutex> lock(mLock);
    mErrorMonitor->getMonitorConfig([&monitorConfig](const Result& ret, const hidl_string& config) {
        ALOGI("[%s %d] config =%s", __FUNCTION__, __LINE__, config.c_str());
        if (Result::OK == ret) {
            monitorConfig = config.c_str();
            ALOGI("[%s %d] monitorConfig =%s", __FUNCTION__, __LINE__, monitorConfig.c_str());
        } else
            monitorConfig.clear();
    });
    return monitorConfig.empty() ? false : true;
}

bool ErrorMonitorClient::getMonitorConfig(std::list<std::shared_ptr<MonitorConfig>> configList) {
    std::string config;
    return getMonitorConfig(config) ? Json2MonitConfig(config.c_str(), configList) ? true : false : false;
}
int32_t ErrorMonitorClient::updateMonitorConfig(std::string& monitorConfig) {
    ALOGI("[%s %d]  monitorConfig : %s", __FUNCTION__, __LINE__, monitorConfig.c_str());
    std::lock_guard<std::mutex> lock(mLock);
    int32_t result = -1;
    if (Result::OK == mErrorMonitor->updateMonitorConfig(monitorConfig))
        result = 0;
    return result;
}

int32_t ErrorMonitorClient::updateMonitorConfig(std::list<std::shared_ptr<MonitorConfig>> configList) {
    std::string configString;
    if (!MonitConfig2Json(configList, &configString))
        return -1;
    return !configString.empty() ? updateMonitorConfig(configString) : -1;
}

int32_t ErrorMonitorClient::setLogLevel(std::string& levelConfig) {
    ALOGI("[%s %d]  levelConfig : %s", __FUNCTION__, __LINE__, levelConfig.c_str());
    std::lock_guard<std::mutex> lock(mLock);
    return (Result::OK == mErrorMonitor->setLogLevel(levelConfig)) ? 0 : -1;
}

int32_t ErrorMonitorClient::setLogLevel(std::list<std::shared_ptr<MonitorConfig>> configList) {
    std::string configString;
    if (!MonitConfig2Json(configList, &configString))
        return -1;
    return !configString.empty() ? setLogLevel(configString) : -1;
}

bool ErrorMonitorClient::getLogLevel(std::string& levelConfig) {
    std::lock_guard<std::mutex> lock(mLock);
    mErrorMonitor->getLogLevel([&levelConfig](const Result& ret, const hidl_string& config) {
        if (Result::OK == ret) {
            levelConfig = config.c_str();
        } else
            levelConfig.clear();
    });
    return levelConfig.empty() ? false : true;
}

bool ErrorMonitorClient::getLogLevel(std::list<std::shared_ptr<MonitorConfig>> configList) {
    std::string config;
    return getLogLevel(config) ? Json2MonitConfig(config.c_str(), configList) ? true : false : false;
}
void ErrorMonitorClient::notifyErrorInfo(int32_t subModule, int32_t level, int32_t logType, int32_t errorType,
                                         const char* msg) {
    ALOGI("[%s %d] subModule: %d,level: %d,logType: %d,errorType:%d,msg: %s ", __FUNCTION__, __LINE__, subModule, level,
          logType, errorType, msg);
    std::lock_guard<std::mutex> lock(mLock);
    std::unique_lock<std::mutex> notifyLock(mNotifyLock);
    auto data = std::make_unique<CollectedData>(subModule, level, logType, errorType, 0, msg);
    mErrorDataQueue.push_back(std::move(data));
}

Return<void> ErrorMonitorClient::onReport(int32_t mainModule, int32_t subModule, int32_t level, int32_t logType,
                                          int64_t events, const hidl_string& msg) {
    ALOGI("[%s %d] mainModule: %d, subModule: %d,level: %d,logType: %d,msg: %s,events:0x%02x", __FUNCTION__, __LINE__,
          mainModule, subModule, level, logType, msg.c_str(),events);
    ALOGI("[%s %d]  msg : %s", __FUNCTION__, __LINE__, msg.c_str());
    sp<ErrorMonitorCallback> f = mCallBack.promote();
    if (f != nullptr) {
        f->onReport(mainModule, subModule, level, logType, events, msg.c_str());
    }
    return Void();
}

Return<void> ErrorMonitorClient::onError(const hidl_string& msg) {
    ALOGI("onError msg=%s", msg.c_str());
    sp<ErrorMonitorCallback> f = mCallBack.promote();
    if (f != nullptr) {
        f->onError(msg.c_str());
    }
    return Void();
}
void ErrorMonitorClient::notifyThreadFunc() {
    while (1) {
        {
            std::lock_guard<std::mutex> lock(mNotifyLock);
            if (!isRunningThread) {
                mCondition.notify_one();
                break;
            }
            if (!mErrorDataQueue.empty()) {
                auto data = mErrorDataQueue.begin();
                std::string msgString = (*data)->msg;
                mErrorMonitor->notifyErrorInfo((*data)->subModule, (*data)->level, (*data)->logType, (*data)->errorType,
                                               msgString);
                mErrorDataQueue.pop_front();
            }
        }
        usleep(5 * 1000); // 5ms
    }
}

void AmlogicErrorNotify(int32_t subModule, int32_t level, int32_t logType, int32_t errorType, const char* msg) {
    ErrorMonitorClient* client = ErrorMonitorClient::getInstance();
    client->notifyErrorInfo(subModule, level, logType, errorType, msg);
    return;
}

}; // namespace android