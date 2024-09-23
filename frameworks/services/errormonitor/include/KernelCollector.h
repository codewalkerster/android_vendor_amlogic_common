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
#ifndef AMLOGIC_EERRORMONITOR_KERNEL_COLLECTOR_H
#define AMLOGIC_EERRORMONITOR_KERNEL_COLLECTOR_H

#include <list>
#include <mutex>
#include <thread>
#include <vector>
#include "Collector.h"
#include "ResManWrapper.h"
namespace android {

class KernelCollector : public Collector {
public:
    KernelCollector();

    virtual ~KernelCollector();

    bool start();

    void stop();

    void onErrorInfo(const char* info, int len);

private:
    void onCollectWork();
    void threadFunc();
    std::mutex mLock;
    std::list<std::unique_ptr<CollectedData>> mErrorDataQueue;
    bool mStarted;
    std::vector<std::thread> mThreadPool;
    std::condition_variable mCondition;
    std::unique_ptr<ResManWrapper> mResManWrapper;
};

}; // namespace android

#endif // AMLOGIC_EERRORMONITOR_KERNEL_COLLECTOR_H