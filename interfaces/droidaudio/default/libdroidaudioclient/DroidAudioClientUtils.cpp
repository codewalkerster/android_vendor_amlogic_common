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
#define LOG_TAG "DroidAudioClientUtils"
//#define LOG_NDEBUG 0

#include <string>
#include <sstream>
#include <shared_mutex>
#include <cutils/properties.h>

#include <android/binder_ibinder.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include <media/AudioSystem.h>

#include <aidl/vendor/amlogic/hardware/droidaudio/BnDroidAudio.h>
#include <aidl/vendor/amlogic/hardware/droidaudio/BnDroidAudioClient.h>
#include <aidl/vendor/amlogic/hardware/droidaudio/IDroidAudio.h>

#include "DroidAudioClientUtils.h"
#include "DroidAudioGetService.h"
#include "DroidAudioManager.h"

using aidl::vendor::amlogic::hardware::droidaudio::IDroidAudio;
using aidl::vendor::amlogic::hardware::droidaudio::IDroidAudioClient;
using aidl::vendor::amlogic::hardware::droidaudio::BnDroidAudioClient;


using namespace std;
using namespace android;

shared_mutex g_getDroidAudioServiceLock;

class DroidAudioServiceClient: public BnDroidAudioClient
{
public:
    DroidAudioServiceClient();

    virtual ::ndk::ScopedAStatus onDroidAudioEvent(int32_t event, const vector<int32_t>& data) override;
    virtual ::ndk::ScopedAStatus onMpeghAsiEvent(int32_t in_event, const std::vector<int32_t>& in_data) override;
    ::ndk::ScopedAIBinder_DeathRecipient mDeathRecipient;
};

static shared_ptr<IDroidAudio> g_pDroidAudioService = nullptr;
static shared_ptr<DroidAudioServiceClient> g_pDroidAudioServiceClient = nullptr;
static ndk::ScopedAIBinder_DeathRecipient g_DroidAudioDeathRecipient;

static void serviceDied(void* cookie) {
    unique_lock<shared_mutex> l(g_getDroidAudioServiceLock);
    AM_LOGW("IDroidAudio service dead !!! cookie:%p", cookie);
    g_pDroidAudioService = nullptr;
}

const shared_ptr<IDroidAudio> get_droidaudio_service() {
    if (g_pDroidAudioService != nullptr) {
        return g_pDroidAudioService;
    }
    const string instance = string() + IDroidAudio::descriptor + "/default";
    ndk::SpAIBinder binder(AServiceManager_waitForService(instance.c_str()));
    g_pDroidAudioService = IDroidAudio::fromBinder(binder);
    R_CHECK_POINTER_LEGAL(nullptr, g_pDroidAudioService, "get droidaudio service fail")
    AM_LOGI("get IDroidAudio service success. ^_^ (%s)", g_pDroidAudioService->isRemote() ? "remote" : "local");
    if (g_pDroidAudioServiceClient == nullptr) {
        g_pDroidAudioServiceClient = ::ndk::SharedRefBase::make<DroidAudioServiceClient>();
        g_DroidAudioDeathRecipient = ndk::ScopedAIBinder_DeathRecipient(AIBinder_DeathRecipient_new(serviceDied));
    }
    binder_status_t binder_status = AIBinder_linkToDeath(g_pDroidAudioService->asBinder().get(),
                                                    g_DroidAudioDeathRecipient.get(), 0);
    if (binder_status != STATUS_OK) {
        AM_LOGE("Failed to AIBinder_linkToDeath. binder_status:%d", binder_status);
    }
    int32_t ret = 0;
    g_pDroidAudioService->registerClient(g_pDroidAudioServiceClient, &ret);
    NO_R_CHECK_RET(ret, "Failed to registerClient. ret:%d", ret)
    return g_pDroidAudioService;
}

DroidAudioServiceClient::DroidAudioServiceClient() {
    AM_LOGI("");
}

::ndk::ScopedAStatus DroidAudioServiceClient::onDroidAudioEvent(
                int32_t event, const vector<int32_t>& data) {
    // TODO:
    AM_LOGI("event:%d", event);
    for (auto v : data) {
        AM_LOGI("value:%d", v);
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudioServiceClient::onMpeghAsiEvent(
                int32_t event, const vector<int32_t>& data) {
    // TODO:
    AM_LOGI("event:%d", event);
    return ::ndk::ScopedAStatus::ok();
}
