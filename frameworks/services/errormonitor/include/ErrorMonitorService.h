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

#ifndef AMLOGIC_ERRORMONITORSERVICE_H
#define AMLOGIC_ERRORMONITORSERVICE_H
#include <list>
#include <utils/RefBase.h>
#include <utils/threads.h>
#include "LogLevelSetter.h"
#include "Monitor.h"
namespace android {
class ErrorMonitorNotify : virtual public RefBase {
public:
    ErrorMonitorNotify() {}
    virtual ~ErrorMonitorNotify() {}
    virtual void onReport(int32_t mainModule, int32_t subModule, int32_t level, int32_t logType, int64_t events,
                          const char* msg) = 0;
    virtual void onError(const std::string& msg) = 0;
};

class ErrorMonitorService : public ErrorDataCallback {
public:
    static ErrorMonitorService* getInstance();

    ErrorMonitorService();

    virtual ~ErrorMonitorService();

    void setListener(const sp<ErrorMonitorNotify>& listener);

    bool startErrorMonitor(std::string& monitorConfig);

    void stopErrorMonitor();

    bool getMonitorConfig(std::string* monitorConfig);

    bool updateMonitorConfig(std::string& monitorConfig);

    bool setLogLevel(std::string& levelConfig);

    bool getLogLevel(std::string* levelConfig);

    void notifyErrorInfo(int32_t subModule, int32_t level, int32_t logType, int32_t errorType, const char* msg);

private:
    void onDataAvailable(std::unique_ptr<ErrorData>& data);
    void onError(const std::string& msg);
    mutable Mutex mLock;
    wp<ErrorMonitorNotify> mNotifyListener;
    std::unique_ptr<Monitor> mMonitor;
    std::unique_ptr<LogLevelSetter> mLogSetter;
};

}; // namespace android

#endif // AMLOGIC_ERRORMONITORSERVICE_H