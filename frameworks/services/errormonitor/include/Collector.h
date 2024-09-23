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
#ifndef AMLOGIC_EERRORMONITOR_COLLECTOR_H
#define AMLOGIC_EERRORMONITOR_COLLECTOR_H

#include <map>
#include <memory>
#include <mutex>
#include "CollectorFilter.h"

namespace android {

typedef enum {
    EERORMONITOR_COLLECTOR_ID_MAIN,
    EERORMONITOR_COLLECTOR_ID_KERNEL,
    EERORMONITOR_COLLECTOR_ID_MODULE,
    EERORMONITOR_COLLECTOR_ID_SYSTEM,
    EERORMONITOR_COLLECTOR_ID_MEDIA,
    EERORMONITOR_COLLECTOR_ID_UNKNOWN,
} errormonitor_collector_id;

class ErrorEvent {
public:
    ErrorEvent(int32_t _mainModule, int64_t _events) {
        mainModule = _mainModule;
        events = _events;
    }
    ~ErrorEvent() = default;
    ErrorEvent(ErrorEvent&) = default;
    ErrorEvent(ErrorEvent&&) = default;
    int32_t mainModule;
    int64_t events;
};

class CollectedData {
public:
    CollectedData()
            : subModule(EERORMONITOR_SUBMODULE_UNKNOWN),
              level(EERORMONITOR_ERROR_LEVEL_SLIGHT),
              logType(0),
              errorType(1),
              timeUs(0),
              msg(nullptr) {};
    CollectedData(int32_t _subModule, int32_t _level, int32_t _logType, int32_t _errorType, int64_t _timeUs,
                  const char* _msg)
            : subModule(_subModule),
              level(_level),
              logType(_logType),
              errorType(_errorType),
              timeUs(_timeUs) {
        if (_msg) {
            msg = (char*)malloc(strlen(_msg));
            memcpy(msg, _msg, strlen(_msg));
        }
    }
    CollectedData(CollectedData& data) {
        subModule = data.subModule;
        level = data.level;
        logType = data.logType;
        errorType = data.errorType;
        timeUs = data.timeUs;
        if (data.msg) {
            msg = (char*)malloc(strlen(data.msg));
            memcpy(msg, data.msg, strlen(data.msg));
        }
    }
    CollectedData(CollectedData&& data) {
        subModule = data.subModule;
        level = data.level;
        logType = data.logType;
        errorType = data.errorType;
        timeUs = data.timeUs;
        msg = data.msg;
        data.msg = nullptr;
    }
    ~CollectedData() {
        if (msg)
            free(msg);
    };
    std::string toString() const {
        std::string info;
        info.append("subModule:");
        info.append(SubModuleString[subModule]);
        info.append(",level:");
        info.append(ErrorLevelString[level]);
        info.append(",logType:");
        bool isGetLogType = false;
        if (logType & EERORMONITOR_LOG_TYPE_LOGCAT) {
            isGetLogType = true;
            info.append("logcat_");
        }
        if (logType & EERORMONITOR_LOG_TYPE_BUGREPORT) {
            isGetLogType = true;
            info.append("bugreport_");
        }
        if (isGetLogType) {
            info.pop_back();
        } else {
            info.append("unknown");
        }
        info.append(",errorType:");
        info.append(std::to_string(errorType));
        info.append(",msg:");
        info.append(msg);
        return info;
    }
    int32_t subModule;
    int32_t level;
    int32_t logType;
    int32_t errorType;
    int64_t timeUs;
    char* msg;
};
class CollectedDataCallback {
public:
    CollectedDataCallback() = default;
    virtual ~CollectedDataCallback() = default;
    virtual void onCollect(std::unique_ptr<CollectedData>& data) = 0;
    virtual void onError(const std::string& msg) = 0;
};

class Collector : public CollectedDataCallback {
public:
    class CollectFilterWrapper : public ErrorDataCallback {
    public:
        CollectFilterWrapper(Collector* _user) { user = _user; }
        virtual ~CollectFilterWrapper() = default;
        void onDataAvailable(std::unique_ptr<ErrorData>& data) {
            if (user)
                user->onFilter(data);
        }
        void onError(const std::string& msg) {
            if (user)
                user->onError(msg);
        }

    private:
        Collector* user;
    };
    Collector();

    virtual ~Collector();

    virtual bool start();

    virtual void stop();

    int32_t getId();

    void addCollector(int32_t id, std::shared_ptr<Collector>& cl);

    std::shared_ptr<Collector> getCollectorById(int id);

    void setErrorDataCallback(ErrorDataCallback* cb);

    void setListener(CollectedDataCallback* cb);

    CollectedDataCallback* getListener();

private:
    void onCollect(std::unique_ptr<CollectedData>& data);
    void onFilter(std::unique_ptr<ErrorData>& data);
    void onError(const std::string& msg);
    void onCollectWork();
    void threadFunc();
    bool mStarted;
    CollectedDataCallback* mCollectedDataCallback;
    ErrorDataCallback* mErrorDataCallback;
    std::unique_ptr<Filter> mFilter;
    std::mutex mLock;
    std::map<int32_t, std::shared_ptr<Collector>> mCollectorsMap;
    std::map<uint32_t, std::shared_ptr<ErrorEvent>> mEventMap;
    std::list<std::unique_ptr<CollectedData>> mErrorDataQueue;
    std::vector<std::thread> mThreadPool;
    std::condition_variable mCondition;

}; // class Collector

}; // namespace android

#endif
