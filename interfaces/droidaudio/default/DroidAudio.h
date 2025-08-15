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



#pragma once

#include <map>
#include <android/binder_ibinder.h>
#include <aidl/vendor/amlogic/hardware/droidaudio/BnDroidAudio.h>
#include <aidl/vendor/amlogic/hardware/droidaudio/Status.h>
#include <aidl/vendor/amlogic/hardware/droidaudio/IDroidAudioClient.h>

namespace aidl::vendor::amlogic::hardware::droidaudio::implementation {

using aidl::vendor::amlogic::hardware::droidaudio::Status;
using aidl::vendor::amlogic::hardware::droidaudio::IDroidAudioClient;

using namespace std;


struct DroidAudio : public BnDroidAudio {
    DroidAudio();
    ~DroidAudio();
    void init();
    int32_t doOnDroidAudioEvent(int32_t event, const vector<int32_t>& data);

    ::ndk::ScopedAStatus registerClient(const shared_ptr<IDroidAudioClient>& client, int32_t* _aidl_return);
    void unregisterClient(uid_t uid, pid_t pid);
    ::ndk::ScopedAStatus AudioManager_reset(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setAudioCmdParam(int32_t cmd, int32_t param1, int32_t param2, int32_t param3, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setOutputDevices(const vector<int32_t>& devices, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getOutputDevices(vector<int32_t>* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setCoexistSpdifOtherEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_isCoexistSpdifOtherEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setSoundBarModeEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_isSoundBarModeEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setSoundSpdifEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_isSoundSpdifEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setSpeakerEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_isSpeakerEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setOutputDeviceDelay(int32_t source, int32_t device, int32_t delayMs, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getOutputDeviceDelay(int32_t source, int32_t device, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setAudioOutputAllDelay(int32_t delayMs, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getAudioOutputAllDelay(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setTvSourceType(int32_t source, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getTvSourceType(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setAudioApplyToAll(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setDigitalAudioMode(int32_t mode, const string& formats, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getDigitalAudioMode(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setForceDDPEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_isForceDDPEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setDolbyDrcMode(int32_t mode, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getDolbyDrcMode(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setDolbyDrcLineLevel(int32_t level, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getDolbyDrcLineLevel(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_isDtsXEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setDtsXDrcEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_isDtsXDrcEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setDialogEnhancerLevel(int32_t level, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getDialogEnhancerLevel(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setSoundDmxMode(int32_t mode, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getSoundDmxMode(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setSoundLevelerMode(int32_t mode, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getSoundLevelerMode(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setSoundLevelerAmount(int32_t value, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getSoundLevelerAmount(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setVadEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_isVadEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_openTvAudio(int32_t source, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getDroidAudioConfig(int32_t id, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setParameters(const string& keyValuePairs, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getParameters(const string& keys, string* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_createAudioPatch(int32_t sourceDevice, int32_t sinkDevice, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_releaseAudioPatch(int32_t handle, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setAiDeEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_isAiDeEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setAiDeGain(int32_t gain, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getAiDeGain(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setGlobalMicEnable(int32_t source, bool enable, int32_t* _aidl_return);
    ::ndk::ScopedAStatus AudioManager_getGlobalMicStatus(int32_t source, bool* _aidl_return);
    ::ndk::ScopedAStatus AudioManager_setMicSource(int32_t source, int32_t* _aidl_return);
    ::ndk::ScopedAStatus AudioManager_getMicSource(int32_t* _aidl_return);
    ::ndk::ScopedAStatus AudioManager_setMicMute(int32_t source, bool mute, int32_t* _aidl_return);
    ::ndk::ScopedAStatus AudioManager_isMicMute(int32_t source, bool* _aidl_return);
    ::ndk::ScopedAStatus AudioManager_setMicGain(int32_t source, int32_t gain, int32_t* _aidl_return);
    ::ndk::ScopedAStatus AudioManager_getMicGain(int32_t source, int32_t* _aidl_return);
    ::ndk::ScopedAStatus AudioManager_setMicReverb(int32_t source, bool enable, int32_t* _aidl_return);
    ::ndk::ScopedAStatus AudioManager_isEnableMicReverb(int32_t source, bool* _aidl_return);
    ::ndk::ScopedAStatus AudioManager_setMicReverbLevel(int32_t source, int32_t level, int32_t* _aidl_return);
    ::ndk::ScopedAStatus AudioManager_getMicReverbLevel(int32_t source, int32_t* _aidl_return);
    ::ndk::ScopedAStatus AudioManager_setVocalIsolateEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_isVocalIsolateEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_setVocalRatio(int32_t ratio, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioManager_getMicVocalRatio(int32_t* _aidl_return) override;

    ::ndk::ScopedAStatus AudioEffect_setAudioEffectEnabled(int32_t effectId, bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_isAudioEffectEnabled(int32_t effectId, bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setParameter(int32_t effectId, const vector<uint8_t>& param, const vector<uint8_t>& value, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getParameter(int32_t effectId, const vector<uint8_t>& param, vector<uint8_t>* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setBasicEffectEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_isBasicEffectEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_initDualEffectMode(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setDualEffectMode(int32_t mode, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getDualEffectMode(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getEffectFunctionConfig(int32_t id, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setBalance(int32_t step, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getBalance(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setTreble(int32_t step, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getTreble(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setBass(int32_t step, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getBass(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setDapParam(int32_t id, int32_t value, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getDapParam(int32_t id, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setDpeParam(int32_t id, int32_t value, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getDpeParam(int32_t id, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setSoundMode(int32_t mode, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getSoundMode(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setUserSoundModeParam(int32_t bandNumber, int32_t value, int32_t bandSum, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getUserSoundModeParam(int32_t bandNumber, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setHpeqBandNum(int32_t value, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getHpeqBandNum(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setVirtualSurroundEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_isVirtualSurroundEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_isDtsVXValidEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setDtsVirtualXEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_isDtsVirtualXEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setDtsVirtualSurroundEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_isDtsVirtualSurroundEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setDtsBassEnhancementEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_isDtsBassEnhancementEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setDtsDialogClarityMode(int32_t mode, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getDtsDialogClarityMode(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setDtsVirtualXMode(int32_t virtualXMode, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_getDtsVirtualXMode(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setDtsTruVolumeHdEnabled(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_isDtsTruVolumeHdEnabled(bool* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_setAISoundModeEnable(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus AudioEffect_isAISoundModeEnabled(bool* _aidl_return) override;

    binder_status_t dump(int fd, const char **args, uint32_t numArgs) override;
    void audioserverDied();
    void audioPortOrPatchUpdate();

private:
    static void clientDied(void* cookie);
    static DroidAudio* getDroidAudio() {
        return mDroidAudio;
    }
    class NotificationClient {
    public:
        NotificationClient(const shared_ptr<IDroidAudioClient>& client, uid_t uid, pid_t pid);
        virtual ~NotificationClient();
        int32_t onDroidAudioEvent(int32_t event, const vector<int32_t>& data);
        uid_t uid() {
            return mUid;
        }
        pid_t pid() {
            return mPid;
        }
        shared_ptr<IDroidAudioClient> client() {
            return mDroidAudioClient;
        }
        void setClientDied() {
            mIsClientDied = true;
        }

    private:
        const uid_t                             mUid;
        const pid_t                             mPid;
        bool                                    mIsClientDied = false;
        const shared_ptr<IDroidAudioClient>     mDroidAudioClient;
        ::ndk::ScopedAIBinder_DeathRecipient    mClientDeathRecipient;
    };
    bool                mbInitStatus = false;
    static DroidAudio*  mDroidAudio;
    mutex mNotificationClientsLock;
    map<int64_t, shared_ptr<NotificationClient>> mNotificationClients;
};
}  // namespace aidl::vendor::amlogic::hardware::droidaudio::implementation
