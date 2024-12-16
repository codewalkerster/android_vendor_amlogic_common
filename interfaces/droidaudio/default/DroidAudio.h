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

namespace aidl::vendor::amlogic::hardware::droidaudio::implementation {

using aidl::vendor::amlogic::hardware::droidaudio::Status;
using aidl::vendor::amlogic::hardware::droidaudio::IDroidAudioClient;

using namespace std;


struct DroidAudio : public BnDroidAudio {
    DroidAudio();
    ~DroidAudio();

    ::ndk::ScopedAStatus init(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus reset(int32_t* _aidl_return) override;
    void doOnDroidAudioEvent();
    ::ndk::ScopedAStatus registerClient(const shared_ptr<IDroidAudioClient>& client, int32_t* _aidl_return) override;
    void removeNotificationClient(uid_t uid, pid_t pid);
    ::ndk::ScopedAStatus setAudioCmdParam(int32_t cmd, int32_t param1,
                            int32_t param2, int32_t param3, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus setOutputDevices(const vector<int32_t>& devices, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus getOutputDevices(vector<int32_t>* devices) override;
    ::ndk::ScopedAStatus setCoexistSpdifOther(bool enable, int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus setMusicStreamVolume(int32_t index, int32_t* _aidl_return __unused) override;
    ::ndk::ScopedAStatus setMasterMute(bool mute, int32_t* _aidl_return) override;

    binder_status_t dump(int fd, const char **args, uint32_t numArgs) override;

private:
    static void clientDied(void* cookie);
    static DroidAudio* getDroidAudio() {
        return mDroidAudio;
    }
    class NotificationClient {
    public:
        NotificationClient(const shared_ptr<IDroidAudioClient>& client, uid_t uid, pid_t pid);
        virtual ~NotificationClient();
        void onDroidAudioEvent(int32_t event, const vector<int32_t>& data);
        uid_t uid() {
            return mUid;
        }
        pid_t pid() {
            return mPid;
        }
        shared_ptr<IDroidAudioClient> client() {
            return mDroidAudioClient;
        }

    private:
        const uid_t                             mUid;
        const pid_t                             mPid;
        const shared_ptr<IDroidAudioClient>     mDroidAudioClient;
        ::ndk::ScopedAIBinder_DeathRecipient    mClientDeathRecipient;
    };
    static DroidAudio* mDroidAudio;
    mutex mNotificationClientsLock;
    map<int64_t, shared_ptr<NotificationClient>> mNotificationClients;
};
}  // namespace aidl::vendor::amlogic::hardware::droidaudio::implementation
