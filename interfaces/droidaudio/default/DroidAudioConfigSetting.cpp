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

#define LOG_TAG "DroidAudioConfigSetting"
//#define LOG_NDEBUG 0

#include <system/audio-base.h>
#include <cutils/properties.h>
#include "unistd.h"
#include <log/log.h>

#include <media/AidlConversion.h>
#include <media/AudioSystem.h>


#include "DroidAudioCommon.h"
#include "DroidAudioCommonType.h"
#include "DroidAudioConfigSetting.h"
#include "SystemControlClient.h"

using namespace std;
using namespace android;


static sp<SystemControlClient> g_SystemControlClient;
#define  DVB_DEMUX_ID_BASE      25

static void encapsulationAndSetParams(const char *key, int32_t value) {
    char param[100 + 11];
    if (strlen(key) >= 100) {
        AM_LOGE("key size > 100, key:%s, setParameters fail", key);
        return;
    }
    sprintf(param, "%s%d", key, value);
    AudioSystem::setParameters(String8(param));
}


bool getPropertyBoolean(const char *key, bool def) {

    int len;
    char buf[100] = {0};
    bool result = def;

    len = property_get(key, buf, "");
    if (len == 1) {
        char ch = buf[0];
        if (ch == '0' || ch == 'n')
            result = false;
        else if (ch == '1' || ch == 'y')
            result = true;
    } else if (len > 1) {
         if (!strcmp(buf, "no") || !strcmp(buf, "false") || !strcmp(buf, "off")) {
            result = false;
        } else if (!strcmp(buf, "yes") || !strcmp(buf, "true") || !strcmp(buf, "on")) {
            result = true;
        }
    }

    return result;
}

bool getDebugEnable() {
    return getPropertyBoolean("vendor.media.droidaudio.debug", false);
}

void setAdFunction(int cmd, int param1, int param2, int param3 __unused) {
    switch (cmd) {
        case DROID_AUDIO_CMD_AD_SWITCH_ENABLE:
            encapsulationAndSetParams("ad_switch_enable=", (param1 > 0 ? 1 : 0));
            break;
        case DROID_AUDIO_CMD_AD_SET_VOLUME:
            encapsulationAndSetParams("dual_decoder_advol_level=", param1);
            break;
        case DROID_AUDIO_CMD_AD_DUAL_SUPPORT:
            encapsulationAndSetParams("hal_param_dual_dec_support=", (param1 > 0 ? 1 : 0));
            break;
        case DROID_AUDIO_CMD_AD_MIX_SUPPORT://Associated audio mixing on/off
            encapsulationAndSetParams("hal_param_dual_dec_support=", param1);
            encapsulationAndSetParams("hal_param_ad_mix_enable=", param1);
            break;
        case DROID_AUDIO_CMD_AD_MIX_LEVEL://Associated audio mixing level
            encapsulationAndSetParams("hal_param_dual_dec_mix_level=", param2);
            break;
        default:
            AM_LOGW("unknown  cmd id:%d param1:%d", cmd, param1);
            break;
    }
}

status_t listAudioPatches(vector<struct audio_patch>& patches) {
    int attempts = 5;
    status_t status;
    unsigned int generation1, generation;
    unsigned int numPatches = 0;
    do {
        if (attempts-- < 0) {
            status = TIMED_OUT;
            break;
        }
        numPatches = 0;
        status = AudioSystem::listAudioPatches(&numPatches, nullptr, &generation1);
        if (status != NO_ERROR) {
            AM_LOGW("AudioSystem::listAudioPatches error: %d", status);
            break;
        }
        patches.resize(numPatches);
        status = AudioSystem::listAudioPatches(&numPatches, patches.data(), &generation);
        AM_LOGV("numPatches: %d, generation1: %d, generation: %d, attempts: %d, status: %d",
            numPatches, generation1, generation, attempts, status);
    } while (generation1 != generation && status == NO_ERROR);
    if (status != NO_ERROR) {
        numPatches = 0;
        patches.clear();
    }
    return status;
}

void listAudioPorts(vector<audio_port_v7>& ports) {
    status_t status;
    unsigned int generation1;
    unsigned int generation;
    unsigned int numPorts;
    int attempts = 5;

    // get the port count and all the ports until they both return the same generation
    do {
        if (attempts-- < 0) {
            status = TIMED_OUT;
            break;
        }
        numPorts = 0;
        status = AudioSystem::listAudioPorts(AUDIO_PORT_ROLE_NONE, AUDIO_PORT_TYPE_NONE, &numPorts, NULL, &generation1);
        if (status != NO_ERROR) {
            AM_LOGW("AudioSystem::listAudioPorts error: %d", status);
            break;
        }
        ports.resize(numPorts);
        status = AudioSystem::listAudioPorts(AUDIO_PORT_ROLE_NONE, AUDIO_PORT_TYPE_NONE, &numPorts, ports.data(), &generation);
        AM_LOGV(" numPorts %d generation %d generation1 %d", numPorts, generation, generation1);
    } while (generation1 != generation && status == NO_ERROR);
    if (status != NO_ERROR) {
        numPorts = 0;
        ports.clear();
    }
}

DroidAudioConfigSetting::DroidAudioConfigSetting(): mInitStatus(false)
{
    AM_LOGI("");
    mNotImptTvHardwareInputService = !getPropertyBoolean("ro.vendor.platform.build.livetv", false);
    mForceManagePatch = getPropertyBoolean("vendor.media.dtv.force.manage.patch", false);
    mExitProcThread = false;
    g_SystemControlClient = ::android::SystemControlClient::getInstance();

    sp<DroidAudioAudioPortCallback> audioPortCallback = new DroidAudioAudioPortCallback(this);
    if (AudioSystem::addAudioPortCallback(audioPortCallback) != NO_ERROR) {
        AM_LOGW("addAudioPortCallback failed");
    }
    mProcThread = thread(&DroidAudioConfigSetting::handleDispatchAudioRoutesChanged, this);
    reloadAudio();
}

DroidAudioConfigSetting::~DroidAudioConfigSetting() {
    mExitProcThread = true;
    sinkChangedSignalNotify();
    if (mProcThread.joinable()) {
        mProcThread.join();
    }
}

void DroidAudioConfigSetting::reloadAudio() {
    int32_t spdifCoexist = g_SystemControlClient->getPropertyInt(PROP_AUDIO_OUTPUT_SPDIF_COEXIST, true);
    setCoexistSpdifOther(spdifCoexist == 1);

    int32_t forceUse = g_SystemControlClient->getPropertyInt(PROP_AUDIO_OUTPUT_FORCEUSE, DROID_AUDIO_FORCE_USE_NONE);
    vector<int32_t> devices;
    devices.push_back(forceUse);
    setOutputDevices(devices);

}

int32_t DroidAudioConfigSetting::init() {
    if (mInitStatus) {
        AM_LOGW("It's already initialized");
        return 0;
    }
    mInitStatus = true;
    return 0;
}

int32_t DroidAudioConfigSetting::reset() {
    setCoexistSpdifOther(true);
    vector<int32_t> devices;
    devices.push_back(DROID_AUDIO_FORCE_USE_NONE); //default value;
    setOutputDevices(devices);
    return 0;
}

int32_t DroidAudioConfigSetting::dump(int fd, const char **args __unused, uint32_t numArgs __unused) {
    int32_t forceUse = g_SystemControlClient->getPropertyInt(PROP_AUDIO_OUTPUT_FORCEUSE, DROID_AUDIO_FORCE_USE_NONE);
    dprintf(fd, "db forceUse: %d\n", forceUse);
    dprintf(fd, "tif: %d\n", !mNotImptTvHardwareInputService);
    dprintf(fd, "mForceManagePatch: %d\n",
        mForceManagePatch);

    for (auto &v : mDemuxs) {
        DroidAudioDemux& demux = v.second;
        dprintf(fd, "START_DECODE(id:%d) format:%d pid:%d start:%d open:%d mute:%d vol:%d\n",
            v.first, demux.mAudioFormat, demux.mAudioPid, demux.mStartStatus, demux.mOpenStatus,
            demux.mMuteStatus, demux.mVolume);
    }
    return STATUS_OK;
}

void DroidAudioConfigSetting::sinkChangedSignalNotify() {
    unique_lock<mutex> mutex(mThreadMutex);
    AM_LOGV("audio changed and notify>>>>>>>");
    mThreadCnd.notify_one();
}

void DroidAudioConfigSetting::handleDispatchAudioRoutesChanged() {
    uint32_t timeoutMs = 500;
    bool standby = true;
    cv_status ret;
    AM_LOGI("start+++");
    while (!mExitProcThread) {
        unique_lock<mutex> mutex(mThreadMutex);
        if (standby) {
            AM_LOGV("mThreadCnd_wait begin+++++++++++++");
            mThreadCnd.wait(mutex);
            AM_LOGV("mThreadCnd_wait end--------");
        }
        ret = mThreadCnd.wait_for(mutex, chrono::milliseconds(timeoutMs));
        if (mExitProcThread) {
            break;
        }
        AM_LOGV("mThreadCnd_wait_for end--------");
        if (cv_status::timeout == ret) {
            handleAudioSinkUpdatedRunnable();
            if (getDebugEnable()) {
                AM_LOGD("timeout:%d ms process finished.", timeoutMs);
            }
            standby = true;
        } else {
            standby = false;
        }
    }
    AM_LOGI("exit---");
    return;
}

void DroidAudioConfigSetting::handleAudioSinkUpdatedRunnable() {

    int32_t ret = 0;
    unique_lock<mutex> l(mMutex);
    if (mNotImptTvHardwareInputService) {
        if (mpAudioPatch == nullptr) {
            if (getDebugEnable()) {
                AM_LOGD("not find dtv audio patch");
            }
            return;
        }
        ret = recreateAudioPatch();
    } else {
        ret = updateAudioPatch();
    }
}

void DroidAudioConfigSetting::findAudioSinkFromAudioPolicy(vector<audio_port_v7>& ports) {
    ports.clear();
    AudioDeviceTypeAddrVector curDevices{};
    audio_attributes_t attributes = AudioSystem::streamTypeToAttributes(AUDIO_STREAM_MUSIC);
    AudioSystem::getDevicesForAttributes(attributes, &curDevices, false);
    if (curDevices.size() == 0) {
        AM_LOGW("not find cur sink device");
        return;
    }
    vector<audio_port_v7> audioPorts;
    listAudioPorts(audioPorts);
    for (auto audioPort : audioPorts) {
        if (audioPort.type == AUDIO_PORT_TYPE_DEVICE && audioPort.role == AUDIO_PORT_ROLE_SINK) {
            for_each(begin(curDevices), end(curDevices), [&](auto device) {
                if ((device.mType & audioPort.ext.device.type) != 0) {
                    ports.push_back(audioPort);
                }
            });
        }
    }
    if (ports.size() == 0) {
        AM_LOGW("not find avail sink device");
        return;
    }
}

int32_t DroidAudioConfigSetting::findAudioDevicePort(audio_devices_t type, audio_port_v7& port) {
    vector<audio_port_v7> audioPorts;
    listAudioPorts(audioPorts);
    for (auto audioPort : audioPorts) {
        if (audioPort.type == AUDIO_PORT_TYPE_DEVICE) {
            if (audioPort.ext.device.type == type) {
                port = audioPort;
                return 0;
            }
        }
    }
    AM_LOGW("not find source device:%#x", type);
    port.id = -1;
    return -1;
}

int32_t DroidAudioConfigSetting::setMusicStreamVolume(int32_t index __unused) {
    // TODO: For the audio middleware projects.
#if 0
    int32_t currentIndex = 0;
    AudioSystem::getStreamVolumeIndex(AUDIO_STREAM_MUSIC, &currentIndex, AUDIO_DEVICE_OUT_DEFAULT);
    int gainValueMb = (int)(100 * AudioSystem::getStreamVolumeDB(AUDIO_STREAM_MUSIC, currentIndex, AUDIO_DEVICE_OUT_SPEAKER));
    struct audio_port_config tunerInAudioPort = mpAudioPatch->sources[0];
    tunerInAudioPort.config_mask = AUDIO_PORT_CONFIG_GAIN;
    tunerInAudioPort.gain.mode = AUDIO_GAIN_MODE_JOINT;
    tunerInAudioPort.gain.values[0] = gainValueMb;
    AM_LOGI("cur music index:%d, gainValueMb:%d", currentIndex, gainValueMb);
    status_t status = AudioSystem::setAudioPortConfig(&tunerInAudioPort);
    if (status != NO_ERROR) {
        AM_LOGE("setAudioPortConfig fail. status:%d", status);
    }

#endif
    return 0;
}

int32_t DroidAudioConfigSetting::updateAudioPatch() {
    vector<audio_patch> patchs;
    listAudioPatches(patchs);
    for (audio_patch patch : patchs) {
        if (patch.sources[0].type == AUDIO_PORT_TYPE_DEVICE &&
            patch.sources[0].ext.device.type == AUDIO_DEVICE_IN_TV_TUNER &&
            patch.sinks[0].type == AUDIO_PORT_TYPE_DEVICE) {
            if (mpAudioPatch != nullptr) {
                if (mpAudioPatch->sinks[0].ext.device.type != patch.sinks[0].ext.device.type ||
                    mpAudioPatch->sinks[0].id != patch.sinks[0].id) {
                    AM_LOGI("update audio patch, sink dev:%#x id %d", patch.sinks[0].ext.device.type,patch.sinks[0].id);
                    delete mpAudioPatch;
                    mpAudioPatch = new audio_patch(patch);
                    return 0;
                } else {
                    AM_LOGI("no sink changed, sink:type %#x id %d", mpAudioPatch->sinks[0].ext.device.type, mpAudioPatch->sinks[0].id);
                    return -1;
                }
            } else {
                mpAudioPatch = new audio_patch(patch);
                AM_LOGI("find TIF audio patch, sink dev:%#x", patch.sinks[0].ext.device.type);
                return 0;
            }
        }
    }
    AM_LOGW("not find TIF tv_tuner->dev audio patch.");
    if (mpAudioPatch != nullptr) {
        delete mpAudioPatch;
        mpAudioPatch = nullptr;
    }
    return -1;
}

int32_t DroidAudioConfigSetting::recreateAudioPatch() {
    audio_port_v7               audioSource{};
    vector<audio_port_v7>       audioSinks;
    audioSource.id = -1;

    findAudioDevicePort(AUDIO_DEVICE_IN_TV_TUNER, audioSource);
    findAudioSinkFromAudioPolicy(audioSinks);

    if (audioSource.id == -1 || audioSinks.size() == 0) {
        AM_LOGW("audioSource id:%d, sinks empty:%zu", audioSource.id, audioSinks.size());
        if (mpAudioPatch != nullptr) {
            AudioSystem::releaseAudioPatch(mpAudioPatch->id);
            delete mpAudioPatch;
            mpAudioPatch = nullptr;
            mDemuxs.clear();
        }
        return -1;
    }

    if (audioSinks.size() != 1) {
        AM_LOGW("do not support num_sinks:%zu", audioSinks.size());
        return -1;
    }

    bool found = false;
    vector<audio_patch> patchs;
    listAudioPatches(patchs);
    for (audio_patch patch : patchs) {
        if (patch.sources[0].type == AUDIO_PORT_TYPE_DEVICE &&
            patch.sources[0].ext.device.type == AUDIO_DEVICE_IN_TV_TUNER &&
            patch.sinks[0].type == AUDIO_PORT_TYPE_DEVICE) {
            if (audioSinks[0].ext.device.type == patch.sinks[0].ext.device.type) {
                if (mpAudioPatch == nullptr) {
                    AM_LOGI("update null -> new audio patch, sink:%#x", audioSinks[0].ext.device.type);
                    mpAudioPatch = new audio_patch(patch);
                } else {
                    if (mpAudioPatch->sinks[0].ext.device.type != audioSinks[0].ext.device.type) {
                        AM_LOGI("update audio patch from pre dev:%#x -> cur dev:%#x",
                            mpAudioPatch->sinks[0].ext.device.type, audioSinks[0].ext.device.type);
                        delete mpAudioPatch;
                        mpAudioPatch = new audio_patch(patch);
                    } else {
                        AM_LOGI("no sink changed, sink:%#x", audioSinks[0].ext.device.type);
                    }
                }
                return -1;
            }
            AM_LOGI("sink changed. recreate, pre:%#x -> cur:%#x",
                patch.sinks[0].ext.device.type, audioSinks[0].ext.device.type);
            found = true;
        }
    }
    if (!found) {
        AM_LOGI("create new dev->dev audio patch");
    }

    AM_LOGI("mpAudioPatch is empty:%d", (mpAudioPatch == nullptr));
    if (mpAudioPatch == nullptr) {
        mDemuxs.clear();
    } else {
        AudioSystem::releaseAudioPatch(mpAudioPatch->id);
        delete mpAudioPatch;
        mpAudioPatch = nullptr;
    }
    audio_patch *audioPatch = new audio_patch;
    audioPatch->num_sinks = 0;
    audioPatch->num_sources = 1;
    audioPatch->sources[0] = audioSource.active_config;
    for (auto audioSink : audioSinks) {
        audioPatch->sinks[audioPatch->num_sinks] = audioSink.active_config;
        audioPatch->num_sinks++;
        if (audioPatch->num_sinks >= AUDIO_PATCH_PORTS_MAX) {
            AM_LOGW("num_sinks:%d > max:%d", audioPatch->num_sinks, AUDIO_PATCH_PORTS_MAX);
            audioPatch->num_sinks = AUDIO_PATCH_PORTS_MAX - 1;
            break;
        }
    }
    AudioSystem::createAudioPatch(audioPatch, &audioPatch->id);
    mpAudioPatch = audioPatch;
    AM_LOGI("createAudioPatch end, id:%d", audioPatch->id);
    return 0;
}

void DroidAudioConfigSetting::releaseTvTunerAudioPatch() {
    if (mpAudioPatch == nullptr) {
        return;
    }
    unique_lock<mutex> demux_l(mDemuxMutex);
    vector<audio_patch> patches;
    int result = listAudioPatches(patches);
    if (result != NO_ERROR) {
        AM_LOGW("listAudioPatches fail");
        return;
    }
    // Look for a patch that matches the provided user side handle
    int pathId = 0;
    for (auto patch : patches) {
        if (patch.num_sources == 1 && patch.sources[0].type == AUDIO_PORT_TYPE_DEVICE &&
            patch.sources[0].ext.device.type == AUDIO_DEVICE_IN_TV_TUNER) {
            // Found it!
            setAudioCmdParam(DROID_AUDIO_CMD_CLOSE_DECODER, 0, 0, pathId);
            pathId++;
            result = AudioSystem::releaseAudioPatch(patch.id);
            if (result != NO_ERROR) {
                AM_LOGW("releaseAudioPatch id:%d fail", patch.id);
            }
            continue;
        }
    }
    mDemuxs.clear();
    delete mpAudioPatch;
    mpAudioPatch = nullptr;
    // If we didn't find a match, then something went awry, but it's probably not fatal...
    AM_LOGI("releaseAudioPatch finished");
 }

int32_t DroidAudioConfigSetting::setAudioCmdParam(int32_t cmd, int32_t param1, int32_t param2, int32_t param3) {
    if (getDebugEnable()) {
        AM_LOGD("cmd:%s(%d) param1:%d param2:%d param3:%d", audioCmd2Str(cmd), cmd, param1, param2, param3);
    }

    int cmdIndex = cmd;

    if (param3 != -1) {
        cmd = cmd + (param3 << DVB_DEMUX_ID_BASE);
        param1 = param1 + (param3 << DVB_DEMUX_ID_BASE);
        param2 = param2 + (param3 << DVB_DEMUX_ID_BASE);
    }

    map<int, DroidAudioDemux>::iterator iter;
    switch (cmdIndex) {
        case DROID_AUDIO_CMD_SET_SPDIF_PROTECTION_MODE:
            encapsulationAndSetParams("hal_param_dtv_spdif_protection_mode=", param1);
            break;
        case DROID_AUDIO_CMD_SET_DEMUX_INFO:
            encapsulationAndSetParams("hal_param_dtv_demux_id=", param2);
            break;
        case DROID_AUDIO_CMD_SET_SECURITY_MEM_LEVEL:
            encapsulationAndSetParams("hal_param_security_mem_level=", param1);
            break;
        case DROID_AUDIO_CMD_SET_MEDIA_SYCN_ID:
            encapsulationAndSetParams("hal_param_media_sync_id=", param1);
            break;
        case DROID_AUDIO_CMD_SET_MEDIA_FIRST_LANG:
            encapsulationAndSetParams("hal_param_dtv_media_first_lang=", param1);
            break;
        case DROID_AUDIO_CMD_SET_MEDIA_SECOND_LANG:
            encapsulationAndSetParams("hal_param_dtv_media_second_lang=", param1);
           break;
        case DROID_AUDIO_CMD_SET_HAS_VIDEO:
            encapsulationAndSetParams("hal_param_has_dtv_video=", param1);
            break;
        case DROID_AUDIO_CMD_START_DECODE:
            encapsulationAndSetParams("hal_param_dtv_audio_fmt=", param1);
            encapsulationAndSetParams("hal_param_dtv_audio_id=", param2);
            encapsulationAndSetParams("hal_param_dtv_patch_cmd=", cmd);
            break;
        case DROID_AUDIO_CMD_PAUSE_DECODE:
            encapsulationAndSetParams("hal_param_dtv_patch_cmd=", cmd);
            break;
        case DROID_AUDIO_CMD_RESUME_DECODE:
            encapsulationAndSetParams("hal_param_dtv_patch_cmd=", cmd);
            break;
        case DROID_AUDIO_CMD_STOP_DECODE:
            encapsulationAndSetParams("hal_param_dtv_patch_cmd=", cmd);
            break;
        case DROID_AUDIO_CMD_SET_DECODE_AD:
            encapsulationAndSetParams("hal_param_dtv_sub_audio_fmt=", param1);
            encapsulationAndSetParams("hal_param_dtv_sub_audio_pid=", param2);
            encapsulationAndSetParams("hal_param_dtv_patch_cmd=", cmd);
            AM_LOGD("SET_DECODE_AD sub_audio_fmt:%d, sub_audio_pid:%d", param1, param2);
            break;
        case DROID_AUDIO_CMD_SET_VOLUME:
            encapsulationAndSetParams("hal_param_dtv_audio_volume=", param1);
            AM_LOGD("CMD_SET_VOLUME, audio volume:%d", param1);
            break;
        case DROID_AUDIO_CMD_SET_MUTE:
            encapsulationAndSetParams("hal_param_tv_mute=", param1);
            AM_LOGD("CMD_SET_MUTE, audio mute:%d", param1);
            break;
        case DROID_AUDIO_CMD_SET_OUTPUT_MODE:
            encapsulationAndSetParams("hal_param_audio_output_mode=", param1); /* refer to AM_AOUT_OutputMode_t */
            break;
        case DROID_AUDIO_CMD_SET_PRE_GAIN:
            encapsulationAndSetParams("hal_param_dtv_patch_cmd=", cmd);
            break;
        case DROID_AUDIO_CMD_SET_PRE_MUTE:
            encapsulationAndSetParams("hal_param_dtv_patch_cmd=", cmd);
            break;
        case DROID_AUDIO_CMD_OPEN_DECODER:
            {
                vector<audio_patch> patchs;
                listAudioPatches(patchs);
                unique_lock<mutex> demux_l(mDemuxMutex);
                {
                    unique_lock<mutex> l(mMutex);
                    if (mNotImptTvHardwareInputService) {
                        recreateAudioPatch();
                    } else {
                        updateAudioPatch();
                    }
                }

                encapsulationAndSetParams("hal_param_dtv_audio_fmt=", param1);
                encapsulationAndSetParams("hal_param_dtv_audio_id=", param2);
                encapsulationAndSetParams("hal_param_dtv_patch_cmd=", cmd);
                if (getDebugEnable()) {
                    AM_LOGD("now start open the decoder:OPEN_DECODER_1(%d), demux count:%zu", param3, mDemuxs.size());
                    for (auto &v : mDemuxs) {
                        DroidAudioDemux& demux = v.second;
                        AM_LOGD("OPEN_DECODER(id:%d) format:%d pid:%d start:%d open:%d mute:%d vol:%d",
                            v.first, demux.mAudioFormat, demux.mAudioPid, demux.mStartStatus, demux.mOpenStatus,
                            demux.mMuteStatus, demux.mVolume);
                    }
                }
                iter = mDemuxs.find(param3);
                DroidAudioDemux newDemux;
                DroidAudioDemux* pDemux = &newDemux;
                if (iter == mDemuxs.end()) {
                    if (param3 >= 0) {
                        newDemux.mAudioFormat = param1;
                        newDemux.mAudioPid = param2;
                        newDemux.mOpenStatus = 1;
                        newDemux.mMuteStatus = 1;
                        mDemuxs.insert(pair<int, DroidAudioDemux>(param3, newDemux));
                    } else {
                        AM_LOGW("OPEN_DECODER invalid demux id:%d", param3);
                        break;
                    }
                } else {
                    pDemux = &iter->second;
                    pDemux->mAudioFormat = param1;
                    pDemux->mAudioPid = param2;
                    pDemux->mOpenStatus = 1;
                }
                if (getDebugEnable()) {
                    AM_LOGD("now end open the decoder demux id:%d demux count:%zu", param3, mDemuxs.size());
                    for (auto &v : mDemuxs) {
                        DroidAudioDemux& demux = v.second;
                        AM_LOGD("OPEN_DECODER_2(id:%d) format:%d pid:%d start:%d open:%d mute:%d vol:%d",
                            v.first, demux.mAudioFormat, demux.mAudioPid, demux.mStartStatus, demux.mOpenStatus,
                            demux.mMuteStatus, demux.mVolume);
                    }
                }
            }
            break;
        case DROID_AUDIO_CMD_CLOSE_DECODER:
            {
                encapsulationAndSetParams("hal_param_dtv_patch_cmd=", cmd);
                unique_lock<mutex> demux_l(mDemuxMutex);
                if (getDebugEnable()) {
                    AM_LOGD("now start close the decoder demux id:%d demux count:%zu", param3, mDemuxs.size());
                    for (auto &v : mDemuxs) {
                        DroidAudioDemux& demux = v.second;
                        AM_LOGD("CLOSE_DECODER_2(id:%d) format:%d pid:%d start:%d open:%d mute:%d vol:%d",
                            v.first, demux.mAudioFormat, demux.mAudioPid, demux.mStartStatus, demux.mOpenStatus,
                            demux.mMuteStatus, demux.mVolume);
                    }
                }
                iter = mDemuxs.find(param3);
                bool found = (iter != mDemuxs.end());
                unique_lock<mutex> l(mMutex);
                if (mNotImptTvHardwareInputService) {//IPTV case and create or relese audio patch be controlled by AUDIOSYSTEMSERVICE
                    if (param3 >= 0) {
                        if (found) {
                            mDemuxs.erase(iter);
                        } else {
                            AM_LOGW("CLOSE_DECODER (%d) maybe already closed", param3);
                        }
                    }
                    if (mDemuxs.size() == 0 && mpAudioPatch != nullptr) {
                        AM_LOGI("ADEC_CLOSE_DECODER releaseAudioPatch id:%d", mpAudioPatch->id);
                        AudioSystem::releaseAudioPatch(mpAudioPatch->id);
                    }
                } else {//DTVKIT case and create or relese audio patch be controlled by TIF
                    if (found) {
                        mDemuxs.erase(iter);
                    } else {
                        AM_LOGW("CLOSE_DECODER (%d) maybe already closed", param3);
                    }
                }
                if (mDemuxs.size() == 0) {
                    delete mpAudioPatch;
                    mpAudioPatch = nullptr;
                }
                if (getDebugEnable()) {
                    AM_LOGD("now end close the decoder, demux id:%d demux count:%zu", param3, mDemuxs.size());
                    for (auto &v : mDemuxs) {
                        DroidAudioDemux& demux = v.second;
                        AM_LOGD("CLOSE_DECODER_2(id:%d) format:%d pid:%d start:%d open:%d mute:%d vol:%d",
                            v.first, demux.mAudioFormat, demux.mAudioPid, demux.mStartStatus, demux.mOpenStatus,
                            demux.mMuteStatus, demux.mVolume);
                    }
                }
            }
            break;
        case DROID_AUDIO_CMD_SET_MEDIA_PRESENTATION_ID:
            encapsulationAndSetParams("hal_param_dtv_media_presentation_id=", param1);
            break;
        case DROID_AUDIO_CMD_SET_AUDIO_PATCH_MANAGE_MODE:
            {
                bool isDvbPlayback = (param1 == 0);
                int forceManagePatchMode = param2;
                if (param3 != 0) {
                    isDvbPlayback = ((param1 -(param3 << DVB_DEMUX_ID_BASE)) == 0);
                    forceManagePatchMode = (param2 -(param3 << DVB_DEMUX_ID_BASE));
                }
                bool hasTif = getPropertyBoolean("ro.vendor.platform.build.livetv", false);
                if (mForceManagePatch) {
                    mNotImptTvHardwareInputService = true;
                } else if (forceManagePatchMode == 0) {
                    // force disable
                    mNotImptTvHardwareInputService = false;
                } else if (forceManagePatchMode == 1) {
                    // force enable
                    mNotImptTvHardwareInputService = true;
                } else {
                    // auto
                    if (hasTif && isDvbPlayback) {
                        mNotImptTvHardwareInputService = false;
                    } else {
                        mNotImptTvHardwareInputService = true;
                    }
                }
                AM_LOGI("mForceManagePatch:%d, isDvbPlayback:%d, forceManagePatchMode:%d, hasTif:%d, mNotImptTvHardwareInputService set:%d",
                        mForceManagePatch, isDvbPlayback, forceManagePatchMode, hasTif, mNotImptTvHardwareInputService);
            }
            break;
        case DROID_AUDIO_CMD_AD_SWITCH_ENABLE:
        case DROID_AUDIO_CMD_AD_SET_VOLUME:
        case DROID_AUDIO_CMD_AD_DUAL_SUPPORT:
        case DROID_AUDIO_CMD_AD_MIX_SUPPORT:
        case DROID_AUDIO_CMD_AD_MIX_LEVEL:
            setAdFunction(cmdIndex, param1, param2, param3);
            break;
        case DROID_AUDIO_CMD_SET_TSPLAYER_CLIENT_DIED:
            {
                unique_lock<mutex> l(mMutex);
                releaseTvTunerAudioPatch();
                mpAudioPatch = nullptr;
            }
            break;
        case DROID_AUDIO_CMD_SET_AUDIO_PLAYBACK_MODE:
            encapsulationAndSetParams("hal_param_dtv_playback_mode=", param1);
            break;
        case DROID_AUDIO_CMD_SET_AUDIO_PICTURE_MODE:
            AM_LOGD("SET_AUDIO_PICTURE_MODE: %s", (param1 == 1? "GAME" : "STANDARD"));
            if (param1 == 1) {
                AudioSystem::setParameters(String8("picture_mode=PQ_MODE_GAME"));
            } else {
                AudioSystem::setParameters(String8("picture_mode=PQ_MODE_STANDARD"));
            }
            break;
        default:
            AM_LOGW("unknown cmd:%d", cmdIndex);
            break;
    }
    return 0;
}

int32_t DroidAudioConfigSetting::setOutputDevices(const vector<int32_t>& devices) {
    if (devices.size() == 0 || devices.size() > 1) {
        AM_LOGW("devices size is:%zu", devices.size());
        return -1;
    }

    switch (devices[0]) {
        case DROID_AUDIO_FORCE_USE_NONE:
        case DROID_AUDIO_FORCE_USE_SPEAKER:
        case DROID_AUDIO_FORCE_USE_SPDIF:
        case DROID_AUDIO_FORCE_USE_HEADPHONES:
        case DROID_AUDIO_FORCE_USE_HDMI:
        case DROID_AUDIO_FORCE_USE_USB:
        case DROID_AUDIO_FORCE_USE_BT_A2DP:
            break;
        default:
            AM_LOGW("unsupported forceUse:%d", devices[0]);
            return -1;
    }
    AM_LOGI("setForceUse:%d", devices[0]);
    g_SystemControlClient->setProperty(PROP_AUDIO_OUTPUT_FORCEUSE, to_string(devices[0]).c_str());
    AudioSystem::setForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA, (audio_policy_forced_cfg_t)devices[0]);
    return 0;
}

int32_t DroidAudioConfigSetting::getOutputDevices(vector<int32_t>* devices) {
    int32_t audioOutStrategy = g_SystemControlClient->getPropertyInt(PROP_AUDIO_OUTPUT_STRATEGY, DROID_AUDIO_OUTPUT_STRATEGY_AUTO);
    int32_t forceUse = 0;
    if (audioOutStrategy == DROID_AUDIO_OUTPUT_STRATEGY_AUTO) {
        AudioDeviceTypeAddrVector curDevices{};
        audio_attributes_t attributes = AudioSystem::streamTypeToAttributes(AUDIO_STREAM_MUSIC);
        AudioSystem::getDevicesForAttributes(attributes, &curDevices, false);
        for (auto device : curDevices) {
            switch (device.mType) {
                case AUDIO_DEVICE_OUT_SPEAKER:
                    forceUse = DROID_AUDIO_FORCE_USE_SPEAKER;
                    break;
                case AUDIO_DEVICE_OUT_SPDIF:
                    forceUse = DROID_AUDIO_FORCE_USE_SPDIF;
                    break;
                case AUDIO_DEVICE_OUT_WIRED_HEADSET:
                case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
                    forceUse = DROID_AUDIO_FORCE_USE_HEADPHONES;
                    break;
                case AUDIO_DEVICE_OUT_HDMI:
                case AUDIO_DEVICE_OUT_HDMI_ARC:
                case AUDIO_DEVICE_OUT_HDMI_EARC:
                    forceUse = DROID_AUDIO_FORCE_USE_HDMI;
                    break;
                case AUDIO_DEVICE_OUT_USB_DEVICE:
                case AUDIO_DEVICE_OUT_USB_ACCESSORY:
                case AUDIO_DEVICE_OUT_USB_HEADSET:
                    forceUse = DROID_AUDIO_FORCE_USE_USB;
                    break;
                case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP:
                case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
                case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER:
                    forceUse = DROID_AUDIO_FORCE_USE_BT_A2DP;
                    break;
                default:
                    AM_LOGW("unsupported dev0:%#x", device.mType);
                    return -1;
            }
            devices->push_back(forceUse);
            if (getDebugEnable()) {
                AM_LOGI("find device:%s", audio_device_to_string(device.mType));
            }
        }
        if (devices->size() == 0) {
            AM_LOGW("not find sink device");
        }
    } else {
        forceUse = (int32_t)AudioSystem::getForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA);
        switch (forceUse) {
            case DROID_AUDIO_FORCE_USE_SPEAKER:
            case DROID_AUDIO_FORCE_USE_SPDIF:
            case DROID_AUDIO_FORCE_USE_HEADPHONES:
            case DROID_AUDIO_FORCE_USE_HDMI:
            case DROID_AUDIO_FORCE_USE_USB:
            case DROID_AUDIO_FORCE_USE_BT_A2DP:
                break;
            default:
                AM_LOGW("unsupported forceUse:%d", forceUse);
                return -1;
        }
        devices->push_back(forceUse);
        if (getDebugEnable()) {
            AM_LOGD("return forceUse:%d", forceUse);
        }
    }
    return 0;
}

void DroidAudioConfigSetting::updateCoexistSpdifOther() {
    bool enable = getPropertyBoolean(PROP_AUDIO_OUTPUT_SPDIF_COEXIST, true);
    int coexist = enable ? 1 : 0;
    int curState = AudioSystem::getDeviceConnectionState(AUDIO_DEVICE_OUT_SPDIF, "");
    AM_LOGI("coexist:%d, curState:%d", coexist, curState);

    struct audio_port_v7 audioPort{};
    audioPort.type = AUDIO_PORT_TYPE_DEVICE;
    audioPort.ext.device.type = AUDIO_DEVICE_OUT_SPDIF;
    android::media::audio::common::AudioPort aidlAudioPort = legacy2aidl_audio_port_v7_AudioPort(audioPort, false).value();
    if (coexist == curState) {
        audio_policy_dev_state_t state = enable ? AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE : AUDIO_POLICY_DEVICE_STATE_AVAILABLE;
        AudioSystem::setDeviceConnectionState(state, aidlAudioPort, AUDIO_FORMAT_DEFAULT);
    }
}

int32_t DroidAudioConfigSetting::setCoexistSpdifOther(bool enable) {
    if (getDebugEnable()) {
        AM_LOGD("enable:%d", enable);
    }
    g_SystemControlClient->setProperty(PROP_AUDIO_OUTPUT_SPDIF_COEXIST, enable ? "1" : "0");
    updateCoexistSpdifOther();
    int forceUse = AudioSystem::getForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA);
    if (forceUse == DROID_AUDIO_FORCE_USE_SPDIF) {
        g_SystemControlClient->setProperty(PROP_AUDIO_OUTPUT_STRATEGY, to_string(DROID_AUDIO_OUTPUT_STRATEGY_AUTO).c_str());
        AudioSystem::setForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA, (audio_policy_forced_cfg_t)DROID_AUDIO_FORCE_USE_NONE);
        AM_LOGI("delete spdif, setForceUse NONE. enable:%d", enable);
    }
    encapsulationAndSetParams("hal_param_spdif_coexist_other=", (int32_t)enable);
    return 0;
}

int32_t DroidAudioConfigSetting::setMasterMute(bool mute) {
    if (getDebugEnable()) {
        AM_LOGD("enable:%d", mute);
    }
    return AudioSystem::setMasterMute(mute);
}

