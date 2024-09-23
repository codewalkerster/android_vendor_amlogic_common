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
#define LOG_TAG "ErrorMonitorJsonUtil"
#include <json/json.h>
#include <string>
#include <utils/Log.h>
#include "JsonUtil.h"

namespace android {

bool Json2MonitConfig(const char* json, std::list<std::shared_ptr<MonitorConfig>>& configList) {
    bool ret = false;
    Json::Value value;
    Json::Reader reader;
    if (!json)
        return false;
    std::string cs(json);
    ret = reader.parse(cs, value);
    if (!ret) {
        ALOGE("[%s %d] json parse fail !!! please check the string:%s", __FUNCTION__, __LINE__, json);
        return false;
    }
    if (!value.isArray()) {
        ALOGE("[%s %d] json is not array !!! please check the string:%s", __FUNCTION__, __LINE__, json);
        return false;
    }
    configList.clear();
    for (int i = 0; i < value.size(); i++) {
        auto config = std::make_shared<MonitorConfig>();
        Json::Value item = value[i];
        config->module = item["module"].asInt();
        config->level = item["level"].asInt();
        ALOGD("[%s %d] module:%d,level:%d", __FUNCTION__, __LINE__, config->module, config->level);
        configList.push_back(config);
    }
    ALOGD("[%s %d] success json:%s", __FUNCTION__, __LINE__, json);
    return true;
}

bool MonitConfig2Json(std::list<std::shared_ptr<MonitorConfig>> configList, std::string* json) {
    Json::Value jsonArray;
    std::string jsonStr;
    for (auto it = configList.begin(); it != configList.end(); ++it) {
        Json::Value jsonConfig;
        jsonConfig["module"] = (*it)->module;
        jsonConfig["level"] = (*it)->level;
        jsonArray.append(jsonConfig);
    }
    Json::StreamWriterBuilder factory;
    jsonStr = Json::writeString(factory, jsonArray);
    ALOGD("[%s %d] success jsonStr:%s", __FUNCTION__, __LINE__, jsonStr.c_str());
    *json = std::string(jsonStr);
    return true;
}
bool Json2CollectedData(const char* json, std::unique_ptr<CollectedData>& data) {
    bool ret = false;
    Json::Value value;
    Json::Reader reader;
    if (!json)
        return false;
    std::string cs(json);
    ret = reader.parse(cs, value);
    if (!ret) {
        ALOGE("[%s %d] json parse fail !!! please check the string:%s", __FUNCTION__, __LINE__, json);
        return false;
    }
    data->subModule = value["subModule"].asInt();
    data->level = value["level"].asInt();
    data->logType = value["logType"].asInt();
    data->errorType = value["errorType"].asInt();
    data->timeUs = getNowTimesUs();
    char* msg = (char*)value["msg"].asCString();
    data->msg = (char*)malloc(strlen(msg));
    memcpy(data->msg, msg, strlen(msg));
    ALOGI("[%s %d] %s", __FUNCTION__, __LINE__, data->toString().c_str());
    return true;
}

}; // namespace android
