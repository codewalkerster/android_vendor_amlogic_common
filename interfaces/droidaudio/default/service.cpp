/*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#define LOG_TAG "vendor.amlogic.droidaudio-service"

#include <inttypes.h>
#include <cutils/properties.h>

#include <android-base/logging.h>
#include <android/binder_ibinder_platform.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/IServiceManager.h>

#include "DroidAudio.h"
#include "DroidAudioClientUtils.h"

using aidl::vendor::amlogic::hardware::droidaudio::implementation::DroidAudio;
using namespace android;
using namespace std;

uint64_t get_systime_ms() {
    struct timespec ts;
    uint64_t sys_time;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    sys_time = (uint64_t)ts.tv_sec * 1000LL + (uint64_t)ts.tv_nsec / (1000*1000LL);
    return sys_time;
}

void waitBootVideoExit() {
    const int32_t TIME_OUT_CNT = 600;
    int32_t retry = 0;
    AM_LOGI("start waiting for bootvideo exit...");
    while (property_get_int32("service.bootvideo.exit", 0) == 1) {
        if (retry >= TIME_OUT_CNT) {
            AM_LOGW("timeout, waited for %d ms for bootvideo exit.", retry * 100);
            return;
        }
        retry++;
        usleep(100 * 1000);
    }
    AM_LOGI("success. waited for %d ms for bootvideo exit.", retry * 100);
}

void waitAudioService(const char *server_name) {
    uint64_t waitStartTime = get_systime_ms();
    uint64_t totalWaited = 0;
    int retry = 0;

    if (server_name == NULL) {
        AM_LOGE("server name is NULL");
        return;
    }
    sp<IServiceManager> sm = defaultServiceManager();
    while (sm->checkService(String16(server_name)) == nullptr) {
        retry++;
        usleep(100 * 1000);
    }
    totalWaited = get_systime_ms() - waitStartTime;
    AM_LOGI("success, waited for %" PRId64 " ms for %s", totalWaited, server_name);
}


int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(16);
    ABinderProcess_startThreadPool();

    std::shared_ptr<DroidAudio> droidAudioService = ::ndk::SharedRefBase::make<DroidAudio>();
    AM_LOGI("start waiting for audioserver...");
    waitAudioService("media.audio_flinger");
    waitAudioService("media.audio_policy");
    waitBootVideoExit();
    droidAudioService->init();
    const std::string Instance = std::string() + DroidAudio::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(droidAudioService->asBinder().get(), Instance.c_str());
    CHECK(status == STATUS_OK) << "Failed to add DroidAudio Factory, status=" << status;

    ABinderProcess_joinThreadPool();
}
