/*
 * Copyright (C) 2024 The Android Open Source Project
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
#define LOG_TAG "DroidAudioCommon"
#define LOG_NDEBUG 0

#include <string>
#include <sstream>
#include <cutils/properties.h>

#include <media/AudioSystem.h>

#include "DroidAudioClientUtils.h"
#include "DroidAudioCommon.h"
#include "DroidAudioDb.h"

using namespace android;

const char* INI_KEY_AM_AUDIO_COMMON_DRIVER_BASE_PROJECT                         = "ini_key_am_audio_common_driver_base_project";

static int32_t g_isDriverBaseProject = -1;

void setParameters(const string &value) {
    AudioSystem::setParameters(String8(value.c_str()));
}

void setParameters(const string &key, int32_t value) {
    ostringstream paramStream;
    paramStream << key << value;
    setParameters(paramStream.str());
}

string getParameters(const string &key) {
    string str(AudioSystem::getParameters(String8(key.c_str())));
    string_view value = str;
    auto pos = value.find("=");
    if (pos != string_view::npos) {
        return string(str.substr(pos + 1));
    } else {
        AM_LOGW("param:%s, not found = in return:%s",
            key.c_str(), str.c_str());
        return str;
    }
}

bool getPropertyBoolean(const char *key, bool def) {
    int len;
    char buf[100] = {0};
    bool result = def;
    len = property_get(key, buf, "");
    if (len == 1) {
        char ch = buf[0];
        if (ch == '0' || ch == 'n')
            result = false;
        else if (ch == '1' || ch == 'y')
            result = true;
    } else if (len > 1) {
         if (!strcmp(buf, "no") || !strcmp(buf, "false") || !strcmp(buf, "off")) {
            result = false;
        } else if (!strcmp(buf, "yes") || !strcmp(buf, "true") || !strcmp(buf, "on")) {
            result = true;
        }
    }
    return result;
}

bool isAudioDebug() {
    return getPropertyBoolean("vendor.media.droidaudio.debug", false);
}

bool isTvPlatform() {
    return getPropertyBoolean("ro.vendor.platform.has.tvuimode", false);
}

bool isSoundbarPlatform() {
    return getPropertyBoolean("ro.vendor.platform.support.soundbar", false);
}

bool isDriverBaseProject() {
    if (g_isDriverBaseProject == -1) {
        inicpp::IniManager& ini = DroidAudioDb::instance()->getIniManager();
        g_isDriverBaseProject = ini[DroidAudioDbDescriptor::DROIDLOGIC_DB_MODULE_ID_AM_AUDIO_COMMON].
                            toInt(INI_KEY_AM_AUDIO_COMMON_DRIVER_BASE_PROJECT);
    }
    return g_isDriverBaseProject == 1;
}

bool isSupportMs12() {
    return getParameters("dolby_ms12_enable") == "1";
}


