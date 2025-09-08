/*
 * Copyright (C) 2017 Amlogic Corporation.
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
#define LOG_TAG "DroidAudioMpeghSetting"
#include "DroidAudioMpeghSetting.h"
#include "DroidAudioCommon.h"
#include <string>

#define ACTION_EVENT_KEY "mpegh_action_event="
#define ACTION_PERSIST_KEY "mpegh_persistency_ctx="
#define ACTION_EVENT_RESET 0
#define SYSTEM_MAX_CMD_ID 8

static const char* DB_KEY_AM_AUDIO_CONFIG_MPEGH_PERSIST                        = "db_key_am_audio_config_mpegh_persist";

static const std::string mpeg_system_cmd[9] = {
    std::string("mpegh_audio_lang"),
    std::string("mpegh_label_lang"),
    std::string("mpegh_drc_att"),
    std::string("mpegh_accessibility"),
    std::string("mpegh_drc_effect"),
    std::string("mpegh_drc_album"),
    std::string("mpegh_drc_boost"),
    std::string("mpegh_tl"),
    std::string("mpegh_reset"),
};

static std::map<int32_t, std::string> mpeghSystemMap = {
    {70, mpeg_system_cmd[0]},
    {71, mpeg_system_cmd[1]},
    {12, mpeg_system_cmd[2]},
    {31, mpeg_system_cmd[3]},
    {10, mpeg_system_cmd[4]},
    {21, mpeg_system_cmd[5]},
    {11, mpeg_system_cmd[6]},
    {20, mpeg_system_cmd[7]},
    {0,  mpeg_system_cmd[8]}
};

static const vector<const char*> g_VecDbString = {
    DB_KEY_AM_AUDIO_CONFIG_MPEGH_PERSIST,
    mpeg_system_cmd[0].c_str(),
    mpeg_system_cmd[1].c_str(),
    mpeg_system_cmd[2].c_str(),
    mpeg_system_cmd[3].c_str(),
    mpeg_system_cmd[4].c_str(),
    mpeg_system_cmd[5].c_str(),
    mpeg_system_cmd[6].c_str(),
    mpeg_system_cmd[7].c_str(),
    mpeg_system_cmd[8].c_str(),
};

DroidAudioMpeghSetting::DroidAudioMpeghSetting() :
    DroidAudioDbDescriptor(DroidAudioDbDescriptor::DROIDLOGIC_DB_MODULE_ID_AM_AUDIO_MPEGH, g_VecDbString) {
}

DroidAudioMpeghSetting::~DroidAudioMpeghSetting() {

}

int32_t DroidAudioMpeghSetting::setSystemConfig(int in_id, const std::string& in_value) {
    auto event= mpeghSystemMap.find(in_id);
    if (event == mpeghSystemMap.end()) {
        ALOGE("invalid mpegh system key:%d", in_id);
        return -1;
    } else {
        if (in_id == ACTION_EVENT_RESET) {
            ALOGI("receive mpegh reset from app");
            string persistStr = getStrFromDb(DB_KEY_AM_AUDIO_CONFIG_MPEGH_PERSIST);
            if (false == persistStr.empty()) {
                std::fill(persistStr.begin(), persistStr.end(), '0');
                setParameters(ACTION_PERSIST_KEY + persistStr);
                putToDb(DB_KEY_AM_AUDIO_CONFIG_MPEGH_PERSIST, persistStr);
                ALOGI("reset persistXml:%s", persistStr.c_str());
            }
        } else {
            string params = event->second + "=" + in_value;
            setParameters(params);
            putToDb(event->second, in_value);
            ALOGI("setSystemConfig key:%s value:%s", event->second.c_str(), params.c_str());
        }
    }
    return 0;
}

std::string DroidAudioMpeghSetting::getSystemConfig(int in_id) {
    auto event= mpeghSystemMap.find(in_id);
    string retValue;
    if (event == mpeghSystemMap.end()) {
        ALOGE("invalid mpegh system key:%d", in_id);
        retValue = "0";
    } else {
        retValue = getStrFromDb(event->second);
        if (retValue.empty()) {
            ALOGI("getStrFromDb is empty, key:%s value:%s", event->second.c_str(), retValue.c_str());
            retValue = "0";
        }
        ALOGI("getSystemConfig key:%s value:%s", event->second.c_str(), retValue.c_str());
    }
    return retValue;
}

int32_t DroidAudioMpeghSetting::setActionEvent(const std::string& in_value) {
    std::string keyValueStr = std::string(ACTION_EVENT_KEY) + in_value;
    setParameters(keyValueStr);
    ALOGI("setActionEvent str:%s ", keyValueStr.c_str());
    return 0;
}

std::string DroidAudioMpeghSetting::getXmlSceneInfo() {
    std::string params = getParametersDirect(std::string("mpegh_audiosceneconfig"));
    ALOGI("getXmlSceneInfo direct:%s", params.c_str());
    return params;
}

vector<uint8_t> DroidAudioMpeghSetting::getDefaultValue(const string& key) {
    int32_t value = 0;
    string key_tmp = key;
    vector<uint8_t> valueBytes(sizeof(int32_t));
    memcpy(valueBytes.data(), &value, sizeof(int32_t));
    return valueBytes;
}

void DroidAudioMpeghSetting::storePersistContext() {
    std::string params = getParametersDirect(std::string("mpegh_persistency_ctx"));
    ALOGI("params:%s", params.c_str());
    putToDb(DB_KEY_AM_AUDIO_CONFIG_MPEGH_PERSIST, params);
    return;
}

void DroidAudioMpeghSetting::init() {
    string persistXml = getStrFromDb(DB_KEY_AM_AUDIO_CONFIG_MPEGH_PERSIST);
    if (false == persistXml.empty()) {
        ALOGI("restore persist:%s", persistXml.c_str());
    } else {
        ALOGI("no persistXml store in db");
    }
    setParameters(ACTION_PERSIST_KEY + persistXml);

    for (int32_t i = 0; i < SYSTEM_MAX_CMD_ID; i++) {
        string dbkeyStr = mpeg_system_cmd[i];
        string dbValueStr = getStrFromDb(dbkeyStr);
        string params = dbkeyStr + "=" + dbValueStr;
        if (dbValueStr.empty() == false) {
            setParameters(params);
            ALOGI("restore system_config key:%s value:%s", dbkeyStr.c_str(), dbValueStr.c_str());
        }
    }
}
