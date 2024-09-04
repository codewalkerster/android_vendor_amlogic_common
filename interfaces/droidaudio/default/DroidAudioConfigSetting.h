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

#include <thread>

#include <system/audio.h>
#include <system/audio_policy.h>
#include <media/AudioSystem.h>

using namespace std;
using namespace android;

//it is used to save the any patch information<DemuxId/AudioFormat/AudioPid/OpenStatus/StartStatus/MuteStatus>
struct DroidAudioDemux {
    DroidAudioDemux() : mAudioFormat(0), mAudioPid(0), mOpenStatus(0), mStartStatus(0), mMuteStatus(0), mVolume(0) {}
    int     mAudioFormat;
    int     mAudioPid;
    int     mOpenStatus;
    int     mStartStatus;
    int     mMuteStatus;
    int     mVolume;
};

class DroidAudioConfigSetting {

public:
    friend class DroidAudioAudioPortCallback;

    static DroidAudioConfigSetting* instance();
    int32_t init();
    int32_t reset();
    int32_t setAudioCmdParam(int32_t cmd, int32_t param1, int32_t param2, int32_t param3);
    int32_t setOutputDevices(const vector<int32_t>& devices);
    int32_t getOutputDevices(vector<int32_t>* devices);
    int32_t setCoexistSpdifOther(bool enable);
    int32_t setMusicStreamVolume(int32_t index);
    int32_t setMasterMute(bool mute);
    int32_t dump(int fd, const char **args, uint32_t numArgs);

private:
    void reloadAudio();
    void findAudioSinkFromAudioPolicy(vector<audio_port_v7>& sinks);
    int32_t findAudioDevicePort(audio_devices_t type, audio_port_v7& port);
    void sinkChangedSignalNotify();
    void handleAudioSinkUpdatedRunnable();
    int32_t updateAudioPatch();
    int32_t recreateAudioPatch();
    void reStartAdecDecoderIfPossible();
    void handleDispatchAudioRoutesChanged();
    void releaseTvTunerAudioPatch();
    void updateCoexistSpdifOther();

    DroidAudioConfigSetting();
    virtual ~DroidAudioConfigSetting();


    bool                            mInitStatus;
    map<int, DroidAudioDemux>       mDemuxs;

    bool                            mNotImptTvHardwareInputService;
    bool                            mForceManagePatch;

    audio_patch*                    mpAudioPatch;
    bool                            mExitProcThread;
    std::thread                     mProcThread;
    std::mutex                      mThreadMutex;
    std::condition_variable         mThreadCnd;

    std::mutex                      mMutex;
    std::mutex                      mDemuxMutex;
};

inline DroidAudioConfigSetting* DroidAudioConfigSetting::instance() {
    static DroidAudioConfigSetting instance;
    return &instance;
}

class DroidAudioAudioPortCallback: public AudioSystem::AudioPortCallback {
public:
    DroidAudioAudioPortCallback(DroidAudioConfigSetting* proc) {
        mDroidAudioConfigSetting = proc;
    }
private:
    virtual void onAudioPortListUpdate() override {
        AM_LOGV("...");
        onProcessDtvAudio();
    }
    virtual void onAudioPatchListUpdate() override {
        AM_LOGV("...");
        onProcessDtvAudio();
    }

    void onProcessDtvAudio() {
        if (mDroidAudioConfigSetting->mNotImptTvHardwareInputService) {
            mDroidAudioConfigSetting->handleAudioSinkUpdatedRunnable();
        } else {
            // handleDispatchAudioRoutesChanged
            mDroidAudioConfigSetting->sinkChangedSignalNotify();
        }
    }

    virtual void onServiceDied() override {
        AM_LOGW("audioserver died...");
        std::thread([this] {
            // wait for the onServiceDied function call to finish.
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            this->mDroidAudioConfigSetting->reloadAudio();
         }).detach();
    }
    DroidAudioConfigSetting* mDroidAudioConfigSetting;
};

