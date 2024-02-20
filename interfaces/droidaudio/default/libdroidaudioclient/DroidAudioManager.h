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

#include <vector>

#include <android/binder_ibinder.h>

#include <aidl/vendor/amlogic/hardware/droidaudio/BnDroidAudio.h>
#include <aidl/vendor/amlogic/hardware/droidaudio/BnDroidAudioClient.h>
#include <aidl/vendor/amlogic/hardware/droidaudio/IDroidAudio.h>
using aidl::vendor::amlogic::hardware::droidaudio::IDroidAudio;
using aidl::vendor::amlogic::hardware::droidaudio::BnDroidAudioClient;

using namespace std;

class DroidAudioManager
{
public:

    static void serviceDied(void* cookie);
    static const shared_ptr<IDroidAudio> get_droid_audio_service();
    static int32_t init();
    static int32_t setAudioCmdParam(int32_t cmd, int32_t param1, int32_t param2, int32_t param3);
    static int32_t setOutputDevices(const vector<int32_t>& devices);
    static int32_t getOutputDevices(vector<int32_t>* devices);
    static int32_t setCoexistSpdifOther(bool enable);
    static int32_t setMusicStreamVolume(int32_t index);

private:

    class DroidAudioServiceClient: public BnDroidAudioClient
    {
    public:
        DroidAudioServiceClient();

        virtual ::ndk::ScopedAStatus onDroidAudioEvent(int32_t event, const vector<int32_t>& data, int32_t* _aidl_return) override;

        ::ndk::ScopedAIBinder_DeathRecipient mDeathRecipient;
    private:
        mutex   mLock;
    };
    static shared_ptr<IDroidAudio> mDroidAudioService;
    static shared_ptr<DroidAudioServiceClient> mDroidAudioServiceClient;
    static ndk::ScopedAIBinder_DeathRecipient mDroidAudioDeathRecipient;
    static mutex gLock;
};

