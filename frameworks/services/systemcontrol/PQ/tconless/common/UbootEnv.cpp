/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "UbootEnv"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <utils/Log.h>
#include "UbootEnv.h"

UbootEnv::UbootEnv()
{
    sys_client = android::SystemControlClient::getInstance();
}

UbootEnv::~UbootEnv()
{
}

bool UbootEnv::write(const char *name, char *value)
{
    bool result = false;
    char env_name_buffer[1024] = {'\0'};

    memset(env_name_buffer, '\0', sizeof(env_name_buffer));
    if (strstr(name, "ubootenv.var.") == NULL) {
        ALOGI("%s, uboot_env_name does not include \"ubootenv.var.\" prefix, now add it!", __FUNCTION__);
        sprintf(env_name_buffer, "ubootenv.var.%s", name);
    } else {
        sprintf(env_name_buffer, "%s", name);
    }

    if (sys_client != nullptr) {
        sys_client->setBootEnv(env_name_buffer, value);
        result = true;
        ALOGI("%s, set [%s]=%s", __FUNCTION__, name, value);
    } else {
        ALOGE("%s, set [%s] to %s fail", __FUNCTION__, name, value);
    }

    return result;
}

bool UbootEnv::read(const char *name, char *value)
{
    std::string p_value;
    bool result = false;
    char env_name_buffer[1024] = {'\0'};

    memset(env_name_buffer, '\0', sizeof(env_name_buffer));
    if (strstr(name, "ubootenv.var.") == NULL) {
        ALOGI("%s, uboot_env_name does not include \"ubootenv.var.\" prefix, now add it!", __FUNCTION__);
        sprintf(env_name_buffer, "ubootenv.var.%s", name);
    } else {
        sprintf(env_name_buffer, "%s", name);
    }

    if (sys_client != nullptr) {
        result = sys_client->getBootEnv(env_name_buffer, p_value);
        if (result) {
            strcpy(value, p_value.c_str());
            ALOGI("%s, read [%s]=%s", __FUNCTION__, name, value);
        } else {
            ALOGE("%s, get %s failed!\n ", __FUNCTION__, name);
        }
    } else {
        ALOGI("%s, get [%s] fail", __FUNCTION__, name);
    }

    return result;
}

