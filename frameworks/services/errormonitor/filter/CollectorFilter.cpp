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
#define LOG_TAG "CollectorFilter"
#include <utils/Log.h>
#include "CollectorFilter.h"

namespace android {
#define DATA_TIMEOUT_US 5 * 100 * 1000 // 500ms

CollectorFilter::CollectorFilter() {
    ALOGI("CollectorFilter construct");
    mStarted = true;
    mThreadPool.push_back(std::thread(&CollectorFilter::threadFunc, this));
    mErrorDateList.clear();
}

CollectorFilter::~CollectorFilter() {
    ALOGI("~CollectorFilter");
    std::unique_lock<std::mutex> lock(mLock);
    mStarted = false;
    mCondition.wait(lock);
    for (int i = 0; i < mThreadPool.size(); i++) {
        mThreadPool[i].join();
    }
    mErrorDateList.clear();
}

bool CollectorFilter::doFilter(std::unique_ptr<ErrorData>& data) {
    ALOGI("[%s %d]  %s", __FUNCTION__, __LINE__, data->toString().c_str());
    std::lock_guard<std::mutex> lock(mLock);
    if (!filterRepeatData(data)) {
        ALOGE("[%s %d] the repeated data don't need to be collected", __FUNCTION__, __LINE__);
        return false;
    }
    mErrorDateList.push_back(std::make_shared<ErrorData>(*data));
    return true;
}
bool CollectorFilter::filterRepeatData(std::unique_ptr<ErrorData>& data) {
    for (auto it = mErrorDateList.begin(); it != mErrorDateList.end(); ++it) {
        if (*data == **it) {
            ALOGI("[%s %d]  find the repeat data: %s", __FUNCTION__, __LINE__, data->toString().c_str());
            return false;
        }
    }
    return true;
}
void CollectorFilter::threadFunc() {
    while (1) {
        {
            std::lock_guard<std::mutex> lock(mLock);
            if (!mStarted) {
                mCondition.notify_one();
                break;
            }
            int64_t nowTimeUs = getNowTimesUs();

            for (auto it = mErrorDateList.begin(); it != mErrorDateList.end(); ++it) {
                if ((nowTimeUs - (*it)->timeUs) > DATA_TIMEOUT_US) {
                    mErrorDateList.erase(it);
                }
            }
        }
        usleep(5 * 1000); // 5ms
    }
}
}; // namespace android
