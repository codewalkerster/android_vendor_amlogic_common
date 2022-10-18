/*
 * Copyright (C) 2011 The Android Open Source Project
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
 *  @author   Tellen Yu
 *  @version  2.0
 *  @date     2014/09/09
 *  @par function description:
 *  - 1 write property or sysfs in daemon
 */

#define LOG_TAG "SystemControl"

//#define LOG_NDEBUG 0
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/properties.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#ifndef RECOVERY_MODE
#include "../../../../provision/ca/include/provision_api.h"
#include <getopt.h>
#include <sys/stat.h>
//#include <tee_client_api.h>
//#include <ta.h>
#endif

#include "common.h"
#include "ProvisionKey.h"

ProvisionKey::ProvisionKey() {
    mLogLevel = LOG_LEVEL_DEFAULT;
}

ProvisionKey::~ProvisionKey() {
}

bool ProvisionKey::getProperty(const char *key, char *value){
    property_get(key, value, "");
    /*
    char buf[PROPERTY_VALUE_MAX] = {0};
    property_get(key, buf, "");
    value.setTo(String16(buf));
    */
    return true;
}

bool ProvisionKey::getPropertyString(const char *key, char *value,  const char *def){
    property_get(key, value, def);
    return true;
}

int32_t ProvisionKey::getPropertyInt(const char *key, int32_t def){
    int len;
    char* end;
    char buf[PROPERTY_VALUE_MAX] = {0};
    int32_t result = def;

    len = property_get(key, buf, "");
    if (len > 0) {
        result = strtol(buf, &end, 0);
        if (end == buf) {
            result = def;
        }
    }

    return result;
}

int64_t ProvisionKey::getPropertyLong(const char *key, int64_t def){

    int len;
    char buf[PROPERTY_VALUE_MAX] = {0};
    char* end;
    int64_t result = def;

    len = property_get(key, buf, "");
    if (len > 0) {
        result = strtoll(buf, &end, 0);
        if (end == buf) {
            result = def;
        }
    }

    return result;
}

bool ProvisionKey::getPropertyBoolean(const char *key, bool def){

    int len;
    char buf[PROPERTY_VALUE_MAX] = {0};
    bool result = def;

    len = property_get(key, buf, "");
    if (len == 1) {
        char ch = buf[0];
        if (ch == '0' || ch == 'n')
            result = false;
        else if (ch == '1' || ch == 'y')
            result = true;
    } else if (len > 1) {
         if (!strcmp(buf, "no") || !strcmp(buf, "false") || !strcmp(buf, "off")) {
            result = false;
        } else if (!strcmp(buf, "yes") || !strcmp(buf, "true") || !strcmp(buf, "on")) {
            result = true;
        }
    }

    return result;
}

void ProvisionKey::setProperty(const char *key, const char *value){
    int err;
    err = property_set(key, value);
    if (err < 0) {
        SYS_LOGE("failed to set system property %s\n", key);
    }
}


//key start
bool ProvisionKey::writeProvisionKey(const char *value, const int size) {
    bool ret = false;
    #ifndef RECOVERY_MODE
    ret = keyProvisionStore(value, size);
    #endif
    return ret;
}

bool ProvisionKey::checkProvisionKey(const uint32_t key_type) {
    bool ret = false;
    #ifndef RECOVERY_MODE
        ret = keyProvisionQuery(key_type, 0);
    #endif
    SYS_LOGI("[%s, %d] ret:%d", __FUNCTION__, __LINE__,ret);
    return ret;
}

bool ProvisionKey::calcChecksumKey(const char *value, const int size, char *keyCheckSum){
    #ifndef RECOVERY_MODE
        return keyProvisionCalcChecksum(value, size, keyCheckSum);
    #endif
    return false;
}

bool ProvisionKey::getKeyProvisionChecksum(int type, char *keyCheckSum){
    bool ret = false;
    char buf[PROVISION_KEY_CHECKSUM_LENGTH+1] = {0};
    #ifndef RECOVERY_MODE
        ret = keyProvisionChecksum(type, buf);
    #endif
    if (ret) {
        char *pTmp = keyCheckSum;
        for (int i=0; i < PROVISION_KEY_CHECKSUM_LENGTH; i++) {
            sprintf(pTmp, "%02X", (unsigned char)buf[i]);
            pTmp+=2;
        }
        return true;
    }
    return false;
}

bool ProvisionKey::deleteProvisionKey(const uint32_t key_type){
    bool ret = false;
    #ifndef RECOVERY_MODE
        ret = keyProvisionDelete(key_type);
    #endif

    return ret;
}

bool ProvisionKey::deleteProvisionKeyEx(const uint32_t key_type, const char *uuid) {
    int ret = -1;
    #ifndef RECOVERY_MODE
    int key_size = 0;
    ret = key_provision_delete(key_type, (const uint8_t *)uuid);
    SYS_LOGI("key_provision_delete key_type: 0x%x uuid:%s ret:0x%x \n", key_type, uuid, ret);
    if (ret == 0) {
        return true;
    }
    #endif
    return false;
}
//key end

void ProvisionKey::setLogLevel(int level){
    mLogLevel = level;
}

bool ProvisionKey::keyProvisionStore(const char *value, const int size) {
    SYS_LOGI("keyProvisionStore key_size: %d\n", size);
    int ret = -1;
    #ifndef RECOVERY_MODE
        ret = key_provision_store(NULL, 0, (uint8_t*)value, (uint32_t)size);
    #endif
    SYS_LOGI("keyProvisionStore ret = %d, %08X\n", ret, ret);
    if (ret == 0)
        return true;
    else
        return false;
}

bool ProvisionKey::keyProvisionQuery (const uint32_t key_type, const int size) {
    uint32_t key_size = size;
    int ret = -1;
    SYS_LOGI("keyProvisionQuery key_type: %d default_storage_location: %d key_size: %d\n", key_type,default_storage_location,size);
    #ifndef RECOVERY_MODE
        ret = key_provision_query(NULL, 0, key_type, &default_storage_location, &key_size);
    #endif
    SYS_LOGI("keyProvisionQuery ret: %d\n",ret);
    if (ret == 0)
        return true;
    else
        return false;
}

bool ProvisionKey::keyProvisionChecksum (const uint32_t key_type, const char *value) {
    SYS_LOGI("keyProvisionChecksum key_type: 0x%08x \n", key_type);
    int ret = -1;
    #ifndef RECOVERY_MODE
        ret = key_provision_checksum(key_type, NULL, 0, (uint8_t*)value);
    #endif
    SYS_LOGI("keyProvisionChecksum ret = %d, %08X\n", ret, ret);
    if (ret == 0)
        return true;
    else
        return false;
}

bool ProvisionKey::keyProvisionDelete (const uint32_t key_type) {
    int ret = -1;
    #ifndef RECOVERY_MODE
        SYS_LOGI("keyProvisionDelete key_type: %d \n", key_type);
        ret = key_provision_delete(key_type, NULL);
    #endif
    if (ret == 0)
        return true;
    else
        return false;
}

bool ProvisionKey::keyProvisionQueryV2 (const uint32_t ext_key_type, const int size) {
    uint32_t key_size = size;
    int ret = -1;
    SYS_LOGI("keyProvisionQueryV2 ext_key_type: %d default_storage_location: %d key_size: %d\n", ext_key_type,default_storage_location,size);
    #ifndef RECOVERY_MODE
        ret = key_provision_query_v2(NULL, 0, ext_key_type, (uint8_t *)default_ext_ta_uuid, &default_storage_location, &key_size);
    #endif
    if (ret == 0)
        return true;
    else
        return false;
}

bool ProvisionKey::keyProvisionChecksumV2 (const char *value, const uint32_t ext_key_type) {
    int ret = -1;
    #ifndef RECOVERY_MODE
        SYS_LOGI("keyProvisionChecksumV2 ext_key_type: %d checksum: %s \n", ext_key_type,value);
        ret = key_provision_checksum_v2(ext_key_type, NULL, 0, (uint8_t *)default_ext_ta_uuid, (uint8_t*)value);
    #endif
    if (ret == 0)
        return true;
    else
        return false;
}

bool ProvisionKey::keyProvisionDeleteV2 (const uint32_t ext_key_type) {
    int ret = -1;
    #ifndef RECOVERY_MODE
        SYS_LOGI("keyProvisionDeleteV2 ext_key_type: %d \n", ext_key_type);
        ret = key_provision_delete(ext_key_type, (uint8_t *)default_ext_ta_uuid); // delete key
    #endif
    if (ret == 0)
        return true;
    else
        return false;
}

bool ProvisionKey::keyProvisionGetPfid () {
    int ret = -1;
    SYS_LOGI("keyProvisionGetPfid default_pfid_buf: %s default_pfid_buf: %d \n", default_pfid_buf,default_id_size);
    #ifndef RECOVERY_MODE
        ret = key_provision_get_pfid(default_pfid_buf, &default_id_size);
    #endif
    if (ret == 0)
        return true;
    else
        return false;
}

bool ProvisionKey::keyProvisionGetDac () {
    int ret = -1;
    SYS_LOGI("keyProvisionGetDac default_dac_buf: %s default_dac_size: %d \n", default_dac_buf,default_dac_size);
    #ifndef RECOVERY_MODE
        ret = key_provision_get_dac(default_dac_buf, &default_dac_size);
    #endif
    if (ret == 0)
        return true;
    else
        return false;
}

bool ProvisionKey::keyProvisionCalcChecksum (const char *value, const int size, char *keyCheckSum) {
    uint32_t key_size = size;
    char buf[PROVISION_KEY_CHECKSUM_LENGTH+1] = {0};
    int ret = -1;
    #ifndef RECOVERY_MODE
        SYS_LOGI("keyProvisionCalcChecksum value: %s key_size: %d \n",value, key_size);
        ret = key_provision_calc_checksum((uint8_t*)value, key_size, (uint8_t*)buf);
        strcpy(keyCheckSum, buf);
    #endif
    if (ret == 0)
        return true;
    else
        return false;
}

int ProvisionKey::getKernelReleaseVersion() {
    int major = 4;
    int minor = 9;
    struct utsname uts;

    if (uname(&uts) == -1) {
        return major;
    }

    if (sscanf(uts.release, "%d.%d", &major, &minor) != 2) {
        return major;
    }

    SYS_LOGI("getKernelReleaseVersion: %d.%d\n", major,minor);
    return major;
}

#if 0
status_t ProvisionKey::dump(int fd, const Vector<String16>& args){
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    if (checkCallingPermission(String16("android.permission.DUMP")) == false) {
        snprintf(buffer, SIZE, "Permission Denial: "
                "can't dump sys.write from pid=%d, uid=%d\n",
                IPCThreadState::self()->getCallingPid(),
                IPCThreadState::self()->getCallingUid());
        result.append(buffer);
    } else {
        Mutex::Autolock lock(mLock);

        result.appendFormat("sys write service wrote by multi-user mode, normal process will have not system privilege\n");
        /*
        int n = args.size();
        for (int i = 0; i + 1 < n; i++) {
            String16 verboseOption("-v");
            if (args[i] == verboseOption) {
                String8 levelStr(args[i+1]);
                int level = atoi(levelStr.string());
                result = String8::format("\nSetting log level to %d.\n", level);
                setLogLevel(level);
                write(fd, result.string(), result.size());
            }
        }*/
    }
    write(fd, result.string(), result.size());
    return NO_ERROR;
}
#endif
