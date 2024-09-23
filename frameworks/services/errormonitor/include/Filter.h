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

#ifndef AMLOGIC_ERRORMONITOR_FILTER_H
#define AMLOGIC_ERRORMONITOR_FILTER_H
#include <list>
#include <mutex>
#include <thread>
#include <vector>
#include "Common.h"
namespace android {
class Filter {
public:
    Filter();

    virtual ~Filter();

    bool start();

    void stop();

    void setListener(ErrorDataCallback* cb);

    void filter(std::unique_ptr<ErrorData>& data);

    std::list<std::unique_ptr<ErrorData>> getOutputQueue();

    virtual bool doFilter(std::unique_ptr<ErrorData>& data) = 0;

private:
    void onFilterWork();
    void onDequeueOutputWork();
    void threadFunc();
    bool mStarted;
    std::mutex mLock;
    ErrorDataCallback* mCallback;
    std::vector<std::thread> mThreadPool;
    std::condition_variable mCondition;
    std::list<std::unique_ptr<ErrorData>> mInputQueue;
    std::list<std::unique_ptr<ErrorData>> mOutputQueue;
};

}; // namespace android
#endif // AMLOGIC_ERRORMONITOR_FILTER_H