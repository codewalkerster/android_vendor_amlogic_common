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

#include <vector>
#include <shared_mutex>

#include <aidl/vendor/amlogic/hardware/droidaudio/IDroidAudio.h>

#include "DroidAudioClientUtils.h"

using aidl::vendor::amlogic::hardware::droidaudio::IDroidAudio;

using namespace std;

extern shared_mutex g_getDroidAudioServiceLock;

const shared_ptr<IDroidAudio> get_droidaudio_service();
template <typename ReturnType, typename Func, typename... Args>
ReturnType executeFunction(const char* funcName, Func func, Args... args) {
    ReturnType ret;

    // set the default return value
    if constexpr (is_same<ReturnType, int>::value) {
        ret = 0;
    } else if constexpr (is_same<ReturnType, bool>::value) {
        ret = false;
    } else if constexpr (is_same<ReturnType, string>::value) {
        ret = "";
    } else {
        ret = ReturnType{};
    }
    shared_lock<shared_mutex> _l(g_getDroidAudioServiceLock);
    auto droidaudio = get_droidaudio_service();
    R_CHECK_POINTER_LEGAL(ret, droidaudio, "%s: get IDroidAudio fail", funcName)
    auto status = (droidaudio.get()->*func)(args..., &ret);
    CHECK_AIDL_STATUS(ret, status, "%s", funcName)
    return ret;
}

#define AML_AIDL_EXECUTE_FUNCTION(ReturnType, func, ...) executeFunction<ReturnType>(#func, func, ##__VA_ARGS__)

