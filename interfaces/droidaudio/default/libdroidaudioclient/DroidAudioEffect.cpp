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

#define LOG_TAG "DroidAudioEffect.C"
//#define LOG_NDEBUG 0


#include <log/log.h>
#include <cutils/properties.h>


#include "DroidAudioClientUtils.h"
#include "DroidAudioEffect.h"
#include "DroidAudioGetService.h"

int32_t DroidAudioEffect::setAudioEffectEnabled(int32_t effectId, bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setAudioEffectEnabled, effectId, enable);
}

bool DroidAudioEffect::isAudioEffectEnabled(int32_t effectId) {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioEffect_isAudioEffectEnabled, effectId);
}

int32_t DroidAudioEffect::setParameter(int32_t effectId, const vector<uint8_t>& param, const vector<uint8_t>& value) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setParameter, effectId, param, value);
}

int32_t DroidAudioEffect::setParameter(int32_t effectId, int32_t param, int32_t value) {
    return DroidAudioEffect::setParameter(effectId, valueToByteArray(param), valueToByteArray(value));
}
int32_t DroidAudioEffect::setParameter(int32_t effectId, int32_t param, const vector<uint8_t>& value) {
    return DroidAudioEffect::setParameter(effectId, valueToByteArray(param), value);
}
int32_t DroidAudioEffect::getParameter(int32_t effectId, const vector<uint8_t>& param, vector<uint8_t>* pValue) {
    R_CHECK_POINTER_LEGAL(-1, pValue,)
    *pValue = AML_AIDL_EXECUTE_FUNCTION(vector<uint8_t>, &IDroidAudio::AudioEffect_getParameter, effectId, param);
    return 0;
}
int32_t DroidAudioEffect::getParameter(int32_t effectId, int32_t param) {
    vector<uint8_t> valueBuffer(4);
    DroidAudioEffect::getParameter(effectId, valueToByteArray(param), &valueBuffer);
    int32_t value = byteArrayToInt(valueBuffer);
    return value;
}
int32_t DroidAudioEffect::getParameter(int32_t effectId, int32_t param, vector<uint8_t>* pValue) {
    return DroidAudioEffect::getParameter(effectId, valueToByteArray(param), pValue);
}

int32_t DroidAudioEffect::setParameter(int32_t effectId, effect_param_t *param) {
    R_CHECK_POINTER_LEGAL(-1, param,)
    vector<uint8_t> pdata;
    vector<uint8_t> vdata;
    pdata.resize(param->psize);
    vdata.resize(param->vsize);
    memcpy(pdata.data(), param->data, param->psize);
    memcpy(vdata.data(), param->data + param->psize, param->vsize);
    return DroidAudioEffect::setParameter(effectId, pdata, vdata);
}
int32_t DroidAudioEffect::getParameter(int32_t effectId, effect_param_t *param) {
    R_CHECK_POINTER_LEGAL(-1, param,)
    vector<uint8_t> pdata;
    vector<uint8_t> vdata;
    pdata.resize(param->psize);
    memcpy(pdata.data(), param->data, param->psize);
    DroidAudioEffect::getParameter(effectId, pdata, &vdata);
    if (vdata.size() != param->vsize) {
        AM_LOGW("need size:%d != ret size:%d", vdata.size(), param->vsize);
        return -1;
    }
    memcpy(param->data, vdata.data(), param->vsize);
    return 0;
}

int32_t DroidAudioEffect::setBasicEffectEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setBasicEffectEnabled, enable);
}
bool DroidAudioEffect::isBasicEffectEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioEffect_isBasicEffectEnabled);
}
int32_t DroidAudioEffect::initDualEffectMode() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_initDualEffectMode);
}
int32_t DroidAudioEffect::setDualEffectMode(int32_t mode) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setDualEffectMode, mode);
}
int32_t DroidAudioEffect::getDualEffectMode() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_getDualEffectMode);
}
int32_t DroidAudioEffect::getEffectFunctionConfig(int32_t id) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_getEffectFunctionConfig, id);
}

int32_t DroidAudioEffect::setBalance(int32_t step) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setBalance, step);
}
int32_t DroidAudioEffect::getBalance() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_getBalance);
}

int32_t DroidAudioEffect::setTreble(int32_t step) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setTreble, step);
}
int32_t DroidAudioEffect::getTreble() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_getTreble);
}
int32_t DroidAudioEffect::setBass(int32_t step) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setBass, step);
}
int32_t DroidAudioEffect::getBass() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_getBass);
}

int32_t DroidAudioEffect::setDapParam(int32_t id, int32_t value) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setDapParam, id, value);
}
int32_t DroidAudioEffect::getDapParam(int32_t id) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_getDapParam, id);
}

int32_t DroidAudioEffect::setDpeParam(int32_t id, int32_t value) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setDpeParam, id, value);
}
int32_t DroidAudioEffect::getDpeParam(int32_t id) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_getDpeParam, id);
}

int32_t DroidAudioEffect::setSoundMode(int32_t mode) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setSoundMode, mode);
}
int32_t DroidAudioEffect::getSoundMode() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_getSoundMode);
}
int32_t DroidAudioEffect::setUserSoundModeParam(int32_t bandNumber, int32_t value, int32_t bandSum) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setUserSoundModeParam, bandNumber, value, bandSum);
}
int32_t DroidAudioEffect::getUserSoundModeParam(int32_t bandNumber) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_getUserSoundModeParam, bandNumber);
}
int32_t DroidAudioEffect::setHpeqBandNum(int32_t num) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setHpeqBandNum, num);
}
int32_t DroidAudioEffect::getHpeqBandNum() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_getHpeqBandNum);
}

int32_t DroidAudioEffect::setVirtualSurroundEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setVirtualSurroundEnabled, enable);
}
bool DroidAudioEffect::isVirtualSurroundEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioEffect_isVirtualSurroundEnabled);
}

bool DroidAudioEffect::isDtsVXValidEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioEffect_isDtsVXValidEnabled);
}
int32_t DroidAudioEffect::setDtsVirtualXEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setDtsVirtualXEnabled, enable);
}
bool DroidAudioEffect::isDtsVirtualXEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioEffect_isDtsVirtualXEnabled);
}
int32_t DroidAudioEffect::setDtsVirtualSurroundEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setDtsVirtualSurroundEnabled, enable);
}
bool DroidAudioEffect::isDtsVirtualSurroundEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioEffect_isDtsVirtualSurroundEnabled);
}
int32_t DroidAudioEffect::setDtsBassEnhancementEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setDtsBassEnhancementEnabled, enable);
}
bool DroidAudioEffect::isDtsBassEnhancementEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioEffect_isDtsBassEnhancementEnabled);
}
int32_t DroidAudioEffect::setDtsDialogClarityMode(int32_t mode) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setDtsDialogClarityMode, mode);
}
int32_t DroidAudioEffect::getDtsDialogClarityMode() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_getDtsDialogClarityMode);
}
int32_t DroidAudioEffect::setDtsVirtualXMode(int32_t mode) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setDtsVirtualXMode, mode);
}
int32_t DroidAudioEffect::getDtsVirtualXMode() {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_getDtsVirtualXMode);
}
int32_t DroidAudioEffect::setDtsTruVolumeHdEnabled(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setDtsTruVolumeHdEnabled, enable);
}
bool DroidAudioEffect::isDtsTruVolumeHdEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioEffect_isDtsTruVolumeHdEnabled);
}
int32_t DroidAudioEffect::setAISoundModeEnable(bool enable) {
    return AML_AIDL_EXECUTE_FUNCTION(int32_t, &IDroidAudio::AudioEffect_setAISoundModeEnable, enable);
}
bool DroidAudioEffect::isAISoundModeEnabled() {
    return AML_AIDL_EXECUTE_FUNCTION(bool, &IDroidAudio::AudioEffect_isAISoundModeEnabled);
}

vector<uint8_t> valueToByteArray(int32_t value) {
//        paramBuffer[0] = param & 0xff;
//        paramBuffer[1] = (param >> 8) & 0xff;
//        paramBuffer[2] = (param >> 16) & 0xff;
//        paramBuffer[3] = (param >> 24) & 0xff;
    vector<uint8_t> byteArray(sizeof(int32_t));
    memcpy(byteArray.data(), &value, sizeof(int));
    return byteArray;
}

vector<uint8_t> valueToByteArray(float value) {
    vector<uint8_t> byteArray(sizeof(float));
    memcpy(byteArray.data(), &value, sizeof(float));
    return byteArray;
}

int32_t byteArrayToInt(vector<uint8_t> byteArray) {
    if (byteArray.size() != 4) {
        AM_LOGE("invalid size:%zu", byteArray.size());
        return 0;
    }
    int32_t value = 0;
    value |= byteArray[0];
    value |= byteArray[1] << 8;
    value |= byteArray[2] << 16;
    value |= byteArray[3] << 24;
    return value;
}

float byteArrayToFloat(const vector<uint8_t>& byteArray) {
    if (byteArray.size() != sizeof(float)) {
        AM_LOGE("invalid size:%zu", byteArray.size());
        return 0.0f;
    }
    float result;
    memcpy(&result, byteArray.data(), sizeof(float));
    return result;
}

vector<uint8_t> extractSubvector(const vector<uint8_t>& input, uint32_t start, uint32_t end) {
    if (start > end || end > input.size()) {
        AM_LOGE("invalid size, start:%d end:%d, input size:%zu", start, end, input.size());
        return vector<uint8_t>();
    }
    return vector<uint8_t>(input.begin() + start, input.begin() + end);
}

const char* DroidAudioEffect::soundMode2Str(int32_t type) {
    ENUM_TYPE_TO_STR_START("COMMON_SOUND_MODE_");
     ENUM_TYPE_TO_STR(COMMON_SOUND_MODE_DYNAMIC)
    ENUM_TYPE_TO_STR(COMMON_SOUND_MODE_STANDARD)
    ENUM_TYPE_TO_STR(COMMON_SOUND_MODE_MUSIC)
    ENUM_TYPE_TO_STR(COMMON_SOUND_MODE_NEWS)
    ENUM_TYPE_TO_STR(COMMON_SOUND_MODE_MOVIE)
    ENUM_TYPE_TO_STR(COMMON_SOUND_MODE_GAME)
    ENUM_TYPE_TO_STR(COMMON_SOUND_MODE_CUSTOM)
    ENUM_TYPE_TO_STR(COMMON_SOUND_MODE_NIGHT)
    ENUM_TYPE_TO_STR_END
}

const char* DroidAudioEffect::dualEffectMode2Str(int32_t type) {
    ENUM_TYPE_TO_STR_START("DUAL_EFFECT_MODE_");
    ENUM_TYPE_TO_STR(DUAL_EFFECT_MODE_AUTO)
    ENUM_TYPE_TO_STR(DUAL_EFFECT_MODE_DTS)
    ENUM_TYPE_TO_STR(DUAL_EFFECT_MODE_DOLBY)
    ENUM_TYPE_TO_STR(DUAL_EFFECT_MODE_OFF)
    ENUM_TYPE_TO_STR_END
}

