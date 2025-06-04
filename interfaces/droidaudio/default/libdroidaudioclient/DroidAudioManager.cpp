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

#include "DroidAudioClientUtils.h"
#include "DroidAudioGetService.h"
#include "DroidAudioManager.h"


extern "C" {
int setAudioParams(int cmd, int param1, int param2, int param3) {
    int32_t ret = DroidAudioManager::setAudioCmdParam(cmd, param1, param2, param3);
    return ret;
}}

int32_t DroidAudioManager::reset() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_reset);
}
int32_t DroidAudioManager::setAudioCmdParam(int32_t cmd, int32_t param1, int32_t param2, int32_t param3) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setAudioCmdParam, cmd, param1, param2, param3);
}
int32_t DroidAudioManager::setOutputDevices(const vector<int32_t>& devices) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setOutputDevices, devices);
}
int32_t DroidAudioManager::getOutputDevices(vector<int32_t>* devices) {
    R_CHECK_POINTER_LEGAL(-1, devices,)
    *devices = AML_AIDL_EXECUTE_FUNCTION(vector<int32_t>, &IDroidAudio::AudioManager_getOutputDevices);
    return 0;
}
int32_t DroidAudioManager::setCoexistSpdifOtherEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setCoexistSpdifOtherEnabled, enable);
}
int32_t DroidAudioManager::isCoexistSpdifOtherEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioManager_isCoexistSpdifOtherEnabled);
}
int32_t DroidAudioManager::setSoundBarModeEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setSoundBarModeEnabled, enable);
}
bool DroidAudioManager::isSoundBarModeEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioManager_isSoundBarModeEnabled);
}
int32_t DroidAudioManager::setSoundSpdifEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setSoundSpdifEnabled, enable);
}
bool DroidAudioManager::isSoundSpdifEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioManager_isSoundSpdifEnabled);
}
int32_t DroidAudioManager::setSpeakerEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setSpeakerEnabled, enable);
}
bool DroidAudioManager::isSpeakerEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioManager_isSpeakerEnabled);
}
int32_t DroidAudioManager::setOutputDeviceDelay(int32_t source, int32_t device, int32_t delayMs) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setOutputDeviceDelay, source, device, delayMs);
}
int32_t DroidAudioManager::getOutputDeviceDelay(int32_t source, int32_t device) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getOutputDeviceDelay, source, device);
}
int32_t DroidAudioManager::setAudioOutputAllDelay(int32_t delayMs) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setAudioOutputAllDelay, delayMs);
}
int32_t DroidAudioManager::getAudioOutputAllDelay() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getAudioOutputAllDelay);
}
int32_t DroidAudioManager::setTvSourceType(int32_t source) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setTvSourceType, source);
}
int32_t DroidAudioManager::getTvSourceType() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getTvSourceType);
}
int32_t DroidAudioManager::setAudioApplyToAll() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setAudioApplyToAll);
}
int32_t DroidAudioManager::setDigitalAudioMode(int32_t mode, const string& formats) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setDigitalAudioMode, mode, formats);
}
int32_t DroidAudioManager::getDigitalAudioMode() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getDigitalAudioMode);
}
int32_t DroidAudioManager::setForceDDPEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setForceDDPEnabled, enable);
}
bool DroidAudioManager::isForceDDPEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioManager_isForceDDPEnabled);
}
int32_t DroidAudioManager::setDolbyDrcMode(int32_t mode) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setDolbyDrcMode, mode);
}
int32_t DroidAudioManager::getDolbyDrcMode() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getDolbyDrcMode);
}
int32_t DroidAudioManager::setDolbyDrcLineLevel(int32_t level) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setDolbyDrcLineLevel, level);
}
int32_t DroidAudioManager::getDolbyDrcLineLevel() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getDolbyDrcLineLevel);
}
bool DroidAudioManager::isDtsXEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioManager_isDtsXEnabled);
}
int32_t DroidAudioManager::setDtsXDrcEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setDtsXDrcEnabled, enable);
}
bool DroidAudioManager::isDtsXDrcEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioManager_isDtsXDrcEnabled);
}
int32_t DroidAudioManager::setDialogEnhancerLevel(int32_t level) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setDialogEnhancerLevel, level);
}
int32_t DroidAudioManager::getDialogEnhancerLevel() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getDialogEnhancerLevel);
}
int32_t DroidAudioManager::setSoundDmxMode(int32_t mode) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setSoundDmxMode, mode);
}
int32_t DroidAudioManager::getSoundDmxMode() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getSoundDmxMode);
}
int32_t DroidAudioManager::setSoundLevelerMode(int32_t mode) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setSoundLevelerMode, mode);
}
int32_t DroidAudioManager::getSoundLevelerMode() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getSoundLevelerMode);
}
int32_t DroidAudioManager::setSoundLevelerAmount(int32_t value) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setSoundLevelerAmount, value);
}
int32_t DroidAudioManager::getSoundLevelerAmount() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getSoundLevelerAmount);
}
int32_t DroidAudioManager::setVadEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setVadEnabled, enable);
}
bool DroidAudioManager::isVadEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioManager_isVadEnabled);
}
int32_t DroidAudioManager::openTvAudio(int32_t source) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_openTvAudio, source);
}
int32_t DroidAudioManager::getDroidAudioConfig(int32_t id) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getDroidAudioConfig, id);
}
int32_t DroidAudioManager::setParameters(const string& keyValuePairs) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setParameters, keyValuePairs);
}
string DroidAudioManager::getParameters(const string& keys) {
    return AML_AIDL_EXECUTE_FUNCTION(string, &IDroidAudio::AudioManager_getParameters, keys);
}
int32_t DroidAudioManager::createAudioPatch(int32_t sourceDevice, int32_t sinkDevice) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_createAudioPatch, sourceDevice, sinkDevice);
}
int32_t DroidAudioManager::releaseAudioPatch(int32_t handle) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_releaseAudioPatch, handle);
}

int32_t DroidAudioManager::setAiDeEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setAiDeEnabled, enable);
}

bool DroidAudioManager::isAiDeEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioManager_isAiDeEnabled);
}

int32_t DroidAudioManager::setAiDeGain(int32_t gain) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setAiDeGain, gain);
}

int32_t DroidAudioManager::getAiDeGain() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getAiDeGain);
}

int32_t DroidAudioManager::setGlobalMicEnable(int32_t source, bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setGlobalMicEnable, source, enable);
}
bool DroidAudioManager::getGlobalMicStatus(int32_t source) {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioManager_getGlobalMicStatus, source);
}
int32_t DroidAudioManager::setMicSource(int32_t source) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setMicSource, source);
}
int32_t DroidAudioManager::getMicSource() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getMicSource);
}
int32_t DroidAudioManager::setMicMute(int32_t source, bool mute) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setMicMute, source, mute);
}
bool DroidAudioManager::isMicMute(int32_t source) {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioManager_isMicMute, source);
}
int32_t DroidAudioManager::setMicGain(int32_t source, int32_t gain) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setMicGain, source, gain);
}
int32_t DroidAudioManager::getMicGain(int32_t source) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getMicGain, source);
}
int32_t DroidAudioManager::setMicReverb(int32_t source, bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setMicReverb, source, enable);
}
bool DroidAudioManager::isEnableMicReverb(int32_t source) {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioManager_isEnableMicReverb, source);
}
int32_t DroidAudioManager::setMicReverbLevel(int32_t source, int32_t level) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_setMicReverbLevel, source, level);
}
int32_t DroidAudioManager::getMicReverbLevel(int32_t source) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioManager_getMicReverbLevel, source);
}

const char* DroidAudioManager::audioCmd2Str(int32_t type) {
    ENUM_TYPE_TO_STR_START("DroidAudioManager::DROID_AUDIO_CMD_");
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_START_DECODE)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_PAUSE_DECODE)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_RESUME_DECODE)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_STOP_DECODE)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_DECODE_AD)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_VOLUME)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_MUTE)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_OUTPUT_MODE)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_PRE_GAIN)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_PRE_MUTE)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_OPEN_DECODER)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_CLOSE_DECODER)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_DEMUX_INFO)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_SECURITY_MEM_LEVEL)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_HAS_VIDEO)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_MEDIA_SYCN_ID)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_AD_SWITCH_ENABLE)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_AD_SET_VOLUME)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_AD_DUAL_SUPPORT)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_AD_MIX_SUPPORT)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_AD_MIX_LEVEL)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_AD_SET_MAIN)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_AD_SET_ASSOCIATE)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_MEDIA_PRESENTATION_ID)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_AUDIO_PATCH_MANAGE_MODE)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_SPDIF_PROTECTION_MODE)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_TSPLAYER_CLIENT_DIED)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_MEDIA_FIRST_LANG)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_MEDIA_SECOND_LANG)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_AUDIO_PICTURE_MODE)
    ENUM_TYPE_TO_STR(DroidAudioManager::DROID_AUDIO_CMD_SET_AUDIO_PLAYBACK_MODE)
    ENUM_TYPE_TO_STR_END
}

const char* DroidAudioManager::audioDigitalMode2Str(int32_t type) {
    ENUM_TYPE_TO_STR_START("DroidAudioManager::DIGITAL_AUDIO_MODE_");
    ENUM_TYPE_TO_STR(DroidAudioManager::DIGITAL_AUDIO_MODE_PCM)
    ENUM_TYPE_TO_STR(DroidAudioManager::DIGITAL_AUDIO_MODE_AUTO)
    ENUM_TYPE_TO_STR(DroidAudioManager::DIGITAL_AUDIO_MODE_MANUAL)
    ENUM_TYPE_TO_STR(DroidAudioManager::DIGITAL_AUDIO_MODE_PASSTHROUGH)
    ENUM_TYPE_TO_STR(DroidAudioManager::DIGITAL_AUDIO_MODE_ALWAYS)
    ENUM_TYPE_TO_STR_END
}

const char* DroidAudioManager::tvSource2Str(int32_t type) {
    ENUM_TYPE_TO_STR_START("DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_");
    ENUM_TYPE_TO_STR(DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_ATV)
    ENUM_TYPE_TO_STR(DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_DTV)
    ENUM_TYPE_TO_STR(DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_AV)
    ENUM_TYPE_TO_STR(DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_HDMI)
    ENUM_TYPE_TO_STR(DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_MEDIA)
    ENUM_TYPE_TO_STR_END
}

const char* DroidAudioManager::audioDelayDev2Str(int32_t type) {
    ENUM_TYPE_TO_STR_START("DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_");
    ENUM_TYPE_TO_STR(DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPEAKER)
    ENUM_TYPE_TO_STR(DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPDIF)
    ENUM_TYPE_TO_STR(DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE)
    ENUM_TYPE_TO_STR(DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_ALL)
    ENUM_TYPE_TO_STR_END
}

