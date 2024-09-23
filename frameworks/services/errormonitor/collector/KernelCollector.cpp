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
#define LOG_TAG "KernelCollector"
#include <utils/Log.h>
#include "JsonUtil.h"
#include "KernelCollector.h"
namespace android {

static void onErrorInfo(void* opaque, const char* info, int len) {
    ALOGI("[%s %d]  info = %s,len = %d", __FUNCTION__, __LINE__, info, len);
    KernelCollector* user = static_cast<KernelCollector*>(opaque);
    if (user)
        user->onErrorInfo(info, len);
}

KernelCollector::KernelCollector() {
    mResManWrapper = std::make_unique<ResManWrapper>("KernelCollector", this);
    ALOGI("KernelCollector construct");
}

KernelCollector::~KernelCollector() {
    ALOGI("~KernelCollector");
}
void KernelCollector::onErrorInfo(const char* info, int len) {
    ALOGI("[%s %d]  info = %s,len = %d", __FUNCTION__, __LINE__, info, len);
    if (!mStarted) {
        ALOGE("[%s %d] the kernel collector has been stopped,so drop it", __FUNCTION__, __LINE__);
        return;
    }
    auto data = std::make_unique<CollectedData>();
    if (!Json2CollectedData(info, data)) {
        ALOGE("[%s %d] the kernel collector has been stopped,so drop it", __FUNCTION__, __LINE__);
        return;
    }
    mErrorDataQueue.push_back(std::move(data));
}
bool KernelCollector::start() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    if (mStarted) {
        ALOGE("[%s %d] the kernel collector has been started !!!", __FUNCTION__, __LINE__);
        return false;
    }
    CollectedDataCallback* cb = getListener();
    if (!cb) {
        ALOGE("[%s %d] can't get the callback", __FUNCTION__, __LINE__);
        return false;
    }
    if (!mResManWrapper || !mResManWrapper->supportRM() || !mResManWrapper->valid()) {
        ALOGE("[%s %d] mResManWrapper init fail !!!", __FUNCTION__, __LINE__);
        if (cb)
            cb->onError("resmanger driver init fail !!!");
        return false;
    }
    if (!mResManWrapper->addErrorInfoCallback(android::onErrorInfo)) {
        ALOGE("[%s %d] addErrorInfoCallback fail !!!", __FUNCTION__, __LINE__);
        if (cb)
            cb->onError("addErrorInfoCallback fail !!!");
        return false;
    }
    mStarted = true;
    mThreadPool.push_back(std::thread(&KernelCollector::threadFunc, this));
    mErrorDataQueue.clear();
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
    return true;
}

void KernelCollector::stop() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::unique_lock<std::mutex> lock(mLock);
    if (!mStarted) {
        ALOGE("[%s %d] the kernel collector has not been started !!!", __FUNCTION__, __LINE__);
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
}

void KernelCollector::onCollectWork() {
    if (mErrorDataQueue.empty())
        return;
    auto data = mErrorDataQueue.begin();
    CollectedDataCallback* cb = getListener();
    if ((*data) && cb) {
        cb->onCollect(*data);
    }
    mErrorDataQueue.pop_front();
}

void KernelCollector::threadFunc() {
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