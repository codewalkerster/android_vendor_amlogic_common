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

#include <stdint.h>
#include <sys/types.h>
#include <cutils/config_utils.h>
#include <cutils/misc.h>
#include <utils/Timers.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/SortedVector.h>
#include <media/AudioParameter.h>
#include <media/AudioPolicy.h>
#include <media/AudioProfile.h>
#include <media/PatchBuilder.h>
#include "AudioPolicyInterface.h"

#include <AudioPolicyManagerObserver.h>
#include <AudioPolicyConfig.h>
#include <PolicyAudioPort.h>
#include <DeviceDescriptor.h>
#include "AudioPolicyManager.h"

namespace android {

class AmlAudioPolicyManager : public AudioPolicyManager
{
public:
    AmlAudioPolicyManager(const sp<const AudioPolicyConfig>& config,
                       EngineInstance&& engine,
                       AudioPolicyClientInterface *clientInterface);
    virtual ~AmlAudioPolicyManager() {};

    virtual void setForceUse(audio_policy_force_use_t usage,
                             audio_policy_forced_cfg_t config);
    virtual status_t setDeviceConnectionState(audio_policy_dev_state_t state,
            const android::media::audio::common::AudioPort& port, audio_format_t encodedFormat);

    virtual status_t checkAndSetVolume(IVolumeCurves &curves,
                                       VolumeSource volumeSource, int index,
                                       const sp<AudioOutputDescriptor>& outputDesc,
                                       DeviceTypeSet deviceTypes,
                                       int delayMs = 0, bool force = false);

    status_t dump(int fd) override;

private:
    bool isScoRequestedForComm() const;
    bool isHearingAidUsedForComm() const;

};

};
