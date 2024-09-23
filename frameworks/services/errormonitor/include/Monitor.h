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
#ifndef AMLOGIC_EERRORMONITOR_MONITOR_H
#define AMLOGIC_EERRORMONITOR_MONITOR_H

#include <list>
#include <mutex>
#include <thread>
#include <vector>
#include "ModuleCollector.h"
#include "MonitorFilter.h"
namespace android {
class Monitor : public ErrorDataCallback {
public:
    class MonitorFilterWrapper : public ErrorDataCallback {
    public:
        MonitorFilterWrapper(Monitor* _user) { user = _user; }
        virtual ~MonitorFilterWrapper() = default;
        void onDataAvailable(std::unique_ptr<ErrorData>& data) {
            if (user)
                user->onFilter(data);
        }
        void onError(const std::string& msg) {
            if (user)
                user->onError(msg);
        }

    private:
        Monitor* user;
    };
    Monitor();

    virtual ~Monitor();

    bool init();

    bool start();

    void stop();

    bool setMonitorConfig(const char* config);

    char* getMonitorConfig();

    void setListener(ErrorDataCallback* cb);

    void notifyError(int32_t subModule, int32_t level, int32_t logType, int32_t errorType, const char* msg);

private:
    void onFilter(std::unique_ptr<ErrorData>& data);
    void onDataAvailable(std::unique_ptr<ErrorData>& data);
    void onError(const std::string& msg);
    bool mStarted;
    std::unique_ptr<Collector> mCollector;
    std::unique_ptr<MonitorFilter> mFilter;
    std::mutex mLock;
    ErrorDataCallback* mCallback;
    std::list<std::unique_ptr<ErrorData>> mPendingQueue;
    std::string mErrorMsg;
};

}; // namespace android

#endif // AMLOGIC_EERRORMONITOR_MONITOR_H