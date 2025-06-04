/*
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

package vendor.amlogic.hardware.droidaudio;

import vendor.amlogic.hardware.droidaudio.Status;
import vendor.amlogic.hardware.droidaudio.IDroidAudioClient;

@VintfStability
interface IDroidAudio {
    // DroidAudioManager
    int registerClient(in IDroidAudioClient client);
    int AudioManager_reset();
    int AudioManager_setAudioCmdParam(int cmd, int param1, int param2, int param3);
    int AudioManager_setOutputDevices(in int[] devices);
    int[] AudioManager_getOutputDevices();
    int AudioManager_setCoexistSpdifOtherEnabled(boolean enable);
    boolean AudioManager_isCoexistSpdifOtherEnabled();
    int AudioManager_setSoundBarModeEnabled(boolean enable);
    boolean AudioManager_isSoundBarModeEnabled();
    int AudioManager_setSoundSpdifEnabled(boolean enable);
    boolean AudioManager_isSoundSpdifEnabled();
    int AudioManager_setSpeakerEnabled(boolean enable);
    boolean AudioManager_isSpeakerEnabled();
    int AudioManager_setOutputDeviceDelay(int source, int device, int delayMs);
    int AudioManager_getOutputDeviceDelay(int source, int device);
    int AudioManager_setAudioOutputAllDelay(int delayMs);
    int AudioManager_getAudioOutputAllDelay();
    int AudioManager_setTvSourceType(int source);
    int AudioManager_getTvSourceType();
    int AudioManager_setAudioApplyToAll();
    int AudioManager_setDigitalAudioMode(int mode, String formats);
    int AudioManager_getDigitalAudioMode();
    int AudioManager_setForceDDPEnabled(boolean enable);
    boolean AudioManager_isForceDDPEnabled();
    int AudioManager_setDolbyDrcMode(int mode);
    int AudioManager_getDolbyDrcMode();
    int AudioManager_setDolbyDrcLineLevel(int level);
    int AudioManager_getDolbyDrcLineLevel();
    boolean AudioManager_isDtsXEnabled();
    int AudioManager_setDtsXDrcEnabled(boolean enable);
    boolean AudioManager_isDtsXDrcEnabled();
    int AudioManager_setDialogEnhancerLevel(int level);
    int AudioManager_getDialogEnhancerLevel();
    int AudioManager_setSoundDmxMode(int mode);
    int AudioManager_getSoundDmxMode();
    int AudioManager_setSoundLevelerMode(int mode);
    int AudioManager_getSoundLevelerMode();
    int AudioManager_setSoundLevelerAmount(int value);
    int AudioManager_getSoundLevelerAmount();
    int AudioManager_setVadEnabled(boolean enable);
    boolean AudioManager_isVadEnabled();
    int AudioManager_openTvAudio(int source);
    int AudioManager_getDroidAudioConfig(int id);
    int AudioManager_setParameters(String keyValuePairs);
    String AudioManager_getParameters(String keys);
    int AudioManager_createAudioPatch(int sourceDevice, int sinkDevice);
    int AudioManager_releaseAudioPatch(int handle);
    int AudioManager_setAiDeEnabled(boolean enable);
    boolean AudioManager_isAiDeEnabled();
    int AudioManager_setAiDeGain(int gain);
    int AudioManager_getAiDeGain();
    int AudioManager_setGlobalMicEnable(int source, boolean enable);
    boolean AudioManager_getGlobalMicStatus(int source);
    int AudioManager_setMicSource(int source);
    int AudioManager_getMicSource();
    int AudioManager_setMicMute(int source, boolean mute);
    boolean AudioManager_isMicMute(int source);
    int AudioManager_setMicGain(int source, int gain);
    int AudioManager_getMicGain(int source);
    int AudioManager_setMicReverb(int source, boolean enable);
    boolean AudioManager_isEnableMicReverb(int source);
    int AudioManager_setMicReverbLevel(int source, int level);
    int AudioManager_getMicReverbLevel(int source);

    // DroidAudioEffect
    int AudioEffect_setAudioEffectEnabled(int effectId, boolean enable);
    boolean AudioEffect_isAudioEffectEnabled(int effectId);
    int AudioEffect_setParameter(int effectId, in byte[] param, in byte[] value);
    byte[] AudioEffect_getParameter(int effectId, in byte[] param);
    int AudioEffect_setBasicEffectEnabled(boolean enable);
    boolean AudioEffect_isBasicEffectEnabled();
    int AudioEffect_initDualEffectMode();
    int AudioEffect_setDualEffectMode(int mode);
    int AudioEffect_getDualEffectMode();
    int AudioEffect_getEffectFunctionConfig(int id);
    int AudioEffect_setBalance(int step);
    int AudioEffect_getBalance();
    int AudioEffect_setTreble(int step);
    int AudioEffect_getTreble();
    int AudioEffect_setBass(int step);
    int AudioEffect_getBass();
    int AudioEffect_setDapParam(int id, int value);
    int AudioEffect_getDapParam(int id);
    int AudioEffect_setDpeParam(int id, int value);
    int AudioEffect_getDpeParam(int id);
    int AudioEffect_setSoundMode(int mode);
    int AudioEffect_getSoundMode();
    int AudioEffect_setUserSoundModeParam(int bandNumber, int value, int bandSum);
    int AudioEffect_getUserSoundModeParam(int bandNumber);
    int AudioEffect_setHpeqBandNum(int num);
    int AudioEffect_getHpeqBandNum();
    int AudioEffect_setVirtualSurroundEnabled(boolean enable);
    boolean AudioEffect_isVirtualSurroundEnabled();
    boolean AudioEffect_isDtsVXValidEnabled();
    int AudioEffect_setDtsVirtualXEnabled(boolean enable);
    boolean AudioEffect_isDtsVirtualXEnabled();
    int AudioEffect_setDtsVirtualSurroundEnabled(boolean enable);
    boolean AudioEffect_isDtsVirtualSurroundEnabled();
    int AudioEffect_setDtsBassEnhancementEnabled(boolean enable);
    boolean AudioEffect_isDtsBassEnhancementEnabled();
    int AudioEffect_setDtsDialogClarityMode(int mode);
    int AudioEffect_getDtsDialogClarityMode();
    int AudioEffect_setDtsVirtualXMode(int virtualXMode);
    int AudioEffect_getDtsVirtualXMode();
    int AudioEffect_setDtsTruVolumeHdEnabled(boolean enable);
    boolean AudioEffect_isDtsTruVolumeHdEnabled();
    int AudioEffect_setAISoundModeEnable(boolean enable);
    boolean AudioEffect_isAISoundModeEnabled();

}
