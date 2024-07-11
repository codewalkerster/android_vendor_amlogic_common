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
#include <vector>

#pragma once

using namespace std;

class DroidAudioManager
{
public:
    static int32_t setAudioCmdParam(int32_t cmd, int32_t param1, int32_t param2, int32_t param3);
    static int32_t setOutputDevices(const vector<int32_t>& devices);
    static int32_t getOutputDevices(vector<int32_t>* devices);
    static int32_t setCoexistSpdifOther(bool enable);
    static int32_t setMusicStreamVolume(int32_t index);
};

