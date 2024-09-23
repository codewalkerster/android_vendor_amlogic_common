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
#define LOG_TAG "AmlErrorMonitor"
#include <utils/Log.h>
#include "KernelCollector.h"
#include "MediaCollector.h"
#include "Monitor.h"
namespace android {

Monitor::Monitor()
        : mStarted(false) {
    ALOGI("Monitor construct");
}

Monitor::~Monitor() {
    ALOGI("~Monitor");
    if (mStarted) {
        stop();
    }
    mCollector->stop();
}
bool Monitor::init() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    mCollector = std::make_unique<Collector>();
    mCollector->setErrorDataCallback(this);
    std::shared_ptr<Collector> moduleCollector = std::make_shared<ModuleCollector>();
    mCollector->addCollector(EERORMONITOR_COLLECTOR_ID_MODULE, moduleCollector);
    std::shared_ptr<Collector> kernelCollector = std::make_shared<KernelCollector>();
    mCollector->addCollector(EERORMONITOR_COLLECTOR_ID_KERNEL, kernelCollector);
    std::shared_ptr<Collector> mediaCollector = std::make_shared<MediaCollector>();
    mCollector->addCollector(EERORMONITOR_COLLECTOR_ID_MEDIA, mediaCollector);
    mPendingQueue.clear();
    return mCollector->start();
}

bool Monitor::start() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    if (mStarted) {
        ALOGE("[%s %d] the Monitor has been started !!!", __FUNCTION__, __LINE__);
        return false;
    }
    while (!mPendingQueue.empty()) {
        auto input = mPendingQueue.begin();
        if ((*input) && mCallback) {
            mCallback->onDataAvailable(*input);
        }
        mPendingQueue.pop_front();
    }
    mFilter = std::make_unique<MonitorFilter>();
    mFilter->setListener(new MonitorFilterWrapper(this));
    mFilter->start();
    mStarted = true;
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
    return true;
}
void Monitor::setListener(ErrorDataCallback* cb) {
    mCallback = cb;
    if (mErrorMsg.size() > 0)
        mCallback->onError(mErrorMsg);
}
void Monitor::stop() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    if (!mStarted) {
        ALOGE("[%s %d] the module collector has not been started !!!", __FUNCTION__, __LINE__);
        return;
    }
    mStarted = false;
    mFilter->setListener(nullptr);
    mFilter->stop();
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
}
void Monitor::onFilter(std::unique_ptr<ErrorData>& data) {
    if (!mStarted || !mCallback)
        return;
    mCallback->onDataAvailable(data);
}

bool Monitor::setMonitorConfig(const char* config) {
    ALOGI("[%s %d]  config=%s", __FUNCTION__, __LINE__, config);
    std::lock_guard<std::mutex> lock(mLock);
    return (mStarted && mFilter) ? mFilter->setFilterConfig(config) : false;
}

char* Monitor::getMonitorConfig() {
    std::lock_guard<std::mutex> lock(mLock);
    return (mStarted && mFilter) ? mFilter->getFilterConfig() : nullptr;
}
void Monitor::notifyError(int32_t subModule, int32_t level, int32_t logType, int32_t errorType, const char* msg) {
    ALOGI("[%s %d] subModule: %d,level: %d,logType: %d,errorType:%d,msg: %s ", __FUNCTION__, __LINE__, subModule, level,
          logType, errorType, msg);
    if (mCollector) {
        std::shared_ptr<ModuleCollector> mc =
            std::static_pointer_cast<ModuleCollector>(mCollector->getCollectorById(EERORMONITOR_COLLECTOR_ID_MODULE));
        if (mc)
            mc->notifyError(subModule, level, logType, errorType, msg);
    }
}

void Monitor::onDataAvailable(std::unique_ptr<ErrorData>& data) {
    if (!data)
        return;
    ALOGI("[%s %d]  mStarted = %s,%s", __FUNCTION__, __LINE__, mStarted ? "true" : "false", data->toString().c_str());
    if (!mStarted)
        return;
    if (mFilter)
        mFilter->filter(data);
}

void Monitor::onError(const std::string& msg) {
    ALOGI("onError msg=%s", msg.c_str());
    mErrorMsg.clear();
    mErrorMsg = msg;
    if (mCallback) {
        mCallback->onError(mErrorMsg);
    }
}
}; // namespace android