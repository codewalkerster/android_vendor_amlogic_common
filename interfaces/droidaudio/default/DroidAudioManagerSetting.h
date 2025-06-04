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

#include <unordered_map>

#include <system/audio.h>
#include <system/audio_policy.h>
#include <media/AudioSystem.h>


using namespace std;
using namespace android;
class DroidAudioDbDescriptor;

class DroidAudioManagerSetting final: public DroidAudioDbDescriptor {
public:
    static DroidAudioManagerSetting* instance();
    vector<uint8_t> getDefaultValue(const string& key) override;
    int32_t init(bool reset = false);
    void reloadAudio();
    void audioPortOrPatchUpdate();
    int32_t dump(int fd, const char **args, uint32_t numArgs);

    int32_t reset();
    int32_t setAudioCmdParam(int32_t cmd, int32_t param1, int32_t param2, int32_t param3);
    int32_t setOutputDevices(const vector<int32_t>& devices);
    int32_t getOutputDevices(vector<int32_t>* devices);
    int32_t setCoexistSpdifOtherEnabled(bool enable);
    bool isCoexistSpdifOtherEnabled();
    int32_t setSoundBarModeEnabled(bool enable);
    bool isSoundBarModeEnabled();
    int32_t setSoundSpdifEnabled(bool enable);
    bool isSoundSpdifEnabled();
    int32_t setSpeakerEnabled(bool enable);
    bool isSpeakerEnabled();
    int32_t setOutputDeviceDelay(int32_t source, int32_t device, int32_t delayMs);
    int32_t getOutputDeviceDelay(int32_t source, int32_t device);
    int32_t setAudioOutputAllDelay(int32_t delayMs);
    int32_t getAudioOutputAllDelay();
    int32_t setTvSourceType(int32_t source);
    int32_t getTvSourceType();
    int32_t setAudioApplyToAll();
    int32_t setDigitalAudioMode(int32_t mode, const string& formats);
    int32_t getDigitalAudioMode();
    int32_t setForceDDPEnabled(bool enable);
    bool isForceDDPEnabled();
    int32_t setDolbyDrcMode(int32_t mode);
    int32_t getDolbyDrcMode();
    int32_t setDolbyDrcLineLevel(int32_t level);
    int32_t getDolbyDrcLineLevel();
    bool isDtsXEnabled();
    int32_t setDtsXDrcEnabled(bool enable);
    bool isDtsXDrcEnabled();
    int32_t setDialogEnhancerLevel(int32_t level);
    int32_t getDialogEnhancerLevel();
    int32_t setSoundDmxMode(int32_t mode);
    int32_t getSoundDmxMode();
    int32_t setSoundLevelerMode(int mode);
    int32_t getSoundLevelerMode();
    int32_t setSoundLevelerAmount(int value);
    int32_t getSoundLevelerAmount();
    int32_t setVadEnabled(bool enable);
    bool isVadEnabled();
    int32_t openTvAudio(int32_t source);
    int32_t getDroidAudioConfig(int32_t id);

    int32_t setParameters(const string& keyValuePairs);
    string getParameters(const string& keys);
    int32_t createAudioPatch(int32_t sourceDevice, int32_t sinkDevice);
    int32_t releaseAudioPatch(int32_t handle);

    int32_t setAiDeEnabled(bool enable);
    bool isAiDeEnabled();
    int32_t setAiDeGain(int32_t value);
    int32_t getAiDeGain();

    int32_t setGlobalMicEnable(int32_t source, bool enable);
    bool getGlobalMicStatus(int32_t source);
    int32_t setMicSource(int32_t source);
    int32_t getMicSource();
    int32_t setMicMute(int32_t source, bool mute);
    bool isMicMute(int32_t source);
    int32_t setMicGain(int32_t source, int32_t gain);
    int32_t getMicGain(int32_t source);
    int32_t setMicReverb(int32_t source, bool enable);
    bool isEnableMicReverb(int32_t source);
    int32_t setMicReverbLevel(int32_t source, int32_t level);
    int32_t getMicReverbLevel(int32_t source);

private:
    DroidAudioManagerSetting();
    int32_t updateCoexistSpdifOther();
    void setDolbyDrcEnabled(bool enable);
    void setDolbyMode(int32_t mode);
    int getAudioFunctionConfig(int id);
    void resetMicSettings();

    bool                                    mInitStatus = false;
    int32_t                                 mCurTvSource;
};

inline DroidAudioManagerSetting* DroidAudioManagerSetting::instance() {
    static DroidAudioManagerSetting instance;
    return &instance;
}

