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

#define ENUM_TYPE_STR_MAX_LEN                           (256)

#define ENUM_TYPE_TO_STR_DEFAULT_STR            "INVALID_ENUM"
#define ENUM_TYPE_TO_STR_START(prefix)                      \
    const char *pStr = ENUM_TYPE_TO_STR_DEFAULT_STR;        \
    int32_t prefixLen = strlen(prefix);                     \
    switch (type) {
#define ENUM_TYPE_TO_STR(x)                                 \
    case x:                                                 \
        pStr = #x;                                          \
        pStr += prefixLen;                                  \
        if (strlen(#x) - prefixLen > 128) {                 \
            pStr += 128;                                    \
        }                                                   \
        break;
#define ENUM_TYPE_TO_STR_END                                \
    default:                                                \
        break;                                              \
    }                                                       \
    return pStr;

#define R_CHECK_RET(ret, fmt, ...)                                                              \
    if (ret != 0) {                                                                             \
        AM_LOGE("ret:%d " fmt, ret, ##__VA_ARGS__);                                             \
        return ret;                                                                             \
    }

#define NO_R_CHECK_RET(ret, fmt, ...)                                                           \
    if (ret != 0) {                                                                             \
        AM_LOGE("ret:%d " fmt, ret, ##__VA_ARGS__);                                             \
    }

#define R_CHECK_PARAM_LEGAL(ret, param, min, max, fmt, ...)                                     \
    if ((int32_t)param < min || param > max) {                                                  \
        AM_LOGE("%s:%d is illegal, min:%d, max:%d " fmt, #param, param, min, max, ##__VA_ARGS__);\
        return ret;                                                                             \
    }

#define R_CHECK_POINTER_LEGAL(ret, pointer, fmt, ...)                                           \
    if (pointer == nullptr) {                                                                   \
        AM_LOGE("%s is null pointer " fmt, #pointer, ##__VA_ARGS__);                            \
        return ret;                                                                             \
    }


#define AM_LOGV(fmt, ...)  ALOGV("[%s:%d] " fmt, __func__,__LINE__, ##__VA_ARGS__)
#define AM_LOGD(fmt, ...)  ALOGD("[%s:%d] " fmt, __func__,__LINE__, ##__VA_ARGS__)
#define AM_LOGI(fmt, ...)  ALOGI("[%s:%d] " fmt, __func__,__LINE__, ##__VA_ARGS__)
#define AM_LOGW(fmt, ...)  ALOGW("[%s:%d] " fmt, __func__,__LINE__, ##__VA_ARGS__)
#define AM_LOGE(fmt, ...)  ALOGE("[%s:%d] " fmt, __func__,__LINE__, ##__VA_ARGS__)

#define CHECK_AIDL_STATUS(ret, status, fmt, ...)                                                \
    if (!status.isOk()) {                                                                       \
        AM_LOGE("call aidl failed! " fmt, ##__VA_ARGS__);                                       \
        return ret;                                                                             \
    }


