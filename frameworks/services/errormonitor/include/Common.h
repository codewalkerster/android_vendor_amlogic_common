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
#ifndef AMLOGIC_EERRORMONITOR_COMMON_H
#define AMLOGIC_EERRORMONITOR_COMMON_H
#include <sstream>
#include <map>

namespace android {

enum VideoErrorEvents {
    VIDEO_ERROR_EVENT_BLACK_SCREEN = 0x1,
    VIDEO_ERROR_EVENT_LAG = 0x2,
    VIDEO_ERROR_EVENT_FLOWER_SCREEN = 0x4,
    VIDEO_ERROR_EVENT_FREEZE = 0x8,
    VIDEO_ERROR_EVENT_SIZE_ABNORMAL = 0x10,
    VIDEO_ERROR_EVENT_CHANGE_CH_SLOW = 0x20,
    VIDEO_ERROR_EVENT_ENTER_SCREENSAVER = 0x40,
};
enum AudioErrorEvents {
    AUDIO_ERROR_EVENT_NO_SOUND = 0x1,
    AUDIO_ERROR_EVENT_VOL_CONTROL_ABNORMAL = 0x2,
    AUDIO_ERROR_EVENT_AV_NONSYNC = 0x4,
    AUDIO_ERROR_EVENT_POP_SOUND = 0x8,
};
enum SubtitleErrorEvents {
    SUBTITLE_ERROR_EVENT_FREEZE = 0x1,
    SUBTITLE_ERROR_EVENT_SHOW_ABNORMAL = 0x2,
};
enum DisplayErrorEvents {
    DISPLAY_ERROR_EVENT_DISP_NO_OUTPUT = 0x1,
    DISPLAY_ERROR_EVENT_DISP_SHOW_ABNORMAL = 0x2,
    DISPLAY_ERROR_EVENT_DISP_BLACKOUT = 0x4,
    DISPLAY_ERROR_EVENT_HDMI_SETTING_ABNORMAL = 0x8,
    DISPLAY_ERROR_EVENT_HDMI_SHOW_ABNORMAL = 0x10,
    DISPLAY_ERROR_EVENT_HDMI_CEC_ABNORMAL = 0x20,
    DISPLAY_ERROR_EVENT_HDCP_ABNORMAL = 0x40,
    DISPLAY_ERROR_EVENT_HDMI_ABNORMAL = 0x80,
    DISPLAY_ERROR_EVENT_CVBS_OUTPUT_ABNORMAL = 0x100,
};

enum DeviceErrorEvents {
    DEVICE_ERROR_EVENT_WIFI_LIST_DISAPPEAR = 0x1,
    DEVICE_ERROR_EVENT_WIFI_HOTPOT_CONNECT_FAIL = 0x2,
    DEVICE_ERROR_EVENT_BT_MATCH_FAIL = 0x4,
    DEVICE_ERROR_EVENT_BT_LOOPBACK_CONNECT_FAIL = 0x8,
};

enum ErrorMonitorModule {
    EERORMONITOR_MODULE_NAME_VIDEO = 0,
    EERORMONITOR_MODULE_NAME_AUDIO = 1,
    EERORMONITOR_MODULE_NAME_DISPLAY = 2,
    EERORMONITOR_MODULE_NAME_ENCODE = 3,
    EERORMONITOR_MODULE_NAME_DVB = 4,
    EERORMONITOR_MODULE_NAME_SYSTEM = 5,
    EERORMONITOR_MODULE_NAME_SUBTITLE = 6,
    EERORMONITOR_MODULE_NAME_DEVICE = 7,
    EERORMONITOR_MODULE_NAME_HDMI = 8,
    EERORMONITOR_MODULE_NAME_UNKNOWN = 9,
};

enum ErrorMonitorSubModule {
    EERORMONITOR_SUBMODULE_OMX = 0,
    EERORMONITOR_SUBMODULE_CODE2 = 1,
    EERORMONITOR_SUBMODULE_DRMPALYER = 2,
    EERORMONITOR_SUBMODULE_DRMSERVICES = 3,
    EERORMONITOR_SUBMODULE_CAS = 4,
    EERORMONITOR_SUBMODULE_VEDC = 5,
    EERORMONITOR_SUBMODULE_KERNEL = 6,
    EERORMONITOR_SUBMODULE_ANDROID = 7,
    EERORMONITOR_SUBMODULE_SYSTEMCONTROL = 8,
    EERORMONITOR_SUBMODULE_AUDIOHAL = 9,
    EERORMONITOR_SUBMODULE_HWC = 10,
    EERORMONITOR_SUBMODULE_HDMITX = 11,
    EERORMONITOR_SUBMODULE_CVBS = 12,
    EERORMONITOR_SUBMODULE_MEDIAHAL = 13,
    EERORMONITOR_SUBMODULE_WIFI = 14,
    EERORMONITOR_SUBMODULE_NUPLAYER = 15,
    EERORMONITOR_SUBMODULE_UNKNOWN = 16
};

enum ErrorMonitorErrorLevel {
    EERORMONITOR_ERROR_LEVEL_SERIOUS = 0,
    EERORMONITOR_ERROR_LEVEL_NORMAL = 1,
    EERORMONITOR_ERROR_LEVEL_SLIGHT = 2,
};

enum ErrorMonitorLogType {
    EERORMONITOR_LOG_TYPE_LOGCAT = 0x1,
    EERORMONITOR_LOG_TYPE_BUGREPORT = 0x2,
};

enum {
    AML_SYS_LOGLEVEL_FATAL = 0,
    AML_SYS_LOGLEVEL_ERROR = 1,
    AML_SYS_LOGLEVEL_WARN = 2,
    AML_SYS_LOGLEVEL_INFO = 3,
    AML_SYS_LOGLEVEL_DEBUG_0 = 4,
    AML_SYS_LOGLEVEL_DEBUG_1 = 5,
    AML_SYS_LOGLEVEL_DEBUG_2 = 6,
    AML_SYS_LOGLEVEL_VERBOSE_0 = 7,
    AML_SYS_LOGLEVEL_VERBOSE_1 = 8,
    AML_SYS_LOGLEVEL_VERBOSE_2 = 9,
    AML_SYS_LOGLEVEL_TRACE = 10
};


static const char* MainModuleString[] = {
    "video",
    "audio",
    "display",
    "encode",
    "dvb",
    "system",
    "subtitle",
    "device",
    "hdmi",
    "unknown",
};

static const char* SubModuleString[] = {
    "omx",
    "code2",
    "drmplayer",
    "drmservices",
    "cas",
    "vdec",
    "kernel",
    "android",
    "systemcontrol",
    "audiohal",
    "hwc",
    "hdmitx",
    "cvbs",
    "mediahal",
    "wifi",
    "nuplayer",
    "unknown",
};

static const char* ErrorLevelString[] = {
    "serious",
    "normal",
    "slight",
};

static const char* LogTypeString[] = {
    "logcat",
    "normal",
    "slight",
};

static std::map<int32_t, std::string> videoErrorEventMap = {
    {VIDEO_ERROR_EVENT_BLACK_SCREEN, "blackScreen_"},
    {VIDEO_ERROR_EVENT_LAG, "videoLag_"},
    {VIDEO_ERROR_EVENT_FLOWER_SCREEN, "blackScreen_"},
    {VIDEO_ERROR_EVENT_FREEZE, "freeze_"},
    {VIDEO_ERROR_EVENT_SIZE_ABNORMAL, "blackScreen_"},
    {VIDEO_ERROR_EVENT_BLACK_SCREEN, "videoSizeAbnormal_"},
    {VIDEO_ERROR_EVENT_CHANGE_CH_SLOW, "changeChannelSlow_"},
    {VIDEO_ERROR_EVENT_ENTER_SCREENSAVER, "ScreenSaver_"},
};

static std::map<int32_t, std::string> audioErrorEventMap = {
    {AUDIO_ERROR_EVENT_NO_SOUND, "noSound_"},
    {AUDIO_ERROR_EVENT_VOL_CONTROL_ABNORMAL, "volControlAbnormal_"},
    {AUDIO_ERROR_EVENT_AV_NONSYNC, "nonSync_"},
    {AUDIO_ERROR_EVENT_POP_SOUND, "popSound_"},
};

static std::map<int32_t, std::string> SubtitleErrorEventMap = {
    {SUBTITLE_ERROR_EVENT_FREEZE, "subFreeze_"},
    {SUBTITLE_ERROR_EVENT_SHOW_ABNORMAL, "showAbnormal_"},
};

static std::map<int32_t, std::string> displayErrorEventMap = {
    {DISPLAY_ERROR_EVENT_DISP_NO_OUTPUT, "DisplayNoOutput_"},
    {DISPLAY_ERROR_EVENT_HDMI_SETTING_ABNORMAL, "HdmiSettingAbnormal"},
    {DISPLAY_ERROR_EVENT_HDMI_SHOW_ABNORMAL, "HdmiShowAbnormal"},
    {DISPLAY_ERROR_EVENT_HDMI_CEC_ABNORMAL, "HdmiCecAbnormal"},
    {DISPLAY_ERROR_EVENT_HDCP_ABNORMAL, "HdcpAbnormal"},
    {DISPLAY_ERROR_EVENT_HDMI_ABNORMAL, "HdmiAbnormal"},
    {DISPLAY_ERROR_EVENT_CVBS_OUTPUT_ABNORMAL, "CvbsOutputAbnormal"},
};

static std::map<int32_t, std::string> deviceErrorEventMap = {
    {DEVICE_ERROR_EVENT_WIFI_LIST_DISAPPEAR, "WifiListDisappear"},
    {DEVICE_ERROR_EVENT_WIFI_HOTPOT_CONNECT_FAIL, "WifiHotpotConnectFail"},
    {DEVICE_ERROR_EVENT_BT_LOOPBACK_CONNECT_FAIL, "BtLoopBackConnectFail"},
};

struct MonitorConfig {
public:
    MonitorConfig()
            : module(EERORMONITOR_MODULE_NAME_UNKNOWN),
              level(EERORMONITOR_ERROR_LEVEL_SLIGHT) {};
    MonitorConfig(MonitorConfig&&) = default;
    ~MonitorConfig() = default;
    int32_t module;
    int32_t level;
};

class ErrorData {
public:
    ErrorData()
            : mainModule(EERORMONITOR_MODULE_NAME_UNKNOWN),
              subModule(EERORMONITOR_SUBMODULE_UNKNOWN),
              level(EERORMONITOR_ERROR_LEVEL_SLIGHT),
              logType(0),
              events(1),
              timeUs(0),
              msg(nullptr) {};
    ErrorData(int32_t _mainModule, int32_t _subModule, int32_t _level, int32_t _logType, int64_t _events,
              int64_t _timeUs, const char* _msg)
            : mainModule(_mainModule),
              subModule(_subModule),
              level(_level),
              logType(_logType),
              events(_events),
              timeUs(_timeUs) {
        if (_msg) {
            msg = (char*)malloc(strlen(_msg));
            memcpy(msg, _msg, strlen(_msg));
        }
    }
    ErrorData(ErrorData& data) {
        mainModule = data.mainModule;
        subModule = data.subModule;
        level = data.level;
        logType = data.logType;
        events = data.events;
        timeUs = data.timeUs;
        if (data.msg) {
            msg = (char*)malloc(strlen(data.msg));
            memcpy(msg, data.msg, strlen(data.msg));
        }
    }
    ErrorData(ErrorData&& data) {
        mainModule = data.mainModule;
        subModule = data.subModule;
        level = data.level;
        logType = data.logType;
        events = data.events;
        timeUs = data.timeUs;
        msg = data.msg;
        data.msg = nullptr;
    }
    bool operator==(const ErrorData& e) {
        if ((mainModule == e.mainModule) && (subModule == e.subModule) && (level == e.level) &&
            (logType == e.logType) && (events == e.events) && strcmp(msg, e.msg) == 0) {
            return true;
        }
        return false;
    }
    std::string toString() const {
        std::string info;
        info.append("mainModule:");
        info.append(MainModuleString[mainModule]);
        info.append(",subModule:");
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
        info.append(",events:");
        bool isGetEvent = false;
        if (mainModule == EERORMONITOR_MODULE_NAME_VIDEO) {
            for (auto it = videoErrorEventMap.begin(); it != videoErrorEventMap.end(); ++it) {
                if (events & it->first) {
                    isGetEvent = true;
                    info.append(it->second);
                }
            }
        } else if (mainModule == EERORMONITOR_MODULE_NAME_AUDIO) {
            for (auto it = audioErrorEventMap.begin(); it != audioErrorEventMap.end(); ++it) {
                if (events & it->first) {
                    isGetEvent = true;
                    info.append(it->second);
                }
            }
        } else if (mainModule == EERORMONITOR_MODULE_NAME_SUBTITLE) {
            for (auto it = SubtitleErrorEventMap.begin(); it != SubtitleErrorEventMap.end(); ++it) {
                if (events & it->first) {
                    isGetEvent = true;
                    info.append(it->second);
                }
            }
        } else if (mainModule == EERORMONITOR_MODULE_NAME_DISPLAY) {
            for (auto it = displayErrorEventMap.begin(); it != displayErrorEventMap.end(); ++it) {
                if (events & it->first) {
                    isGetEvent = true;
                    info.append(it->second);
                }
            }
        } else if (mainModule == EERORMONITOR_MODULE_NAME_DEVICE) {
            for (auto it = deviceErrorEventMap.begin(); it != deviceErrorEventMap.end(); ++it) {
                if (events & it->first) {
                    isGetEvent = true;
                    info.append(it->second);
                }
            }
        }
        if (isGetEvent) {
            info.pop_back();
        } else {
            info.append("unknown");
        }
        info.append(",timeUs:");
        info.append(std::to_string(timeUs));
        info.append(",msg:");
        info.append(msg);
        return info;
    }
    ~ErrorData() {
        if (msg)
            free(msg);
    };
    int32_t mainModule;
    int32_t subModule;
    int32_t level;
    int32_t logType;
    int64_t events;
    int64_t timeUs;
    char* msg;
};

class ErrorDataCallback {
public:
    ErrorDataCallback() = default;
    virtual ~ErrorDataCallback() = default;
    virtual void onDataAvailable(std::unique_ptr<ErrorData>& data) = 0;
    virtual void onError(const std::string& msg) = 0;
};

inline int64_t getNowTimesUs() {
    struct timespec now;
    clock_gettime(CLOCK_BOOTTIME, &now);
    int64_t now_time = (int64_t)now.tv_sec * 1000 * 1000 + (int64_t)now.tv_nsec / 1000;
    return now_time;
}
}; // namespace android

#endif
