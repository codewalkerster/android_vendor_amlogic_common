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
#ifndef AMLOGIC_EERRORMONITOR_MONITOR_FILTER_H
#define AMLOGIC_EERRORMONITOR_MONITOR_FILTER_H

#include <map>
#include <memory>
#include <mutex>
#include "Filter.h"

namespace android {

int32_t findMainModule(int32_t module);

class MonitorFilter : public Filter {
public:
    MonitorFilter();

    virtual ~MonitorFilter();

    bool doFilter(std::unique_ptr<ErrorData>& data);

    bool setFilterConfig(const char* config);

    char* getFilterConfig();

private:
    std::list<std::shared_ptr<MonitorConfig>> mConfigList;
    char* mConfigString;
};

}; // namespace android

#endif // AMLOGIC_EERRORMONITOR_MONITOR_FILTER_H