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
#define LOG_TAG "LogLevelSetter"
#include <string.h>
#include <string>
#include <utils/Log.h>
#include "JsonUtil.h"
#include "LogLevelSetter.h"

namespace android {
static std::string video_tag[] = {"Omx", "Codec2", "Drmplayer", "Tsplayer", "Drm_service", "Cas_service", "Dec"};

#define RESMAN_DEBUG_NODE "/sys/class/resource_mgr/res_sys_debug"

LogLevelSetterImpl::LogLevelSetterImpl()
        : mLevelConfig(nullptr) {
    mLevelConfigList.clear();
    mResManFd = open(RESMAN_DEBUG_NODE, O_RDWR);
    ALOGI("LogLevelSetterImpl construct mResManFd =%d", mResManFd);
}

LogLevelSetterImpl::~LogLevelSetterImpl() {
    ALOGI("~LogLevelSetterImpl");
    mLevelConfigList.clear();
    if (mResManFd > 0)
        close(mResManFd);
    if (mLevelConfig)
        free(mLevelConfig);
}

bool LogLevelSetterImpl::setLogLevel(const char* configString) {
    if (!Json2MonitConfig(configString, mLevelConfigList))
        return false;
    ALOGI("[%s %d] show the config list:", __FUNCTION__, __LINE__);
    for (auto it = mLevelConfigList.begin(); it != mLevelConfigList.end(); ++it) {
        ALOGI("module = %d,level = %d", (*it)->module, (*it)->level);
    }
    auto findLogModuleName = [](int32_t module) -> std::string {
        std::string logModule;
        switch (module) {
            case EERORMONITOR_MODULE_NAME_VIDEO:
                logModule = "Video";
                break;
            case EERORMONITOR_MODULE_NAME_AUDIO:
                logModule = "Audio";
                break;
            case EERORMONITOR_MODULE_NAME_DISPLAY:
                logModule = "Display";
                break;
            case EERORMONITOR_MODULE_NAME_ENCODE:
                logModule = "Encode";
                break;
            case EERORMONITOR_MODULE_NAME_DVB:
                logModule = "Dvb";
                break;
            default:
                break;
        }
        return logModule;
    };
    auto findLogLevel = [](int32_t level) -> std::string {
        std::string logLevel;
        switch (level) {
            case AML_SYS_LOGLEVEL_FATAL:
                logLevel = "0";
                break;
            case AML_SYS_LOGLEVEL_ERROR:
                logLevel = "1";
                break;
            case AML_SYS_LOGLEVEL_WARN:
                logLevel = "2";
                break;
            case AML_SYS_LOGLEVEL_INFO:
                logLevel = "3";
                break;
            case AML_SYS_LOGLEVEL_DEBUG_0:
                logLevel = "4";
                break;
            case AML_SYS_LOGLEVEL_DEBUG_1:
                logLevel = "5";
                break;
            case AML_SYS_LOGLEVEL_DEBUG_2:
                logLevel = "6";
                break;
            case AML_SYS_LOGLEVEL_VERBOSE_0:
                logLevel = "7";
                break;
            case AML_SYS_LOGLEVEL_VERBOSE_1:
                logLevel = "8";
                break;
            case AML_SYS_LOGLEVEL_VERBOSE_2:
                logLevel = "9";
                break;
            case AML_SYS_LOGLEVEL_TRACE:
                logLevel = "10";
                break;
            default:
                break;
        }
        return logLevel;
    };
    std::string debugString;
    for (auto it = mLevelConfigList.begin(); it != mLevelConfigList.end(); ++it) {
        std::string moduleName = findLogModuleName((*it)->module);
        std::string level = findLogLevel((*it)->level);
        if (moduleName.empty() || level.empty())
            continue;
        int count = sizeof(video_tag) / sizeof(video_tag[0]);
        switch ((*it)->module) {
            case EERORMONITOR_MODULE_NAME_VIDEO:
                for (int i = 0; i < count; i++) {
                    debugString.append(moduleName);
                    debugString.append("_");
                    debugString.append(video_tag[i]);
                    debugString.append(":debuglevel:");
                    debugString.append(level);
                    debugString.append(";");
                }
                break;
            case EERORMONITOR_MODULE_NAME_AUDIO:
            case EERORMONITOR_MODULE_NAME_DISPLAY:
            case EERORMONITOR_MODULE_NAME_ENCODE:
            case EERORMONITOR_MODULE_NAME_DVB:
            default:
                ALOGE("the module are not supported !! name:%d", (*it)->module);
                break;
        }
    }
    if (!debugString.empty() && mResManFd > 0) {
        ALOGI("[%s %d]  set the debug string to resmanger driver :%s", __FUNCTION__, __LINE__, debugString.c_str());
        mLevelConfig = (char*)malloc(strlen(configString));
        memcpy(mLevelConfig, configString, strlen(configString));
        write(mResManFd, debugString.c_str(), debugString.size());
        return true;
    }
    return false;
}

char* LogLevelSetterImpl::getLogLevel() { return mLevelConfig ? mLevelConfig : nullptr; }

LogLevelSetterImpl1::LogLevelSetterImpl1()
        : mLevelConfig(nullptr) {
    mResManWrapper = std::make_unique<ResManWrapper>("LogLevelSetterImpl1", this);
    ALOGI("LogLevelSetterImpl1 construct");
}

LogLevelSetterImpl1::~LogLevelSetterImpl1() {
    ALOGI("~LogLevelSetterImpl1");
    if (mLevelConfig)
        free(mLevelConfig);
}

bool LogLevelSetterImpl1::setLogLevel(const char* configString) {
    if (!mResManWrapper || !mResManWrapper->supportRM() || !mResManWrapper->valid()) {
        ALOGE("[%s %d] mResManWrapper init fail !!!", __FUNCTION__, __LINE__);
        return false;
    }
    ALOGI("LogLevelSetterImpl1 setLogLevel configString = %s", configString);
    return mResManWrapper->setDebugInfo(configString) ? true : false;
}

char* LogLevelSetterImpl1::getLogLevel() { return mLevelConfig ? mLevelConfig : nullptr; }

} // namespace android
