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

#ifndef AMLOGIC_ERRORMONITORHAL_H
#define AMLOGIC_ERRORMONITORHAL_H
#include <utils/Mutex.h>
#include <vendor/amlogic/hardware/errormonitor/1.0/IErrorMonitor.h>
#include <vendor/amlogic/hardware/errormonitor/1.0/types.h>
#include "ErrorMonitorService.h"
namespace vendor {
namespace amlogic {
namespace hardware {
namespace errormonitor {
namespace V1_0 {
namespace implementation {
using ::android::ErrorMonitorNotify;
using ::android::ErrorMonitorService;
using ::android::sp;
using ::android::hardware::hidl_string;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::vendor::amlogic::hardware::errormonitor::V1_0::IErrorMonitor;
using ::vendor::amlogic::hardware::errormonitor::V1_0::IErrorMonitorCallback;
using ::vendor::amlogic::hardware::errormonitor::V1_0::Result;

class ErrorMonitorHal : public IErrorMonitor,
                        public ErrorMonitorNotify {

public:
    ErrorMonitorHal(ErrorMonitorService* service);

    virtual ~ErrorMonitorHal();

    Return<void> setCallback(const sp<IErrorMonitorCallback>& callback) override;

    Return<Result> startErrorMonitor(const hidl_string& monitorConfig) override;

    Return<void> stopErrorMonitor() override;

    Return<void> getMonitorConfig(getMonitorConfig_cb _hidl_cb);

    Return<Result> updateMonitorConfig(const hidl_string& monitorConfig) override;

    Return<Result> setLogLevel(const hidl_string& levelConfig) override;

    Return<void> getLogLevel(getLogLevel_cb _hidl_cb);

    Return<void> notifyErrorInfo(int32_t subModule, int32_t level, int32_t logType, int32_t errorType,
                                 const hidl_string& msg);

private:
    void onReport(int32_t mainModule, int32_t subModule, int32_t level, int32_t logType, int64_t events,
                  const char* msg);
    void onError(const std::string& msg);
    void handleServiceDeath(uint32_t cookie);
    ErrorMonitorService* mErrorMonitorService;
    sp<IErrorMonitorCallback> mCallBack;
    mutable android::Mutex mLock;
    std::string mErrorMsg;
    class DeathRecipient : public android::hardware::hidl_death_recipient {
    public:
        DeathRecipient(sp<ErrorMonitorHal> sch);

        // hidl_death_recipient interface
        void serviceDied(uint64_t cookie, const ::android::wp<::android::hidl::base::V1_0::IBase>& who) override;

    private:
        sp<ErrorMonitorHal> mErrorMonitorlHal;
    };
    sp<DeathRecipient> mDeathRecipient;

}; // ErrorMonitorHal

} // namespace implementation
} // namespace V1_0
} // namespace errormonitor
} // namespace hardware
} // namespace amlogic
} // namespace vendor
#endif // AMLOGIC_ERRORMONITORHAL_H