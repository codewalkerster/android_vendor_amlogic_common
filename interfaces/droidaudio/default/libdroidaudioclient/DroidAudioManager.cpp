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

#define LOG_TAG "DroidAudioManager.C"
//#define LOG_NDEBUG 0


#include <log/log.h>
#include <cutils/properties.h>

#include <android/binder_ibinder.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include "DroidAudioCommon.h"
#include "DroidAudioCommonType.h"
#include "DroidAudioManager.h"
#include <aidl/vendor/amlogic/hardware/droidaudio/BnDroidAudio.h>
#include <aidl/vendor/amlogic/hardware/droidaudio/IDroidAudio.h>

using aidl::vendor::amlogic::hardware::droidaudio::IDroidAudio;
using aidl::vendor::amlogic::hardware::droidaudio::IDroidAudioClient;


using namespace std;
//using namespace android;


mutex DroidAudioManager::gLock;
shared_ptr<IDroidAudio> DroidAudioManager::mDroidAudioService = nullptr;
shared_ptr<DroidAudioManager::DroidAudioServiceClient> DroidAudioManager::mDroidAudioServiceClient = nullptr;
ndk::ScopedAIBinder_DeathRecipient DroidAudioManager::mDroidAudioDeathRecipient;

const shared_ptr<IDroidAudio> DroidAudioManager::get_droid_audio_service() {
    unique_lock<mutex> l(gLock);

    if (mDroidAudioService != nullptr) {
        return mDroidAudioService;
    }
    const string instance = string() + IDroidAudio::descriptor + "/default";
    ndk::SpAIBinder binder(AServiceManager_waitForService(instance.c_str()));
    mDroidAudioService = IDroidAudio::fromBinder(binder);
    if (mDroidAudioService == nullptr) {
        AM_LOGE("get droidaudio service fail");
        return nullptr;
    }
    AM_LOGI("get IDroidAudio service success. ^_^ (%s)", mDroidAudioService->isRemote() ? "remote" : "local");
    if (mDroidAudioServiceClient == nullptr) {
        mDroidAudioServiceClient = ::ndk::SharedRefBase::make<DroidAudioServiceClient>();
        mDroidAudioDeathRecipient = ndk::ScopedAIBinder_DeathRecipient(AIBinder_DeathRecipient_new(DroidAudioManager::serviceDied));
    }
    binder_status_t binder_status = AIBinder_linkToDeath(mDroidAudioService->asBinder().get(),
                                                    mDroidAudioDeathRecipient.get(), 0);
    if (binder_status != STATUS_OK) {
        AM_LOGE("Failed to AIBinder_linkToDeath. binder_status:%d", binder_status);
    }
    int32_t ret = 0;
    mDroidAudioService->registerClient(mDroidAudioServiceClient, &ret);
    NO_R_CHECK_RET(ret, "Failed to registerClient. ret:%d", ret)
    return mDroidAudioService;
}

DroidAudioManager::DroidAudioServiceClient::DroidAudioServiceClient() {
    AM_LOGI("");
}

void DroidAudioManager::serviceDied(void* cookie) {
    unique_lock<mutex> l(gLock);
    AM_LOGW("IDroidAudio service dead !!! cookie:%p", cookie);
    mDroidAudioService = nullptr;
}

::ndk::ScopedAStatus DroidAudioManager::DroidAudioServiceClient::onDroidAudioEvent(
                int32_t event, const vector<int32_t>& /*data*/, int32_t* /*_aidl_return*/) {
    // TODO:
    AM_LOGI("event:%d", event);
    return ::ndk::ScopedAStatus::ok();
}

int32_t DroidAudioManager::init() {
    AM_LOGI("");
    return 0;
}

extern "C" {
int setAudioParams(int cmd, int param1, int param2, int param3) {
    int32_t ret = DroidAudioManager::setAudioCmdParam(cmd, param1, param2, param3);
    return ret;
}}

int32_t DroidAudioManager::setAudioCmdParam(int32_t cmd, int32_t param1, int32_t param2, int32_t param3) {
    const shared_ptr<IDroidAudio>& droidaudio = DroidAudioManager::get_droid_audio_service();
    AM_LOGD("cmd:%s(%d) param1 = %d, param2 = %d, param3 = %d",  audioCmd2Str(cmd), cmd, param1, param2, param3);
    R_CHECK_POINTER_LEGAL(-1, droidaudio,)
    int32_t ret = 0;
    droidaudio->setAudioCmdParam(cmd, param1, param2, param3, &ret);
    R_CHECK_RET(ret,)
    return ret;
}

int32_t DroidAudioManager::setOutputDevices(const vector<int32_t>& devices) {
    const shared_ptr<IDroidAudio>& droidaudio = DroidAudioManager::get_droid_audio_service();
    R_CHECK_POINTER_LEGAL(-1, droidaudio,)
    int32_t ret = 0;
    droidaudio->setOutputDevices(devices, &ret);
    R_CHECK_RET(ret,)
    return ret;
}

int32_t DroidAudioManager::getOutputDevices(vector<int32_t>* devices) {
    const shared_ptr<IDroidAudio>& droidaudio = DroidAudioManager::get_droid_audio_service();
    R_CHECK_POINTER_LEGAL(-1, droidaudio,)
    int32_t ret = 0;
    droidaudio->getOutputDevices(devices);
    R_CHECK_RET(ret,)
    return ret;
}

int32_t DroidAudioManager::setCoexistSpdifOther(bool enable) {
    const shared_ptr<IDroidAudio>& droidaudio = DroidAudioManager::get_droid_audio_service();
    R_CHECK_POINTER_LEGAL(-1, droidaudio,)
    int32_t ret = 0;
    droidaudio->setCoexistSpdifOther(enable, &ret);
    R_CHECK_RET(ret,)
    return ret;
}

int32_t DroidAudioManager::setMusicStreamVolume(int32_t index) {
    const shared_ptr<IDroidAudio>& droidaudio = DroidAudioManager::get_droid_audio_service();
    R_CHECK_POINTER_LEGAL(-1, droidaudio,)
    int32_t ret = 0;
    droidaudio->setMusicStreamVolume(index, &ret);
    R_CHECK_RET(ret,)
    return ret;
}




