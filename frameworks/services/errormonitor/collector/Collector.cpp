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
#define LOG_TAG "ErrorMonitorCollector"
#include <utils/Log.h>
#include "Collector.h"
#include "EventMapUtil.h"
namespace android {
Collector::Collector()
        : mStarted(false),
          mCollectedDataCallback(nullptr) {
    ALOGI("Collector construct");

}

Collector::~Collector() {
    ALOGI("~Collector");
    if (mStarted) {
        stop();
    }
}

bool Collector::start() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    if (mStarted) {
        ALOGE("[%s %d] the collector has been started !!!", __FUNCTION__, __LINE__);
        return false;
    }
    for (auto it = mCollectorsMap.begin(); it != mCollectorsMap.end(); it++) {
        if (!it->second->start())
            return false;
    }
    initEventMap(mEventMap);
    mFilter = std::make_unique<CollectorFilter>();
    mFilter->setListener(new CollectFilterWrapper(this));
    mFilter->start();
    mStarted = true;
    mThreadPool.push_back(std::thread(&Collector::threadFunc, this));
    mErrorDataQueue.clear();
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
    return true;
}

void Collector::stop() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::unique_lock<std::mutex> lock(mLock);
    if (!mStarted) {
        ALOGE("[%s %d] the collector has not been started !!!", __FUNCTION__, __LINE__);
        return;
    }
    for (auto it = mCollectorsMap.begin(); it != mCollectorsMap.end(); it++) {
        it->second->stop();
    }
    mStarted = false;
    mFilter->stop();
    mCondition.wait(lock);
    for (int i = 0; i < mThreadPool.size(); i++) {
        mThreadPool[i].join();
    }
    mThreadPool.clear();
    mErrorDataQueue.clear();
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
}

void Collector::addCollector(int32_t id, std::shared_ptr<Collector>& cl) {
    ALOGI("[%s %d]  id = %d ", __FUNCTION__, __LINE__, id);
    std::lock_guard<std::mutex> lock(mLock);
    if (id < 0 || id >= EERORMONITOR_COLLECTOR_ID_UNKNOWN || !cl) {
        ALOGE("[%s %d] the id:%d doesn't exist or cl is null", __FUNCTION__, __LINE__, id);
        return;
    }
    cl->setListener(this);
    mCollectorsMap.insert(std::pair<int32_t, std::shared_ptr<Collector>>(id, cl));
}

std::shared_ptr<Collector> Collector::getCollectorById(int id) {
    ALOGI("[%s %d]  id = %d ", __FUNCTION__, __LINE__, id);
    std::lock_guard<std::mutex> lock(mLock);
    if (id < 0 || id >= EERORMONITOR_COLLECTOR_ID_UNKNOWN) {
        ALOGE("[%s %d] the id:%d doesn't exist", __FUNCTION__, __LINE__, id);
        return nullptr;
    }
    auto it = mCollectorsMap.find(id);
    if (it != mCollectorsMap.end()) {
        return it->second;
    }
    return nullptr;
}

void Collector::setErrorDataCallback(ErrorDataCallback* cb) {
    mErrorDataCallback = cb;
}

void Collector::setListener(CollectedDataCallback* cb) {
    mCollectedDataCallback = cb;
}

CollectedDataCallback* Collector::getListener() {
    return mCollectedDataCallback;
}

void Collector::onError(const std::string& msg) {
    ALOGI("onError msg=%s", msg.c_str());
    if (mErrorDataCallback)
        mErrorDataCallback->onError(msg);
}

void Collector::onCollect(std::unique_ptr<CollectedData>& data) {
    std::lock_guard<std::mutex> lock(mLock);
    ALOGI("[%s %d] %s", __FUNCTION__, __LINE__, data->toString().c_str());
    mErrorDataQueue.push_back(std::move(data));
}

void Collector::onFilter(std::unique_ptr<ErrorData>& data) {
    ALOGI("[%s %d]  %s", __FUNCTION__, __LINE__, data->toString().c_str());
    if (mErrorDataCallback)
        mErrorDataCallback->onDataAvailable(data);
}

void Collector::onCollectWork() {
    if (mErrorDataQueue.empty())
        return;
    std::shared_ptr<ErrorEvent> event;
    auto data = mErrorDataQueue.begin();
    uint32_t test1 = ((*data)->subModule << 16) | (*data)->errorType;
    auto it = mEventMap.find(((*data)->subModule << 16) | (*data)->errorType);
    if (it != mEventMap.end()) {
        event = it->second;
        ALOGD("get the mainModule:%d,events:0x%02x", event->mainModule, event->events);
        std::unique_ptr<ErrorData> error = std::make_unique<ErrorData>(event->mainModule, (*data)->subModule, (*data)->level,
                                                    (*data)->logType, event->events, (*data)->timeUs, (*data)->msg);
        ALOGD("get the error %s", error->toString().c_str());
        mFilter->filter(error);
    } else {
        ALOGE("the data can't be changed to event, %s", (*data)->toString().c_str());
    }
    mErrorDataQueue.pop_front();
}
void Collector::threadFunc() {
    while (1) {
        {
            std::lock_guard<std::mutex> lock(mLock);
            if (!mStarted) {
                mCondition.notify_one();
                break;
            }
            onCollectWork();
        }
        usleep(5 * 1000); // 5ms
    }
}
}; // namespace android
