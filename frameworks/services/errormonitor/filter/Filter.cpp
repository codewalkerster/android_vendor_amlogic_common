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
#define LOG_TAG "ErrorMonitorFilter"
#include <utils/Log.h>
#include "Filter.h"
namespace android {
Filter::Filter() {
    ALOGI("Filter construct");
}

Filter::~Filter() {
    ALOGI("~Filter");
    if (mStarted) {
        stop();
    }
}

bool Filter::start() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    if (mStarted) {
        ALOGE("[%s %d] the Filter has been started !!!", __FUNCTION__, __LINE__);
        return false;
    }
    mStarted = true;
    mThreadPool.push_back(std::thread(&Filter::threadFunc, this));
    mInputQueue.clear();
    mOutputQueue.clear();
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
    return true;
}

void Filter::stop() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::unique_lock<std::mutex> lock(mLock);
    if (!mStarted) {
        ALOGE("[%s %d] the Filter has not been started !!!", __FUNCTION__, __LINE__);
        return;
    }
    mStarted = false;
    mCondition.wait(lock);
    for (int i = 0; i < mThreadPool.size(); i++) {
        mThreadPool[i].join();
    }
    mInputQueue.clear();
    mOutputQueue.clear();
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
    return;
}

void Filter::setListener(ErrorDataCallback* cb) {
    mCallback = cb;
}

void Filter::filter(std::unique_ptr<ErrorData>& data) {
    ALOGI("[%s %d] %s", __FUNCTION__, __LINE__,data->msg);
    ALOGI("[%s %d] %s", __FUNCTION__, __LINE__,data->toString().c_str());
    std::lock_guard<std::mutex> lock(mLock);
    if (!mStarted) {
        ALOGE("[%s %d] the Filter has not been started !!!", __FUNCTION__, __LINE__);
        return;
    }
    mInputQueue.push_back(std::move(data));
}

void Filter::onFilterWork() {
    if (mInputQueue.empty())
        return;
    auto data = mInputQueue.begin();
    if (!(*data))
        return;
    if (doFilter(*data)) {
        ALOGI("[%s %d] %s", __FUNCTION__, __LINE__,(*data)->toString().c_str());
        mOutputQueue.push_back(std::move(*data));
    }
    mInputQueue.pop_front();
}
void Filter::onDequeueOutputWork() {
    if (mOutputQueue.empty())
        return;
    auto data = mOutputQueue.begin();
    if ((*data) && mCallback) {
        mCallback->onDataAvailable(*data);
    }
    mOutputQueue.pop_front();
}

void Filter::threadFunc() {
    while (1) {
        {
            std::lock_guard<std::mutex> lock(mLock);
            if (!mStarted) {
                mCondition.notify_one();
                break;
            }
            onFilterWork();
            onDequeueOutputWork();
        }
        usleep(5 * 1000); // 5ms
    }
}
}; // namespace android
