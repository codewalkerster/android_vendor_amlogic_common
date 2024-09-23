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
#define LOG_TAG "MonitorFilter"
#include <string.h>
#include <utils/Log.h>
#include "JsonUtil.h"
#include "MonitorFilter.h"
namespace android {

MonitorFilter::MonitorFilter()
        : mConfigString(nullptr) {
    ALOGI("MonitorFilter construct");
}

MonitorFilter::~MonitorFilter() {
    ALOGI("~MonitorFilter");
    if (mConfigString)
        free(mConfigString);
}

bool MonitorFilter::doFilter(std::unique_ptr<ErrorData>& data) {
    ALOGI("[%s %d]  %s", __FUNCTION__, __LINE__, data->toString().c_str());
    for (auto it = mConfigList.begin(); it != mConfigList.end(); ++it) {
        if ((*it)->module == data->mainModule && data->level <= (*it)->level) {
            ALOGI("[%s %d]  match the filter rules", __FUNCTION__, __LINE__);
            return true;
        }
    }
    ALOGE("the error event don't match any filter rules, so drop it!");
    return false;
}

bool MonitorFilter::setFilterConfig(const char* config) {
    if (!config)
        return false;
    ALOGI("[%s %d]  config=%s", __FUNCTION__, __LINE__, config);
    if (!Json2MonitConfig(config, mConfigList))
        return false;
    ALOGI("[%s %d] show the config list:", __FUNCTION__, __LINE__);
    for (auto it = mConfigList.begin(); it != mConfigList.end(); ++it) {
        ALOGI("module = %d,level = %d", (*it)->module, (*it)->level);
    }
    if (mConfigString)
        free(mConfigString);
    mConfigString = (char*)malloc(strlen(config));
    memcpy(mConfigString, config, strlen(config));
    return true;
}
char* MonitorFilter::getFilterConfig() {
    ALOGI("[%s %d]  mConfigString = %s", __FUNCTION__, __LINE__, mConfigString);
    return mConfigString;
}

}; // namespace android