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

#include <aidlcommonsupport/NativeHandle.h>
#include <utils/Log.h>
#include <dlfcn.h>
#include <system/audio.h>
#include <binder/IPCThreadState.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include "unistd.h"

//#include <media/AudioAttributes.h>
#include <media/AudioSystem.h>


#include "DroidAudio.h"
#include "DroidAudioCommon.h"
#include "DroidAudioConfigSetting.h"

using namespace std;
using namespace android;
using aidl::vendor::amlogic::hardware::droidaudio::IDroidAudioClient;

namespace aidl::vendor::amlogic::hardware::droidaudio::implementation {

DroidAudio* DroidAudio::mDroidAudio;


DroidAudio::DroidAudio()
{
    AM_LOGI("");
    mDroidAudio = this;
    DroidAudioConfigSetting::instance();
}

DroidAudio::~DroidAudio() {
    AM_LOGI("");
}

void DroidAudio::removeNotificationClient(uid_t uid, pid_t pid) {
    AM_LOGW("pid:%d, uid:%d death...", pid, uid);
    unique_lock<mutex> _l(mNotificationClientsLock);
    int64_t token = ((int64_t)uid<<32) | pid;
    auto iter = mNotificationClients.find(token);
    if (iter != mNotificationClients.end()) {
        mNotificationClients.erase(token);
    } else {
        AM_LOGW("not found pid:%d, uid:%d ", pid, uid);
    }
}

DroidAudio::NotificationClient::NotificationClient(const shared_ptr<IDroidAudioClient>& client, uid_t uid, pid_t pid)
    : mUid(uid), mPid(pid), mDroidAudioClient(client) {
    mClientDeathRecipient = ndk::ScopedAIBinder_DeathRecipient(AIBinder_DeathRecipient_new(DroidAudio::clientDied));
    binder_status_t binder_status = AIBinder_linkToDeath(client->asBinder().get(), mClientDeathRecipient.get(), this);
    if (binder_status != STATUS_OK) {
        AM_LOGE("Failed to AIBinder_linkToDeath. binder_status:%d", binder_status);
    }
}

DroidAudio::NotificationClient::~NotificationClient() {
    AIBinder_unlinkToDeath(mDroidAudioClient->asBinder().get(), mClientDeathRecipient.get(), nullptr);
}

void DroidAudio::NotificationClient::onDroidAudioEvent(int32_t event, const vector<int32_t>& data) {
    if (mDroidAudioClient != 0) {
        mDroidAudioClient->onDroidAudioEvent(event, data, 0);
    }
}

void DroidAudio::doOnDroidAudioEvent() {
    unique_lock<mutex> _l(mNotificationClientsLock);
    for (auto client : mNotificationClients) {
        vector<int32_t> data;
        client.second->onDroidAudioEvent(0, data);
    }
}

void DroidAudio::clientDied(void* cookie) {
    DroidAudio::NotificationClient* notify = (DroidAudio::NotificationClient *)cookie;
    AM_LOGW("client:%p death...", notify);
    DroidAudio::getDroidAudio()->removeNotificationClient(notify->uid(), notify->pid());
}

::ndk::ScopedAStatus DroidAudio::init(int32_t* _aidl_return __unused) {
    uid_t uid = IPCThreadState::self()->getCallingUid();
    pid_t pid = IPCThreadState::self()->getCallingPid();
    AM_LOGI("pid:%d, uid:%d", pid, uid);
    DroidAudioConfigSetting::instance()->init();
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::reset(int32_t* _aidl_return __unused) {
    DroidAudioConfigSetting::instance()->reset();
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::registerClient(const shared_ptr<IDroidAudioClient>& client, int32_t* _aidl_return __unused) {

    uid_t uid = IPCThreadState::self()->getCallingUid();
    pid_t pid = IPCThreadState::self()->getCallingPid();
    AM_LOGI("pid:%d, uid:%d", pid, uid);
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
        AM_LOGI("uid:%d pid:%d, client already existed, client cout:%zu", uid, pid, mNotificationClients.size());
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::setAudioCmdParam(int32_t cmd, int32_t param1,
    int32_t param2, int32_t param3, int32_t* _aidl_return __unused) {
    DroidAudioConfigSetting::instance()->setAudioCmdParam(cmd, param1, param2, param3);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus DroidAudio::setOutputDevices(const vector<int32_t>& devices, int32_t* _aidl_return __unused) {
    DroidAudioConfigSetting::instance()->setOutputDevices(devices);
    return ::ndk::ScopedAStatus::ok();

}
::ndk::ScopedAStatus DroidAudio::getOutputDevices(vector<int32_t>* devices) {
    DroidAudioConfigSetting::instance()->getOutputDevices(devices);
    return ::ndk::ScopedAStatus::ok();

}
::ndk::ScopedAStatus DroidAudio::setCoexistSpdifOther(bool enable, int32_t* _aidl_return __unused) {
    AM_LOGI("enable:%d", enable);
    DroidAudioConfigSetting::instance()->setCoexistSpdifOther(enable);
    return ::ndk::ScopedAStatus::ok();

}

::ndk::ScopedAStatus DroidAudio::setMusicStreamVolume(int32_t index, int32_t* _aidl_return __unused) {
    AM_LOGI("index:%d", index);
    DroidAudioConfigSetting::instance()->setMusicStreamVolume(index);
    return ::ndk::ScopedAStatus::ok();
}

binder_status_t DroidAudio::dump(int fd, const char **args, uint32_t numArgs) {
    {
        unique_lock<mutex> _l(mNotificationClientsLock);
        dprintf(fd, "client list:\n");
        for (auto client : mNotificationClients) {
            dprintf(fd, "client: %d, uid: %d\n", client.second->pid(), client.second->uid());
        }
        DroidAudioConfigSetting::instance()->dump(fd, args, numArgs);
    }
    return STATUS_OK;
}

}  // namespace vendor::amlogic::hardware::droidaudio::implementation
