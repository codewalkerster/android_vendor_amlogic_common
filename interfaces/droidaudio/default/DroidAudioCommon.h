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

#include <string>

using namespace std;
extern const char* INI_KEY_AM_AUDIO_COMMON_DRIVER_BASE_PROJECT;


void setParameters(const string &value);
void setParameters(const string &key, int32_t value);
string getParameters(const string &key);

bool getPropertyBoolean(const char *key, bool def);
bool isAudioDebug();
bool isTvPlatform();
bool isSoundbarPlatform();
bool isDriverBaseProject();
bool isSupportMs12();

