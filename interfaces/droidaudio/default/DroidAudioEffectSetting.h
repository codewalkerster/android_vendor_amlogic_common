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
#include <shared_mutex>

#include <media/AudioEffect.h>

#include "DroidAudioEffect.h"

using namespace std;
using namespace android;

class AudioEffect;
class DroidAudioEffectSetting;
class DroidAudioDbDescriptor;

class AmlAudioEffect : public android::AudioEffect {
public:
    AmlAudioEffect(DroidAudioEffectSetting *pSetting,int32_t id, const string& name, const string& type, const string& uuid);
    virtual ~AmlAudioEffect() {}
    virtual int32_t init() = 0;
    int32_t putToDb(const string& key, int32_t value);
    int32_t getIntFromDb(const string& key);
    int32_t getIntFromIni(const string& key);
    using android::AudioEffect::setParameter;
    int32_t setParameter(int32_t param, int32_t value);
    int32_t setParameter(int32_t param, const vector<uint8_t>& value);
    int32_t setParameter(const vector<uint8_t>& param, const vector<uint8_t>& value);
    using android::AudioEffect::getParameter;
    int32_t getParameter(int32_t param);
    int32_t getParameter(int32_t param, vector<uint8_t>& value);
    int32_t getParameter(const vector<uint8_t>& param, vector<uint8_t>& value);
    DroidAudioEffectSetting*        mpSetting;
    int32_t                         mId;
    const string                    mEffectName;
    const string                    mEffectType;
    const string                    mEffectUuid;
};

class AmlAudioEffectHpeq : public AmlAudioEffect {
public:
    AmlAudioEffectHpeq(DroidAudioEffectSetting* pSetting);
    int32_t init() override;
    int32_t seParamEnable(int32_t enable);
    int32_t setSoundMode(int32_t mode);
    int32_t getSoundMode();
    int32_t setUserSoundModeParam(int32_t bandNumber, int32_t value, int32_t bandSum);
    int32_t getUserSoundModeParam(int32_t bandNumber);
    int32_t setHpeqBandNum(int32_t num);
    int32_t setDifferentBandEffects(int32_t bandnum, int32_t value, bool needsave);
    static int32_t toHpeqSoundMode(int32_t mode);
private:
    enum HPEQ_SOUND_MODE_E {
        HPEQ_SOUND_MODE_STANDARD                                                   = 0,
        HPEQ_SOUND_MODE_MUSIC                                                      = 1,
        HPEQ_SOUND_MODE_NEWS                                                       = 2,
        HPEQ_SOUND_MODE_MOVIE                                                      = 3,
        HPEQ_SOUND_MODE_GAME                                                       = 4,
        HPEQ_SOUND_MODE_CUSTOM                                                     = 5,
        HPEQ_SOUND_MODE_MIN                                                        = HPEQ_SOUND_MODE_STANDARD,
        HPEQ_SOUND_MODE_MAX                                                        = HPEQ_SOUND_MODE_CUSTOM,
    };
    enum {
        PARAM_CMD_ENABLE                                                            = 0,
        PARAM_CMD_BAND_NUM                                                          = 1,
        PARAM_CMD_SOUND_MODE                                                        = 2,
        PARAM_CMD_CUSTOM                                                            = 3,
    };
    string eqBandNumberToDbId(int32_t bandNumber);
};

class AmlAudioEffectBalance : public AmlAudioEffect {
public:
    AmlAudioEffectBalance(DroidAudioEffectSetting* pSetting);
    int32_t init() override;
    int32_t seParamEnable(int32_t enable);
    int32_t setBalance(int32_t step);
    int32_t getBalance();
private:
    enum {
        PARAM_CMD_BALANCE_LEVEL                                                     = 0,
        PARAM_CMD_BALANCE_ENABLE                                                    = 1,
    };
};

class AmlAudioEffectTrebleBass : public AmlAudioEffect {
public:
    AmlAudioEffectTrebleBass(DroidAudioEffectSetting* pSetting);
    int32_t init() override;
    int32_t seParamEnable(int32_t enable);
    int32_t setTreble(int32_t step);
    int32_t getTreble();
    int32_t setBass(int32_t step);
    int32_t getBass();
private:
    enum {
        PARAM_CMD_TREBLEBASS_BASS_LEVEL                                             = 0,
        PARAM_CMD_TREBLEBASS_TREBLE_LEVEL                                           = 1,
        PARAM_CMD_TREBLEBASS_ENABLE                                                 = 2,
    };
};

class AmlAudioEffectVirtualSurround : public AmlAudioEffect {
public:
    AmlAudioEffectVirtualSurround(DroidAudioEffectSetting* pSetting);
    int32_t init();
    int32_t setVirtualSurroundEnabled (bool enable);
    bool isVirtualSurroundEnabled();
private:
    enum {
        PARAM_CMD_ENABLE                                                            = 0,
    };
};

class AmlAudioEffectDpe : public AmlAudioEffect {
public:
    AmlAudioEffectDpe(DroidAudioEffectSetting* pSetting);
    int32_t init() override;
    int32_t setDpeParam(int32_t id, int32_t value);
    int32_t getDpeParam(int32_t id);
private:
    int32_t setDpeToDb(int32_t id, int32_t value);
    int32_t getDpeParamInternal(int32_t id);
    const int32_t DEFAULT_DPE_BAND_AMOUNT       = 3;
    const float DEFAULT_DPE_FRAME_DURATION      = 5.0f;
    static const std::unordered_map<int32_t, const char*> mMapDpeDbConvert;
};

class AmlAudioEffectDap : public AmlAudioEffect {
public:
    static const char* INI_KEY_AM_AUDIO_EFFECT_DAP_VERSION;
    AmlAudioEffectDap(DroidAudioEffectSetting* pSetting, int32_t dapVersion);
    int32_t init() override;
    int32_t setDapEnable(int32_t enable);
    int32_t getDapEnable();
    int32_t setDapParam(int32_t id, int32_t value);
    int32_t getDapParam(int32_t id);
    static int32_t toDapProfileID(int32_t mode);
private:
    enum {
        AML_AUDIO_EFFECT_ID_DAP1_3_2                                                = 0,
        AML_AUDIO_EFFECT_ID_DAP2_4                                                  = 1,
    };
    void initDap_1_3_2();
    void initDap_2_4();
    int32_t getDapParamInternal(int32_t id);
    int32_t saveDapParam(int32_t id, int32_t value);
    int32_t saveDbDap24Param(int32_t id, int32_t value);
    int32_t getDbDap24Param(int32_t id);
    int32_t  mDapVersion = AML_AUDIO_EFFECT_ID_DAP2_4;
};

class AmlAudioEffectVirtualX : public AmlAudioEffect {
public:
    AmlAudioEffectVirtualX(DroidAudioEffectSetting* pSetting);
    int32_t init() override;
    int32_t setDtsVirtualXUserMode(int32_t mode);
    int32_t setDtsVirtualXEnabled(bool enable);
    bool isDtsVirtualXEnabled();
    int32_t setDtsVirtualSurroundEnabled(bool enable);
    bool isDtsVirtualSurroundEnabled();
    int32_t setDtsBassEnhancementEnabled(bool enable);
    bool isDtsBassEnhancementEnabled();
    int32_t setDtsDialogClarityMode(int32_t mode);
    int32_t getDtsDialogClarityMode();
    int32_t setDtsVirtualXMode(int32_t virtualXMode);
    int32_t getDtsVirtualXMode();
    int32_t setDtsTruVolumeHdEnabled(bool enable);
    bool isDtsTruVolumeHdEnabled();

private:
    const char* INI_KEY_AM_AUDIO_EFFECT_VIRTUALX_VERSION                            = "ini_key_am_audio_effect_virtualx_version";
    enum {
        PARAM_CMD_DTS_MBHL_ENABLE_I32                                               = 0,
        PARAM_CMD_DTS_TBHDX_ENABLE_I32                                              = 35,
        PARAM_CMD_DTS_VX_ENABLE_I32                                                 = 46,
        PARAM_CMD_DTS_LOUDNESS_CONTROL_ENABLE_I32                                   = 67,
        PARAM_CMD_DTS_ENABLE_V4                                                     = 82,
        PARAM_CMD_DIALOGCLARITY_MODE                                                = 83,
        PARAM_CMD_SURROUND_MODE                                                     = 84,
        PARAM_CMD_DTS_VIRTUALX_USER_MODE                                            = 96,
        PARAM_CMD_TBHDX_PROCESS_DISCARD_I32                                         = 81,
    };
    enum {
        INI_VALUE_AM_AUDIO_EFFECT_VIRTUALX_VERSION_1                                = 1,
        INI_VALUE_AM_AUDIO_EFFECT_VIRTUALX_VERSION_4                                = 4,
    };
    int32_t         mVirtualXVersion = INI_VALUE_AM_AUDIO_EFFECT_VIRTUALX_VERSION_1;
};

class AmlAudioEffectAmlPeq : public AmlAudioEffect {
public:
    AmlAudioEffectAmlPeq(DroidAudioEffectSetting* pSetting);
    int32_t init() override;
};

class DroidAudioEffectSetting final: public DroidAudioDbDescriptor {
public:
    static DroidAudioEffectSetting* instance() {
        static DroidAudioEffectSetting instance;
        return &instance;
    }
    vector<uint8_t> getDefaultValue(const string& key) override;
    int32_t init();
    int32_t deinit();
    int32_t reset();
    int32_t setAudioEffectEnabled(int32_t effectId, bool enable);
    bool isAudioEffectEnabled(int32_t effectId);
    int32_t setParameter(int32_t effectId, const vector<uint8_t>& param, const vector<uint8_t>& value);
    int32_t getParameter(int32_t effectId, const vector<uint8_t>& param, vector<uint8_t>* pValue);

    int32_t setBasicEffectEnabled(bool enable);
    bool isBasicEffectEnabled() {
        return mbBasicEffectEnabled;
    }
    int32_t initDualEffectMode();
    int32_t setDualEffectMode(int32_t mode);
    int32_t getDualEffectMode();
    int32_t getEffectFunctionConfig(int32_t id);

#define AML_GET_EFFECT_AND_LOCK(T, id, ret)                                                                     \
    unique_lock<mutex> demux_l(mAmlEffectMutex[id]);                                                            \
    auto effect = getEffect<T>(id);                                                                             \
    R_CHECK_POINTER_LEGAL(ret, effect, ", %s effect was not created!", mAmlEffectName[id].c_str())

    int32_t setBalance(int32_t step) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectBalance, DroidAudioEffect::EFFECT_ID_BALANCE, -1)
        return effect->setBalance(step);
    };
    int32_t getBalance() {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectBalance, DroidAudioEffect::EFFECT_ID_BALANCE, -1)
        return effect->getBalance();
    }

    int32_t setTreble(int32_t step) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectTrebleBass, DroidAudioEffect::EFFECT_ID_TREBLEBASS, -1)
        return effect->setTreble(step);
    };
    int32_t getTreble() {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectTrebleBass, DroidAudioEffect::EFFECT_ID_TREBLEBASS, -1)
        return effect->getTreble();
    }
    int32_t setBass(int32_t step) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectTrebleBass, DroidAudioEffect::EFFECT_ID_TREBLEBASS, -1)
        return effect->setBass(step);
    };
    int32_t getBass() {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectTrebleBass, DroidAudioEffect::EFFECT_ID_TREBLEBASS, -1)
        return effect->getBass();
    }

    int32_t setDapParam(int32_t id, int32_t value) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectDap, DroidAudioEffect::EFFECT_ID_DAP, -1)
        return effect->setDapParam(id, value);
    };
    int32_t getDapParam(int32_t id) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectDap, DroidAudioEffect::EFFECT_ID_DAP, -1)
        return effect->getDapParam(id);
    }

    int32_t setDpeParam(int32_t id, int32_t value) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectDpe, DroidAudioEffect::EFFECT_ID_DPE, -1)
        return effect->setDpeParam(id, value);
    };
    int32_t getDpeParam(int32_t id) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectDpe, DroidAudioEffect::EFFECT_ID_DPE, -1)
        return effect->getDpeParam(id);
    }

    int32_t setSoundMode(int32_t mode);
    int32_t getSoundMode() {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectHpeq, DroidAudioEffect::EFFECT_ID_HPEQ, -1)
        return effect->getSoundMode();
    }
    int32_t setUserSoundModeParam(int32_t bandNumber, int32_t value, int32_t bandSum) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectHpeq, DroidAudioEffect::EFFECT_ID_HPEQ, -1)
        return effect->setUserSoundModeParam(bandNumber, value, bandSum);
    };
    int32_t getUserSoundModeParam(int32_t bandNumber) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectHpeq, DroidAudioEffect::EFFECT_ID_HPEQ, -1)
        return effect->getUserSoundModeParam(bandNumber);
    }
    int32_t setHpeqBandNum(int32_t num) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectHpeq, DroidAudioEffect::EFFECT_ID_HPEQ, -1)
        return effect->setHpeqBandNum(num);
    };
    int32_t getHpeqBandNum();
    int32_t setVirtualSurroundEnabled(bool enable) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualSurround, DroidAudioEffect::EFFECT_ID_VIRTUALSURROUND, -1)
        return effect->setVirtualSurroundEnabled(enable);
    };
    bool isVirtualSurroundEnabled() {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualSurround, DroidAudioEffect::EFFECT_ID_VIRTUALSURROUND, -1)
        return effect->isVirtualSurroundEnabled();
    }

    bool isDtsVXValidEnabled() {
        return mbSupportVirtualX;
    }

    int32_t setDtsVirtualXEnabled(bool enable) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualX, DroidAudioEffect::EFFECT_ID_VIRTUALX, -1)
        return effect->setDtsVirtualXEnabled(enable);
    };
    bool isDtsVirtualXEnabled() {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualX, DroidAudioEffect::EFFECT_ID_VIRTUALX, -1)
        return effect->isDtsVirtualXEnabled();
    }
    int32_t setDtsVirtualSurroundEnabled(bool enable) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualX, DroidAudioEffect::EFFECT_ID_VIRTUALX, -1)
        return effect->setDtsVirtualSurroundEnabled(enable);
    };
    bool isDtsVirtualSurroundEnabled() {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualX, DroidAudioEffect::EFFECT_ID_VIRTUALX, -1)
        return effect->isDtsVirtualSurroundEnabled();
    }
    int32_t setDtsBassEnhancementEnabled(bool enable) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualX, DroidAudioEffect::EFFECT_ID_VIRTUALX, -1)
        return effect->setDtsBassEnhancementEnabled(enable);
    };
    bool isDtsBassEnhancementEnabled() {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualX, DroidAudioEffect::EFFECT_ID_VIRTUALX, -1)
        return effect->isDtsBassEnhancementEnabled();
    }
    int32_t setDtsDialogClarityMode(int32_t mode) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualX, DroidAudioEffect::EFFECT_ID_VIRTUALX, -1)
        return effect->setDtsDialogClarityMode(mode);
    };
    int32_t getDtsDialogClarityMode() {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualX, DroidAudioEffect::EFFECT_ID_VIRTUALX, -1)
        return effect->getDtsDialogClarityMode();
    }
    int32_t setDtsVirtualXMode(int32_t virtualXMode) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualX, DroidAudioEffect::EFFECT_ID_VIRTUALX, -1)
        return effect->setDtsVirtualXMode(virtualXMode);
    };
    int32_t getDtsVirtualXMode() {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualX, DroidAudioEffect::EFFECT_ID_VIRTUALX, -1)
        return effect->getDtsVirtualXMode();
    }
    int32_t setDtsTruVolumeHdEnabled(bool enable) {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualX, DroidAudioEffect::EFFECT_ID_VIRTUALX, -1)
        return effect->setDtsTruVolumeHdEnabled(enable);
    };
    bool isDtsTruVolumeHdEnabled() {
        AML_GET_EFFECT_AND_LOCK(AmlAudioEffectVirtualX, DroidAudioEffect::EFFECT_ID_VIRTUALX, false)
        return effect->isDtsTruVolumeHdEnabled();
    }

    // AiAQ Control APIs
    int32_t setAISoundModeEnable(bool enable);
    bool isAISoundModeEnabled();
    int32_t localUeventProcess(const std::string& msg);
    static int32_t nativeUeventHandle(void *owner, std::string msg);

    void reloadAudio();
    int32_t dump(int32_t fd, const char **args, uint32_t numArgs);

private:
    DroidAudioEffectSetting();
    virtual ~DroidAudioEffectSetting() {};
    int32_t effectConfigId2EffectId(int32_t configId);
    int32_t readAudioConfigFromHal();
    int32_t initBasicEffectMode();
    int32_t getOnBasicEffectCount();
    sp<AmlAudioEffect> createAudioEffect(int32_t index);
    int32_t destroyAudioEffect(int32_t id);
    sp<AmlAudioEffect> createAndInitEffect(int32_t id);
    template <typename T> T* getEffect(int32_t id) {
        AmlAudioEffect* pBase = mAmlAudioEffect[id].get();
//        R_CHECK_POINTER_LEGAL(nullptr, pBase, "effect:%s was not created", mAmlEffectName[id].c_str());
        T* effect = static_cast<T*>(pBase);
        return effect;
    }

    //AI Sound mode/dynamic sound mode control
    class AiRcLabel {
        public:
            enum class AiRcLabelType {
                SPORTS = 0,
                NEWS,
                TV_MOVIES,
                MUSIC_CLASSICAL,
                MUSIC_POP,
                MUSIC_ROCK,
                MUSIC_FOLK,
                MUSIC_JAZZ,
                MUSIC_HIPHOP,
                MUSIC_METAL,
                INVALID = -100
            };

            static constexpr float FLOAT_INT16_CONVERSION_FACTOR = 32765.0f;

            static int toLabel(int labelAndValue) {
                return labelAndValue & 0x000000FF;
            }

            static float toScore(int labelAndValue) {
                int scoreInt = (labelAndValue >> 16);
                return static_cast<float>(scoreInt) / FLOAT_INT16_CONVERSION_FACTOR;
            }

            static int ueventMsgToAiLabel(const std::string &msg);

            int mLabel;
            float mScore;

            AiRcLabel() : mLabel(static_cast<int>(AiRcLabelType::TV_MOVIES)), mScore(0.0f) {}
            AiRcLabel(int label, float score) : mLabel(label), mScore(score) {}

            int toSoundMode() const {
                switch (static_cast<AiRcLabelType>(mLabel)) {
                    case AiRcLabelType::SPORTS:        return DroidAudioEffect::COMMON_SOUND_MODE_GAME;
                    case AiRcLabelType::NEWS:          return DroidAudioEffect::COMMON_SOUND_MODE_NEWS;
                    case AiRcLabelType::TV_MOVIES:     return DroidAudioEffect::COMMON_SOUND_MODE_MOVIE;
                    case AiRcLabelType::MUSIC_CLASSICAL:
                    case AiRcLabelType::MUSIC_POP:
                    case AiRcLabelType::MUSIC_ROCK:
                    case AiRcLabelType::MUSIC_FOLK:
                    case AiRcLabelType::MUSIC_JAZZ:
                    case AiRcLabelType::MUSIC_HIPHOP:
                    case AiRcLabelType::MUSIC_METAL:
                        return DroidAudioEffect::COMMON_SOUND_MODE_MUSIC;
                    default:
                        return DroidAudioEffect::COMMON_SOUND_MODE_STANDARD;
                }
            }

            static std::string toString(int label) {
                static const std::unordered_map<int, std::string> labelMap = {
                    {static_cast<int>(AiRcLabelType::SPORTS), "sport"},
                    {static_cast<int>(AiRcLabelType::NEWS), "news"},
                    {static_cast<int>(AiRcLabelType::TV_MOVIES), "movies"},
                    {static_cast<int>(AiRcLabelType::MUSIC_CLASSICAL), "music_classical"},
                    {static_cast<int>(AiRcLabelType::MUSIC_POP), "music_pop"},
                    {static_cast<int>(AiRcLabelType::MUSIC_ROCK), "music_rock"},
                    {static_cast<int>(AiRcLabelType::MUSIC_FOLK), "music_folk"},
                    {static_cast<int>(AiRcLabelType::MUSIC_JAZZ), "music_jazz"},
                    {static_cast<int>(AiRcLabelType::MUSIC_HIPHOP), "music_hiphop"},
                    {static_cast<int>(AiRcLabelType::MUSIC_METAL), "music_metal"}
                };
                auto it = labelMap.find(label);
                return it != labelMap.end() ? it->second : "Unknown";
            }

            bool isSoundModeChanged(const AiRcLabel& newLabel) const {
                bool labelChanged = (mLabel != newLabel.mLabel);
                bool scoreCondition = (newLabel.mScore > 0.50f || newLabel.mScore > mScore);
                return labelChanged && scoreCondition;
            }

            static bool isValid(int label) {
                return (label >= static_cast<int>(AiRcLabelType::SPORTS) &&
                        label <= static_cast<int>(AiRcLabelType::MUSIC_METAL)) ||
                    label == static_cast<int>(AiRcLabelType::INVALID);
            }
    };

    AiRcLabel mAISoundLabel;
    bool mAISoundModeEnabled;
    //

    array<sp<AmlAudioEffect>, DroidAudioEffect::EFFECT_ID_MAX + 1>  mAmlAudioEffect;
    array<mutex, DroidAudioEffect::EFFECT_ID_MAX + 1>               mAmlEffectMutex;
    array<const string, DroidAudioEffect::EFFECT_ID_MAX + 1>        mAmlEffectName;

    bool                                                mbSupportVirtualX = false;
    bool                                                mbEffectInit = false;
    bool                                                mbBasicEffectEnabled = false;
    int32_t                                             mDualEffectMode;
    mutex                                               mMutex;

    int32_t                                             mDolbyMS12AudioConfig = DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_Y;
    int32_t                                             mDtsVirtualxAudioConfig = DroidAudioEffect::EFFECT_CONFIG_OFF;
    int32_t                                             mEffectBalanceAudioConfig = DroidAudioEffect::EFFECT_CONFIG_OFF;
    int32_t                                             mEffectTrebleBassAudioConfig = DroidAudioEffect::EFFECT_CONFIG_OFF;
    int32_t                                             mEffectVirtualSurroundAudioConfig = DroidAudioEffect::EFFECT_CONFIG_OFF;
    int32_t                                             mEffectDPEAudioConfig = DroidAudioEffect::EFFECT_CONFIG_OFF;
    int32_t                                             mEffectAmlPeqAudioConfig = DroidAudioEffect::EFFECT_CONFIG_OFF;
    int32_t                                             mEffectEQAudioConfig = 0;
    int32_t                                             mEffectDolbyDRCAudioConfig = 0;
    int32_t                                             mEffectDtsDRCAudioConfig = 0;
    int32_t                                             mAudioLatencyConfig = 0;
    int32_t                                             mForceDDPConfig = 0;
    int32_t                                             mEffectEngineerModeConfig = 0;
    int32_t                                             mPassthroughConfig = 0;
    int32_t                                             mAiDeConfig = 0;
    int32_t                                             mAiAQConfig = 0;
    int32_t                                             mAiVolumeEqConfig = 0;
    int32_t                                             mOttMs12Config = 0;
    int32_t                                             mGlobalMicDeviceTypeConfig = 0;
};

