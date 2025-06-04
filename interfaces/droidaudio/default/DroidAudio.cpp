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

#define LOG_TAG "DroidAudio"
//#define LOG_NDEBUG 0

#include "unistd.h"
#include <thread>

#include <aidlcommonsupport/NativeHandle.h>
#include <utils/Log.h>
#include <dlfcn.h>
#include <system/audio.h>
#include <binder/IPCThreadState.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

//#include <media/AudioAttributes.h>
#include <media/AudioSystem.h>

#include "DroidAudio.h"
#include "DroidAudioDb.h"
#include "DroidAudioClientUtils.h"
#include "DroidAudioManagerSetting.h"
#include "DroidAudioEffectSetting.h"

using namespace std;
using namespace android;
using aidl::vendor::amlogic::hardware::droidaudio::IDroidAudioClient;

namespace aidl::vendor::amlogic::hardware::droidaudio::implementation {

DroidAudio* DroidAudio::mDroidAudio;

class DroidAudioAudioPortCallback: public AudioSystem::AudioPortCallback {
public:
    DroidAudioAudioPortCallback(DroidAudio* proc) {
        mDroidAudio = proc;
    }
private:
    virtual void onAudioPortListUpdate() override {
        AM_LOGV("...");
        mDroidAudio->audioPortOrPatchUpdate();
    }
    virtual void onAudioPatchListUpdate() override {
        AM_LOGV("...");
        mDroidAudio->audioPortOrPatchUpdate();
    }
    virtual void onServiceDied() override {
        AM_LOGW("audioserver died...");
        std::thread([this] {
            // wait for the onServiceDied function call to finish.
            this_thread::sleep_for(std::chrono::milliseconds(200));
            this->mDroidAudio->audioserverDied();
         }).detach();
    }
    DroidAudio* mDroidAudio;
};

DroidAudio::DroidAudio()
{
    AM_LOGI("");
    mDroidAudio = this;
    DroidAudioManagerSetting::instance();
    DroidAudioEffectSetting::instance();
    DroidAudioDb::instance()->init();
    AM_LOGI("");
}

DroidAudio::~DroidAudio() {
    AM_LOGI("");
}

DroidAudio::NotificationClient::NotificationClient(const shared_ptr<IDroidAudioClient>& client, uid_t uid, pid_t pid)
    : mUid(uid), mPid(pid), mDroidAudioClient(client) {
    mClientDeathRecipient = ndk::ScopedAIBinder_DeathRecipient(AIBinder_DeathRecipient_new(DroidAudio::clientDied));
    auto unlinkCallback = [](void* cookie) {
        if (cookie == nullptr) {
            AM_LOGW("NotificationClient: unlinked from death recipient, cookie is null.");
            return;
        }
        DroidAudio::NotificationClient* notify = (DroidAudio::NotificationClient *)cookie;
        AM_LOGW("NotificationClient: client:%p pid:%d, uid:%d unlinked from death recipient", notify, notify->uid(), notify->pid());
    };
    AIBinder_DeathRecipient_setOnUnlinked(mClientDeathRecipient.get(), unlinkCallback);

    binder_status_t binder_status = AIBinder_linkToDeath(client->asBinder().get(), mClientDeathRecipient.get(), this);
    if (binder_status != STATUS_OK) {
        AM_LOGE("Failed to AIBinder_linkToDeath. binder_status:%d", binder_status);
    }
}

DroidAudio::NotificationClient::~NotificationClient() {
    if (mIsClientDied) {
        return;
    }
    AIBinder_unlinkToDeath(mDroidAudioClient->asBinder().get(), mClientDeathRecipient.get(), this);
}

int32_t DroidAudio::NotificationClient::onDroidAudioEvent(int32_t event, const vector<int32_t>& data) {
    if (mDroidAudioClient != nullptr) {
        AM_LOGV("client(pid:%d)", mPid);
        auto ret = mDroidAudioClient->onDroidAudioEvent(event, data);
        if (!ret.isOk()) {
            AM_LOGE("client(pid:%d) error", mPid);
            return -1;
        }
    } else {
        AM_LOGE("client(pid:%d) is null", mPid);
    }
    return 0;
}

int32_t DroidAudio::doOnDroidAudioEvent(int32_t event, const vector<int32_t>& data) {
    unique_lock<mutex> _l(mNotificationClientsLock);
    for (auto it = mNotificationClients.begin(); it != mNotificationClients.end();) {
        int64_t pid = it->first & 0xffff;
        if (it->second != nullptr) {
            int32_t ret = it->second->onDroidAudioEvent(event, data);
            if (ret != 0) {
                it = mNotificationClients.erase(it);
            }
            ++it;
        } else {
            ++it;
            AM_LOGW("NotificationClient on client(pid:%" PRId64 ") is null", pid);
        }
    }
    return 0;
}

void DroidAudio::clientDied(void* cookie) {
    DroidAudio::NotificationClient* notify = (DroidAudio::NotificationClient *)cookie;
    AM_LOGW("client:%p pid:%d, uid:%d death...", notify, notify->pid(), notify->uid());
    notify->setClientDied();
    DroidAudio::getDroidAudio()->unregisterClient(notify->uid(), notify->pid());
}
void DroidAudio::audioserverDied() {
    DroidAudioManagerSetting::instance()->reloadAudio();
    DroidAudioEffectSetting::instance()->reloadAudio();
}

void DroidAudio::audioPortOrPatchUpdate() {
    DroidAudioManagerSetting::instance()->audioPortOrPatchUpdate();
}

binder_status_t DroidAudio::dump(int fd, const char **args, uint32_t numArgs) {
    dprintf(fd, "------------------------------------------ DroidAudio ----------------------------------------\n");
    vector<int32_t> data;
    data.push_back(88);
    doOnDroidAudioEvent(55, data);
    {
        unique_lock<mutex> _l(mNotificationClientsLock);
        dprintf(fd, "client list:\n");
        for (auto client : mNotificationClients) {
            dprintf(fd, "client: %d, uid: %d\n", client.second->pid(), client.second->uid());
        }
        DroidAudioManagerSetting::instance()->dump(fd, args, numArgs);
    }
    DroidAudioEffectSetting::instance()->dump(fd, args, numArgs);
    return STATUS_OK;
}

void DroidAudio::init() {
    if (mbInitStatus) {
        AM_LOGW("It's already initialized");
        return;
    }

    sp<DroidAudioAudioPortCallback> audioPortCallback = new DroidAudioAudioPortCallback(this);
    if (AudioSystem::addAudioPortCallback(audioPortCallback) != NO_ERROR) {
        AM_LOGW("addAudioPortCallback failed");
    }
    DroidAudioManagerSetting::instance()->init();
    DroidAudioEffectSetting::instance()->init();
    mbInitStatus = true;
}

::ndk::ScopedAStatus DroidAudio::registerClient(const shared_ptr<IDroidAudioClient>& client, int32_t* _aidl_return) {

    uid_t uid = AIBinder_getCallingUid();
    pid_t pid = AIBinder_getCallingPid();
    if (client == nullptr) {
        AM_LOGW("pid:%d, uid:%d, client is null ", pid, uid);
        return ::ndk::ScopedAStatus::ok();
    }
    unique_lock<mutex> _l(mNotificationClientsLock);
    int64_t token = ((int64_t)uid << 32) | pid;
    if (mNotificationClients.find(token) == mNotificationClients.end()) {
        shared_ptr<NotificationClient> notificationClient = make_shared<NotificationClient>(client, uid, pid);
        mNotificationClients.insert(pair(token, notificationClient));
        AM_LOGI("client:%p, uid:%d pid:%d, client cout:%zu", notificationClient.get(), uid, pid, mNotificationClients.size());
    } else {
        AM_LOGW("uid:%d pid:%d, client already existed, client cout:%zu", uid, pid, mNotificationClients.size());
    }
    *_aidl_return = 0;
    return ::ndk::ScopedAStatus::ok();
}

void DroidAudio::unregisterClient(uid_t uid, pid_t pid) {
    unique_lock<mutex> _l(mNotificationClientsLock);
    int64_t token = ((int64_t)uid << 32) | pid;
    auto iter = mNotificationClients.find(token);
    if (iter != mNotificationClients.end()) {
        AM_LOGI("delete client pid:%d, uid:%d ", pid, uid);
        mNotificationClients.erase(token);
    } else {
        AM_LOGW("not found pid:%d, uid:%d ", pid, uid);
    }
}

::ndk::ScopedAStatus DroidAudio::AudioManager_reset(int32_t* _aidl_return) {
    DroidAudioManagerSetting::instance()->reset();
    DroidAudioEffectSetting::instance()->reset();
    *_aidl_return = 0;
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setAudioCmdParam(
    int32_t cmd, int32_t param1, int32_t param2, int32_t param3, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setAudioCmdParam(cmd, param1, param2, param3);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setOutputDevices(const vector<int32_t>& devices, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setOutputDevices(devices);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getOutputDevices(vector<int32_t>* _aidl_return) {
    DroidAudioManagerSetting::instance()->getOutputDevices(_aidl_return);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setCoexistSpdifOtherEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setCoexistSpdifOtherEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_isCoexistSpdifOtherEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->isCoexistSpdifOtherEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setSoundBarModeEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setSoundBarModeEnabled(enable);
    *_aidl_return |= DroidAudioEffectSetting::instance()->setAudioEffectEnabled(DroidAudioEffect::EFFECT_ID_DAP, enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_isSoundBarModeEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->isSoundBarModeEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setSoundSpdifEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setSoundSpdifEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_isSoundSpdifEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->isSoundSpdifEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setSpeakerEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setSpeakerEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_isSpeakerEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->isSpeakerEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setOutputDeviceDelay(int32_t source, int32_t device, int32_t delayMs, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setOutputDeviceDelay(source, device, delayMs);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getOutputDeviceDelay(int32_t source, int32_t device, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getOutputDeviceDelay(source, device);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setAudioOutputAllDelay(int32_t delayMs, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setAudioOutputAllDelay(delayMs);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getAudioOutputAllDelay(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getAudioOutputAllDelay();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setTvSourceType(int32_t source, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setTvSourceType(source);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getTvSourceType(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getTvSourceType();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setAudioApplyToAll(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setAudioApplyToAll();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setDigitalAudioMode(int32_t mode, const string& formats, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setDigitalAudioMode(mode, formats);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getDigitalAudioMode(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getDigitalAudioMode();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setForceDDPEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setForceDDPEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_isForceDDPEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->isForceDDPEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setDolbyDrcMode(int32_t mode, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setDolbyDrcMode(mode);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getDolbyDrcMode(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getDolbyDrcMode();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setDolbyDrcLineLevel(int32_t level, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setDolbyDrcLineLevel(level);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getDolbyDrcLineLevel(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getDolbyDrcLineLevel();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_isDtsXEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->isDtsXEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setDtsXDrcEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setDtsXDrcEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_isDtsXDrcEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->isDtsXDrcEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setDialogEnhancerLevel(int32_t level, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setDialogEnhancerLevel(level);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getDialogEnhancerLevel(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getDialogEnhancerLevel();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setSoundDmxMode(int32_t mode, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setSoundDmxMode(mode);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getSoundDmxMode(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getSoundDmxMode();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setSoundLevelerMode(int32_t mode, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setSoundLevelerMode(mode);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getSoundLevelerMode(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getSoundLevelerMode();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setSoundLevelerAmount(int32_t value, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setSoundLevelerAmount(value);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getSoundLevelerAmount(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getSoundLevelerAmount();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setVadEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setVadEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_isVadEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->isVadEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_openTvAudio(int32_t source, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->openTvAudio(source);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getDroidAudioConfig(int32_t id, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getDroidAudioConfig(id);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setParameters(const string& keyValuePairs, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setParameters(keyValuePairs);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getParameters(const string& keys, string* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getParameters(keys);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_createAudioPatch(int32_t sourceDevice, int32_t sinkDevice, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->createAudioPatch(sourceDevice, sinkDevice);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_releaseAudioPatch(int32_t handle, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->releaseAudioPatch(handle);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::AudioManager_setAiDeEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setAiDeEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::AudioManager_isAiDeEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->isAiDeEnabled();
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::AudioManager_setAiDeGain(int32_t gain, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setAiDeGain(gain);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::AudioManager_getAiDeGain(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getAiDeGain();
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::AudioManager_setGlobalMicEnable(int32_t source, bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setGlobalMicEnable(source, enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getGlobalMicStatus(int32_t source, bool* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getGlobalMicStatus(source);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setMicSource(int32_t source, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setMicSource(source);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getMicSource(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getMicSource();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setMicMute(int32_t source, bool mute, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setMicMute(source, mute);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_isMicMute(int32_t source, bool* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->isMicMute(source);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setMicGain(int32_t source, int32_t gain, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setMicGain(source, gain);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getMicGain(int32_t source, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getMicGain(source);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setMicReverb(int32_t source, bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setMicReverb(source, enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_isEnableMicReverb(int32_t source, bool* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->isEnableMicReverb(source);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_setMicReverbLevel(int32_t source, int32_t level, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->setMicReverbLevel(source, level);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioManager_getMicReverbLevel(int32_t source, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioManagerSetting::instance()->getMicReverbLevel(source);
    return ::ndk::ScopedAStatus::ok();
}


// AudioEffect impl
::ndk::ScopedAStatus DroidAudio::AudioEffect_setAudioEffectEnabled(int32_t effectId, bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setAudioEffectEnabled(effectId, enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_isAudioEffectEnabled(int32_t effectId, bool* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->isAudioEffectEnabled(effectId);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setParameter(int32_t effectId, const vector<uint8_t>& param, const vector<uint8_t>& value, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setParameter(effectId, param, value);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getParameter(int32_t effectId, const vector<uint8_t>& param, vector<uint8_t>* _aidl_return) {
    DroidAudioEffectSetting::instance()->getParameter(effectId, param, _aidl_return);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setBasicEffectEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setBasicEffectEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_isBasicEffectEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->isBasicEffectEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_initDualEffectMode(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->initDualEffectMode();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setDualEffectMode(int32_t mode, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setDualEffectMode(mode);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getDualEffectMode(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->getDualEffectMode();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getEffectFunctionConfig(int32_t id, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->getEffectFunctionConfig(id);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setBalance(int32_t step, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setBalance(step);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getBalance(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->getBalance();
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::AudioEffect_setTreble(int32_t step, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setTreble(step);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getTreble(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->getTreble();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setBass(int32_t step, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setBass(step);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getBass(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->getBass();
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::AudioEffect_setDapParam(int32_t id, int32_t value, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setDapParam(id, value);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getDapParam(int32_t id, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->getDapParam(id);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::AudioEffect_setDpeParam(int32_t id, int32_t value, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setDpeParam(id, value);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getDpeParam(int32_t id, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->getDpeParam(id);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::AudioEffect_setSoundMode(int32_t mode, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setSoundMode(mode);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getSoundMode(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->getSoundMode();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setUserSoundModeParam(int32_t bandNumber, int32_t value, int32_t bandSum, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setUserSoundModeParam(bandNumber, value, bandSum);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getUserSoundModeParam(int32_t bandNumber, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->getUserSoundModeParam(bandNumber);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setHpeqBandNum(int32_t num,int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setHpeqBandNum(num);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getHpeqBandNum(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->getHpeqBandNum();
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::AudioEffect_setVirtualSurroundEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setVirtualSurroundEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_isVirtualSurroundEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->isVirtualSurroundEnabled();
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::AudioEffect_isDtsVXValidEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->isDtsVXValidEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setDtsVirtualXEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setDtsVirtualXEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_isDtsVirtualXEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->isDtsVirtualXEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setDtsVirtualSurroundEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setDtsVirtualSurroundEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_isDtsVirtualSurroundEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->isDtsVirtualSurroundEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setDtsBassEnhancementEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setDtsBassEnhancementEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_isDtsBassEnhancementEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->isDtsBassEnhancementEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setDtsDialogClarityMode(int32_t mode, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setDtsDialogClarityMode(mode);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getDtsDialogClarityMode(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->getDtsDialogClarityMode();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setDtsVirtualXMode(int32_t mode, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setDtsVirtualXMode(mode);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_getDtsVirtualXMode(int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->getDtsVirtualXMode();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setDtsTruVolumeHdEnabled(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setDtsTruVolumeHdEnabled(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_isDtsTruVolumeHdEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->isDtsTruVolumeHdEnabled();
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_setAISoundModeEnable(bool enable, int32_t* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->setAISoundModeEnable(enable);
    return ::ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus DroidAudio::AudioEffect_isAISoundModeEnabled(bool* _aidl_return) {
    *_aidl_return = DroidAudioEffectSetting::instance()->isAISoundModeEnabled();
    return ::ndk::ScopedAStatus::ok();
}


}  // namespace vendor::amlogic::hardware::droidaudio::implementation
