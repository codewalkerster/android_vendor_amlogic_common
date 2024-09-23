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
#define LOG_TAG "ModuleCollector"
#include <utils/Log.h>
#include "ModuleCollector.h"
namespace android {
ModuleCollector::ModuleCollector()
        : mStarted(false) {
    ALOGI("ModuleCollector construct");
}

ModuleCollector::~ModuleCollector() {
    ALOGI("~ModuleCollector");
    if (mStarted) {
        stop();
    }
}

bool ModuleCollector::start() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    if (mStarted) {
        ALOGE("[%s %d] the module collector has been started !!!", __FUNCTION__, __LINE__);
        return false;
    }
    mStarted = true;
    mThreadPool.push_back(std::thread(&ModuleCollector::threadFunc, this));
    mErrorDataQueue.clear();
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
    return true;
}

void ModuleCollector::stop() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::unique_lock<std::mutex> lock(mLock);
    if (!mStarted) {
        ALOGE("[%s %d] the module collector has not been started !!!", __FUNCTION__, __LINE__);
        return;
    }
    mStarted = false;
    mCondition.wait(lock);
    for (int i = 0; i < mThreadPool.size(); i++) {
        mThreadPool[i].join();
    }
    mThreadPool.clear();
    mErrorDataQueue.clear();
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
    return;
}

void ModuleCollector::notifyError(int32_t subModule, int32_t level, int32_t logType, int32_t errorType,
                                  const char* msg) {
    ALOGI("[%s %d] subModule: %d,level: %d,logType: %d,errorType:%d,msg: %s ", __FUNCTION__, __LINE__, subModule, level,
          logType, errorType, msg);
    std::lock_guard<std::mutex> lock(mLock);
    if (!mStarted) {
        ALOGE("[%s %d] the module collector has not been started !!!", __FUNCTION__, __LINE__);
        return;
    }
    if (!msg) {
        ALOGE("[%s %d] the message is null !!!", __FUNCTION__, __LINE__);
        return;
    }
    auto data = std::make_unique<CollectedData>(subModule, level, logType, errorType, getNowTimesUs(), msg);
    mErrorDataQueue.push_back(std::move(data));
}

void ModuleCollector::onCollectWork() {
    if (mErrorDataQueue.empty())
        return;
    auto data = mErrorDataQueue.begin();
    CollectedDataCallback* cb = getListener();
    if ((*data) && cb) {
        cb->onCollect(*data);
    }
    mErrorDataQueue.pop_front();
}

void ModuleCollector::threadFunc() {
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