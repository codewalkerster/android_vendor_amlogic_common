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

#define LOG_TAG "DroidAudioPatchManager"
//#define LOG_NDEBUG 0

#include <system/audio-base.h>
#include "unistd.h"
#include <log/log.h>

#include <media/AidlConversion.h>
#include <media/AudioSystem.h>


#include "DroidAudioCommon.h"
#include "DroidAudioClientUtils.h"
#include "DroidAudioPatchManager.h"
#include "DroidAudioManager.h"
#include "SystemControlClient.h"

using namespace std;
using namespace android;


static sp<SystemControlClient> g_SystemControlClient;
#define  DVB_DEMUX_ID_BASE      25

void setAdFunction(int cmd, int param1, int param2, int param3 __unused) {
    switch (cmd) {
        case DroidAudioManager::DROID_AUDIO_CMD_AD_SWITCH_ENABLE:
            setParameters("ad_switch_enable=", (param1 > 0 ? 1 : 0));
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_AD_SET_VOLUME:
            setParameters("dual_decoder_advol_level=", param1);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_AD_DUAL_SUPPORT:
            setParameters("hal_param_dual_dec_support=", (param1 > 0 ? 1 : 0));
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_AD_MIX_SUPPORT://Associated audio mixing on/off
            setParameters("hal_param_dual_dec_support=", param1);
            setParameters("hal_param_ad_mix_enable=", param1);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_AD_MIX_LEVEL://Associated audio mixing level
            setParameters("hal_param_dual_dec_mix_level=", param2);
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

int32_t DroidAudioPatchManager::init() {
    if (mInitStatus) {
        AM_LOGI("It's already initialized");
        return 0;
    }
    mNotImptTvHardwareInputService = !getPropertyBoolean("ro.vendor.platform.build.livetv", false);
    mForceManagePatch = getPropertyBoolean("vendor.media.dtv.force.manage.patch", false);
    g_SystemControlClient = ::android::SystemControlClient::getInstance();
    mProcThread = thread(&DroidAudioPatchManager::handleDispatchAudioRoutesChanged, this);
    mInitStatus = true;
    return 0;
}

void DroidAudioPatchManager::reloadAudio() {
    audioPortOrPatchUpdate();
}


void DroidAudioPatchManager::audioPortOrPatchUpdate() {
    if (mNotImptTvHardwareInputService) {
        handleAudioSinkUpdatedRunnable();
    } else {
        // handleDispatchAudioRoutesChanged
        sinkChangedSignalNotify();
    }
}

int32_t DroidAudioPatchManager::dump(int fd, const char **args __unused, uint32_t numArgs __unused) {
    dprintf(fd, "tif:                                   %10d | mForceManagePatch:                %10d\n", !mNotImptTvHardwareInputService, mForceManagePatch);
    for (auto &v : mDemuxs) {
        DroidAudioDemux& demux = v.second;
        dprintf(fd, "START_DECODE(id:%d) format:%d pid:%d start:%d open:%d mute:%d vol:%d\n",
            v.first, demux.mAudioFormat, demux.mAudioPid, demux.mStartStatus, demux.mOpenStatus,
            demux.mMuteStatus, demux.mVolume);
    }
    return STATUS_OK;
}

void DroidAudioPatchManager::sinkChangedSignalNotify() {
    unique_lock<mutex> mutex(mThreadMutex);
    AM_LOGV("audio changed and notify>>>>>>>");
    mThreadCnd.notify_one();
}

void DroidAudioPatchManager::handleDispatchAudioRoutesChanged() {
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
            if (isAudioDebug()) {
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

void DroidAudioPatchManager::handleAudioSinkUpdatedRunnable() {
    int32_t ret = 0;
    unique_lock<mutex> l(mMutex);
    if (mNotImptTvHardwareInputService) {
        if (mpAudioPatch == nullptr) {
            if (isAudioDebug()) {
                AM_LOGD("not find dtv audio patch");
            }
            return;
        }
        ret = recreateAudioPatch();
    } else {
        ret = updateAudioPatch();
    }

    if (mpAudioPatch != nullptr && mCurTunerSourceType == DroidAudioManager::SOURCE_TYPE_ATV) {
        AM_LOGI("ATV source, start playing");
        ::setParameters("hal_param_tuner_in=atv");
    }
}

void DroidAudioPatchManager::findAudioSinkFromAudioPolicy(vector<audio_port_v7>& ports) {
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

int32_t DroidAudioPatchManager::findAudioDevicePort(audio_devices_t type, audio_port_v7& port) {
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

int32_t DroidAudioPatchManager::updateAudioPatch() {
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

int32_t DroidAudioPatchManager::recreateAudioPatch() {
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

void DroidAudioPatchManager::releaseTvTunerAudioPatch() {
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
            setAudioCmdParam(DroidAudioManager::DROID_AUDIO_CMD_CLOSE_DECODER, 0, 0, pathId);
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

int32_t DroidAudioPatchManager::setAudioCmdParam(int32_t cmd, int32_t param1, int32_t param2, int32_t param3) {
    if (isAudioDebug()) {
        AM_LOGD("cmd:%s(%d) param1:%d param2:%d param3:%d", DroidAudioManager::audioCmd2Str(cmd), cmd, param1, param2, param3);
    }

    int cmdIndex = cmd;

    if (param3 != -1) {
        cmd = cmd + (param3 << DVB_DEMUX_ID_BASE);
        param1 = param1 + (param3 << DVB_DEMUX_ID_BASE);
        param2 = param2 + (param3 << DVB_DEMUX_ID_BASE);
    }

    map<int, DroidAudioDemux>::iterator iter;
    switch (cmdIndex) {

        case DroidAudioManager::DROID_AUDIO_CMD_SET_SPDIF_PROTECTION_MODE:
            setParameters("hal_param_dtv_spdif_protection_mode=", param1);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_AUDIO_PATCH_ADDRESS:
            setParameters("hal_param_dtv_audio_patch_address=", param1);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_DEMUX_INFO:
            setParameters("hal_param_dtv_demux_id=", param2);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_SECURITY_MEM_LEVEL:
            setParameters("hal_param_security_mem_level=", param1);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_MEDIA_SYCN_ID:
            setParameters("hal_param_media_sync_id=", param1);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_MEDIA_FIRST_LANG:
            setParameters("hal_param_dtv_media_first_lang=", param1);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_MEDIA_SECOND_LANG:
            setParameters("hal_param_dtv_media_second_lang=", param1);
           break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_HAS_VIDEO:
            setParameters("hal_param_has_dtv_video=", param1);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_START_DECODE:
            setParameters("hal_param_dtv_audio_fmt=", param1);
            setParameters("hal_param_dtv_audio_id=", param2);
            setParameters("hal_param_dtv_patch_cmd=", cmd);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_PAUSE_DECODE:
            setParameters("hal_param_dtv_patch_cmd=", cmd);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_RESUME_DECODE:
            setParameters("hal_param_dtv_patch_cmd=", cmd);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_STOP_DECODE:
            setParameters("hal_param_dtv_patch_cmd=", cmd);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_DECODE_AD:
            setParameters("hal_param_dtv_sub_audio_fmt=", param1);
            setParameters("hal_param_dtv_sub_audio_pid=", param2);
            setParameters("hal_param_dtv_patch_cmd=", cmd);
            AM_LOGD("SET_DECODE_AD sub_audio_fmt:%d, sub_audio_pid:%d", param1, param2);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_VOLUME:
            setParameters("hal_param_dtv_audio_volume=", param1);
            AM_LOGD("CMD_SET_VOLUME, audio volume:%d", param1);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_MUTE:
            setParameters("hal_param_tv_mute=", param1);
            AM_LOGD("CMD_SET_MUTE, audio mute:%d", param1);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_OUTPUT_MODE:
            setParameters("hal_param_audio_output_mode=", param1); /* refer to AM_AOUT_OutputMode_t */
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_PRE_GAIN:
            setParameters("hal_param_dtv_patch_cmd=", cmd);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_PRE_MUTE:
            setParameters("hal_param_dtv_patch_cmd=", cmd);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_OPEN_DECODER:
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
                setParameters("hal_param_dtv_audio_fmt=", param1);
                setParameters("hal_param_dtv_audio_id=", param2);
                setParameters("hal_param_dtv_patch_cmd=", cmd);
                if (isAudioDebug()) {
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
                if (isAudioDebug()) {
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
        case DroidAudioManager::DROID_AUDIO_CMD_CLOSE_DECODER:
            {
                setParameters("hal_param_dtv_patch_cmd=", cmd);
                unique_lock<mutex> demux_l(mDemuxMutex);
                if (isAudioDebug()) {
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
                if (isAudioDebug()) {
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
        case DroidAudioManager::DROID_AUDIO_CMD_SET_MEDIA_PRESENTATION_ID:
            setParameters("hal_param_dtv_media_presentation_id=", param1);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_AUDIO_PATCH_MANAGE_MODE:
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
        case DroidAudioManager::DROID_AUDIO_CMD_AD_SWITCH_ENABLE:
        case DroidAudioManager::DROID_AUDIO_CMD_AD_SET_VOLUME:
        case DroidAudioManager::DROID_AUDIO_CMD_AD_DUAL_SUPPORT:
        case DroidAudioManager::DROID_AUDIO_CMD_AD_MIX_SUPPORT:
        case DroidAudioManager::DROID_AUDIO_CMD_AD_MIX_LEVEL:
            setAdFunction(cmdIndex, param1, param2, param3);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_TSPLAYER_CLIENT_DIED:
            {
                unique_lock<mutex> l(mMutex);
                releaseTvTunerAudioPatch();
                mpAudioPatch = nullptr;
            }
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_AUDIO_PLAYBACK_MODE:
            setParameters("hal_param_dtv_playback_mode=", param1);
            break;
        case DroidAudioManager::DROID_AUDIO_CMD_SET_AUDIO_PICTURE_MODE:
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

int32_t DroidAudioPatchManager::createAudioPatch(int32_t sourceDevice, int32_t sinkDevice) {
    audio_patch_handle_t handle = 0;
    struct audio_patch patch = {};
    int32_t ret = 0;

    audio_port_v7 source;
    audio_port_v7 sink;
    ret = findAudioDevicePort((audio_devices_t)sourceDevice, source);
    R_CHECK_RET(ret, "not found source device:%#x", sourceDevice)
    ret = findAudioDevicePort((audio_devices_t)sinkDevice, sink);
    R_CHECK_RET(ret, "not found sink device:%#x", sinkDevice)

    patch.sources[0].id = source.id;
    patch.sources[0].role = source.role;
    patch.sources[0].type = source.type;
    patch.sources[0].channel_mask = AUDIO_CHANNEL_IN_STEREO;
    patch.sources[0].sample_rate = 48000;
    patch.sources[0].format = AUDIO_FORMAT_PCM_16_BIT;
    patch.sources[0].config_mask = AUDIO_PORT_CONFIG_ALL;
    patch.sources[0].ext.device.type = (audio_devices_t)sourceDevice;
    patch.num_sources = 1;

    patch.sinks[0].id = sink.id;
    patch.sinks[0].role = sink.role;
    patch.sinks[0].type = sink.type;
    patch.sinks[0].channel_mask = AUDIO_CHANNEL_IN_STEREO;
    patch.sinks[0].sample_rate = 48000;
    patch.sinks[0].format = AUDIO_FORMAT_PCM_16_BIT;
    patch.sinks[0].config_mask = AUDIO_PORT_CONFIG_ALL;
    patch.sinks[0].ext.device.type = (audio_devices_t)sinkDevice;
    patch.num_sinks = 1;
    ret = AudioSystem::createAudioPatch(&patch, &handle);
    R_CHECK_RET(ret,)
    return handle;
}

int32_t DroidAudioPatchManager::releaseAudioPatch(int32_t handle) {
    status_t ret = AudioSystem::releaseAudioPatch(handle);
    R_CHECK_RET(ret,)
    return 0;
}

int32_t DroidAudioPatchManager::openTvAudio(int32_t source) {
    AM_LOGI("source: %d", source);
    if (source == DroidAudioManager::SOURCE_TYPE_ATV /* SOURCE_TYPE_ATV */) {
        ::setParameters("hal_param_tuner_in=atv");
    } else if (source == DroidAudioManager::SOURCE_TYPE_DTV /* SOURCE_TYPE_DTV */) {
        ::setParameters("hal_param_tuner_in=dtv");
    } else {
        AM_LOGW("openTvAudio unsupported source type:%d", source);
    }
    mCurTunerSourceType = source;
    return 0;
}

