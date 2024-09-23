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

#ifndef AMLOGIC_ERRORMONITOR_EVENTMAPUTIL_H
#define AMLOGIC_ERRORMONITOR_EVENTMAPUTIL_H
#include <map>
#include "Collector.h"
#include "Common.h"
namespace android {
enum SYSTEMCONTROL_EVENT_TYPE {
    AML_SYS_TYPE_HDMI_SETTING_ABNORMAL = 0,
    AML_SYS_TYPE_HDMI_SHOW_ABNORMAL = 1,
    AML_SYS_TYPE_HDMI_CEC_ABNORMAL = 2,
    AML_SYS_TYPE_HDMI_HDCP_ABNORMAL = 3,
};

void initEventMap(std::map<uint32_t, std::shared_ptr<ErrorEvent>>& map);

}; // namespace android

#endif