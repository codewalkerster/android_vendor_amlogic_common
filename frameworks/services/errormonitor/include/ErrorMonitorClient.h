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
#ifndef AMLOGIC_EERRORMONITOR_CLIENT_H
#define AMLOGIC_EERRORMONITOR_CLIENT_H

#include <list>
#include <mutex>
#include <thread>
#include <utils/Mutex.h>
#include <vendor/amlogic/hardware/errormonitor/1.0/IErrorMonitor.h>
#include <vendor/amlogic/hardware/errormonitor/1.0/IErrorMonitorCallback.h>
#include <vendor/amlogic/hardware/errormonitor/1.0/types.h>
#include "Collector.h"
#include "Common.h"

using ::android::hardware::hidl_string;
using ::android::hardware::Return;
using ::vendor::amlogic::hardware::errormonitor::V1_0::IErrorMonitor;
using ::vendor::amlogic::hardware::errormonitor::V1_0::IErrorMonitorCallback;
namespace android {

class ErrorMonitorClient : public IErrorMonitorCallback {
public:
    class ErrorMonitorCallback : public virtual RefBase {
    public:
        ErrorMonitorCallback() = default;
        virtual ~ErrorMonitorCallback() = default;
        virtual void onReport(int32_t mainModule, int32_t subModule, int32_t level, int32_t logType, int64_t events,
                              const char* msg);
        virtual void onError(const char* msg);
    };

    ErrorMonitorClient();

    virtual ~ErrorMonitorClient();

    static ErrorMonitorClient* getInstance();

    void setErrorMonitorCallback(const sp<ErrorMonitorCallback>& f);

    int32_t startErrorMonitor(std::string& monitorConfig);

    int32_t startErrorMonitor(std::list<std::shared_ptr<MonitorConfig>> configList);

    void stopErrorMonitor();

    bool getMonitorConfig(std::string& monitorConfig);

    bool getMonitorConfig(std::list<std::shared_ptr<MonitorConfig>> configList);

    int32_t updateMonitorConfig(std::string& monitorConfig);

    int32_t updateMonitorConfig(std::list<std::shared_ptr<MonitorConfig>> configList);

    int32_t setLogLevel(std::string& levelConfig);

    int32_t setLogLevel(std::list<std::shared_ptr<MonitorConfig>> configList);

    bool getLogLevel(std::string& levelConfig);

    bool getLogLevel(std::list<std::shared_ptr<MonitorConfig>> configList);

    void notifyErrorInfo(int32_t subModule, int32_t level, int32_t logType, int32_t errorType, const char* msg);

private:
    Return<void> onReport(int32_t mainModule, int32_t subModule, int32_t level, int32_t logType, int64_t events,
                          const hidl_string& msg);
    Return<void> onError(const hidl_string& msg);
    void notifyThreadFunc();
    static ErrorMonitorClient* mInstance;
    wp<ErrorMonitorCallback> mCallBack;
    sp<IErrorMonitor> mErrorMonitor;
    std::list<std::unique_ptr<CollectedData>> mErrorDataQueue;
    std::thread mNotifyThread;
    std::condition_variable mCondition;
    bool isRunningThread;
    std::mutex mLock;
    std::mutex mNotifyLock;
};

} // namespace android

#endif // AMLOGIC_EERRORMONITOR_CLIENT_H