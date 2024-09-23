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
#define LOG_TAG "ErrorMonitorEventMapUtil"
#include <utils/Log.h>
#include "AmlMediaErrorCodes.h"
#include "EventMapUtil.h"

namespace android {

void initEventMap(std::map<uint32_t, std::shared_ptr<ErrorEvent>>& map) {
    /* vdec */
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_VEDC << 16) | MEDIA_ERRORCODES_VDEC_F_NO_MEMORY,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_VIDEO, VIDEO_ERROR_EVENT_BLACK_SCREEN)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_VEDC << 16) | MEDIA_ERRORCODES_VDEC_E_TIMEOUT,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_VIDEO,
                                     VIDEO_ERROR_EVENT_BLACK_SCREEN | VIDEO_ERROR_EVENT_FLOWER_SCREEN)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_VEDC << 16) | MEDIA_ERRORCODES_VDEC_E_BITRATE_NOT_SUPPORTED,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_VIDEO, VIDEO_ERROR_EVENT_BLACK_SCREEN)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_VEDC << 16) | MEDIA_ERRORCODES_VDEC_E_PROFILELEVEL_NOT_SUPPORTED,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_VIDEO, VIDEO_ERROR_EVENT_BLACK_SCREEN)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_VEDC << 16) | MEDIA_ERRORCODES_VDEC_M_NOT_READY,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_VIDEO, VIDEO_ERROR_EVENT_BLACK_SCREEN)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_VEDC << 16) | MEDIA_ERRORCODES_VDEC_M_BAD_INPUT,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_VIDEO, VIDEO_ERROR_EVENT_BLACK_SCREEN |
                                                                         VIDEO_ERROR_EVENT_FLOWER_SCREEN |
                                                                         VIDEO_ERROR_EVENT_LAG)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_VEDC << 16) | MEDIA_ERRORCODES_VDEC_W_INPUT_UNDERRUN,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_VIDEO, VIDEO_ERROR_EVENT_FREEZE)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_VEDC << 16) | MEDIA_ERRORCODES_VDEC_W_OUTPUT_UNDERRUN,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_VIDEO, VIDEO_ERROR_EVENT_FREEZE)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_VEDC << 16) | MEDIA_ERRORCODES_VDEC_W_OUTPUT_NOT_READY,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_VIDEO,
                                     VIDEO_ERROR_EVENT_BLACK_SCREEN | VIDEO_ERROR_EVENT_FREEZE)));
    /* systemcontrol */
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_SYSTEMCONTROL << 16) | AML_SYS_TYPE_HDMI_SETTING_ABNORMAL,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_DISPLAY, DISPLAY_ERROR_EVENT_HDMI_SETTING_ABNORMAL)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_SYSTEMCONTROL << 16) | AML_SYS_TYPE_HDMI_SHOW_ABNORMAL,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_DISPLAY, DISPLAY_ERROR_EVENT_HDMI_SHOW_ABNORMAL)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_SYSTEMCONTROL << 16) | AML_SYS_TYPE_HDMI_CEC_ABNORMAL,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_DISPLAY, DISPLAY_ERROR_EVENT_HDMI_CEC_ABNORMAL)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_SYSTEMCONTROL << 16) | AML_SYS_TYPE_HDMI_HDCP_ABNORMAL,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_DISPLAY, DISPLAY_ERROR_EVENT_HDCP_ABNORMAL)));

    /* hdmitx */
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_HDMITX << 16) | 0x4,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_DISPLAY, DISPLAY_ERROR_EVENT_DISP_BLACKOUT)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_HDMITX << 16) | 0x6,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_DISPLAY, DISPLAY_ERROR_EVENT_HDMI_SHOW_ABNORMAL)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_HDMITX << 16) | 0x8,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_DISPLAY, DISPLAY_ERROR_EVENT_DISP_NO_OUTPUT)));
    /* hwc */
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_HWC << 16),
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_DISPLAY, DISPLAY_ERROR_EVENT_DISP_BLACKOUT)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_HWC << 16) | 0x4,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_DISPLAY, DISPLAY_ERROR_EVENT_DISP_BLACKOUT)));

    /* nuplayer */
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_NUPLAYER << 16),
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_AUDIO, AUDIO_ERROR_EVENT_NO_SOUND)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_NUPLAYER << 16| 0x2),
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_AUDIO, AUDIO_ERROR_EVENT_AV_NONSYNC)));

    /* mediahal */
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_MEDIAHAL << 16),
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_AUDIO, AUDIO_ERROR_EVENT_NO_SOUND)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_MEDIAHAL << 16) | 0x1,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_VIDEO, VIDEO_ERROR_EVENT_FREEZE)));
    map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_MEDIAHAL << 16) | 0x2,
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_VIDEO, VIDEO_ERROR_EVENT_BLACK_SCREEN)));
    /* device */
     map.insert(std::pair<int32_t, std::shared_ptr<ErrorEvent>>(
        (EERORMONITOR_SUBMODULE_WIFI << 16),
        std::make_shared<ErrorEvent>(EERORMONITOR_MODULE_NAME_DEVICE, DEVICE_ERROR_EVENT_WIFI_LIST_DISAPPEAR)));



}

}; // namespace android
