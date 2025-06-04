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

using namespace std;

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

class DroidAudioPatchManager final {
public:
    static DroidAudioPatchManager& instance() {
        static DroidAudioPatchManager instance;
        return instance;
    }
    int32_t init();
    int32_t setAudioCmdParam(int32_t cmd, int32_t param1, int32_t param2, int32_t param3);
    int32_t createAudioPatch(int32_t sourceDevice, int32_t sinkDevice);
    int32_t releaseAudioPatch(int32_t handle);
    int32_t openTvAudio(int32_t source);

    void reloadAudio();
    void audioPortOrPatchUpdate();
    int32_t dump(int fd, const char **args, uint32_t numArgs);

private:
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

    DroidAudioPatchManager() {};

    map<int, DroidAudioDemux>               mDemuxs;

    bool                                    mNotImptTvHardwareInputService = false;
    bool                                    mInitStatus = false;
    bool                                    mForceManagePatch = false;
    int32_t                                 mCurTunerSourceType = -1;

    audio_patch*                            mpAudioPatch;
    bool                                    mExitProcThread;
    std::thread                             mProcThread;
    std::mutex                              mThreadMutex;
    std::condition_variable                 mThreadCnd;

    std::mutex                              mMutex;
    std::mutex                              mDemuxMutex;
};


