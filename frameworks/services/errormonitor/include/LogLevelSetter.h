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
#ifndef AMLOGIC_EERRORMONITOR_LOG_LEVEL_SETTER_H
#define AMLOGIC_EERRORMONITOR_LOG_LEVEL_SETTER_H

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include "Common.h"
#include "ResManWrapper.h"

namespace android {

class LogLevelSetter {
public:
    LogLevelSetter() {};

    virtual ~LogLevelSetter() {};

    virtual bool setLogLevel(const char* configString) = 0;

    virtual char* getLogLevel() = 0;
};

class LogLevelSetterImpl : public LogLevelSetter {
public:
    LogLevelSetterImpl();

    virtual ~LogLevelSetterImpl();

    bool setLogLevel(const char* configString);

    char* getLogLevel();

private:
    int32_t mResManFd;
    std::list<std::shared_ptr<MonitorConfig>> mLevelConfigList;
    char* mLevelConfig;
};

class LogLevelSetterImpl1 : public LogLevelSetter {
public:
    LogLevelSetterImpl1();

    virtual ~LogLevelSetterImpl1();

    bool setLogLevel(const char* configString);

    char* getLogLevel();

private:
    char* mLevelConfig;
    std::unique_ptr<ResManWrapper> mResManWrapper;
};

}; // namespace android

#endif // AMLOGIC_EERRORMONITOR_LOG_LEVEL_SETTER_H