/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "CTconPGamma"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <utils/Log.h>

#include "tcondef.h"
#include "CTconPGamma.h"
#include "PGammaDeviceHkc.h"
#include "PGammaDeviceCsot.h"


CTconPGamma *CTconPGamma::mInstance = NULL;
CTconPGamma *CTconPGamma::GetInstance()
{
    if (NULL == mInstance)
        mInstance = new CTconPGamma();
    return mInstance;
}

CTconPGamma::CTconPGamma():
    ukeyPGammaInfo(NULL),
    mUkeyInstance(NULL),
    device(NULL),
    plat(NULL)
{
    memset(pgammaFlashPath, 0, sizeof(pgammaFlashPath));
    memset(pmu_Path, 0, sizeof(pmu_Path));
    memset(&pmuInfo, 0, sizeof(pmuInfo));
}

CTconPGamma::~CTconPGamma() { }

int CTconPGamma::Init(char *pgammaPath, char *pmuPath)
{
    int ret = -1;
    if (pgammaPath && (!access(pgammaPath, F_OK))) {
        strncpy(pgammaFlashPath, pgammaPath, sizeof(pgammaFlashPath));
    } else {
        if (!access(LCD_TCON_SPI_FLASH_PATH_K54, F_OK))
            strncpy(pgammaFlashPath, LCD_TCON_SPI_FLASH_PATH_K54, sizeof(pgammaFlashPath));
        else if (!access(LCD_TCON_SPI_FLASH_PATH_K49, F_OK))
            strncpy(pgammaFlashPath, LCD_TCON_SPI_FLASH_PATH_K49, sizeof(pgammaFlashPath));
        else {
            ALOGE("No flash path...\n");
            goto __init_exit;
        }
    }
    ALOGD("Use PGamma Path: %s\n", pgammaFlashPath);

    mUkeyInstance = UKeyBlock::GetInstance();
    if ((!mUkeyInstance) || (mUkeyInstance->Init() < 0)) {
        ALOGE("Ukey init fail, exit...\n");
        goto __init_exit;
    }
    ukeyPGammaInfo = mUkeyInstance->getPgammaUkeyInfo();

    if (pmuPath) {
        strncpy(pmu_Path, pmuPath, sizeof(pmu_Path));
    } else {
        plat = PlatMisc::GetInstance();
        if (!plat || !plat->getPmuPath(pmu_Path, sizeof(pmu_Path))) {
            ALOGE("Pgamma Get pmu path fail!\n");
            goto __init_exit;
        }
    }

    if (GetPmuInfo(pmu_Path))
        ALOGD("Get pmu info from pmuPath:%s ok\n", pmu_Path);

    if (ukeyPGammaInfo && ukeyPGammaInfo->size != 0 && strlen(pgammaFlashPath) > 0) {
        if (PGammaDeviceHkc::detect(pgammaFlashPath)) {
            device = new PGammaDeviceHkc();
            ALOGD("PGamma hkc device detect.\n");
        } else if (PGammaDeviceCsot::detect(pgammaFlashPath)) {
            device = new PGammaDeviceCsot();
            ALOGD("PGamma csot device detect.\n");
        }
        if (!device) {
            ALOGE("PGamma dev alloc fail, exit...\n");
            goto __init_exit;
        }
        if (0 != device->Init(pgammaFlashPath, pmuInfo.buf, pmuInfo.len)) {
            ALOGE("PGamma dev init fail, exit...\n");
            goto __init_exit;
        }
    }

    ret = 0;

__init_exit:
    if (mUkeyInstance)
        mUkeyInstance->UnInit();
    return ret;
}

int CTconPGamma::GenerateBin(char *binPath)
{
    int ret = -1;
    int fd = -1;
    int i = 0, j = 0;
    unsigned short crc = 0;
    unsigned char *binbuf = NULL;
    PGammaBufferInfo_s *pgammaInfo = NULL;
    char realPath[128] = {0};

    if (!device)
        goto __gen_bin_exit;

    memset(realPath, 0, sizeof(realPath));

    if (!strcasecmp(binPath, "default")) {
        if (!plat)
            plat = PlatMisc::GetInstance();
        if (!plat || !plat->getPGammaPath(realPath, sizeof(realPath))) {
            ALOGE("Pgamma Get pmu path fail!\n");
            goto __gen_bin_exit;
        }
    } else {
        strncpy(realPath, binPath, sizeof(realPath));
    }
    // get flash pgamma info
    pgammaInfo = device->GenerateBuffer();
    if (!pgammaInfo) {
        ALOGE("Generate buffer fail\n");
        goto __gen_bin_exit;
    }

    // check and read bin files
    fd = open(realPath, O_RDWR|O_CREAT, 0664);
    if (fd < 0) {
        ALOGE("Open file(%s) error: %s\n", realPath, strerror(errno));
        goto __gen_bin_exit;
    }

    binbuf = (unsigned char *)calloc(1, pgammaInfo->size);
    if (!binbuf) {
        ALOGE("Alloc bin buffer fail!\n");
        goto __gen_bin_exit;
    }

    if (read(fd, binbuf, pgammaInfo->size) < 0) {
        ALOGE("Read file(%s) error: %s\n", realPath, strerror(errno));
    }

    // compare bin and flash gamma info
    if (memcmp(binbuf, pgammaInfo->buffer, pgammaInfo->size)) {
        // difference content, replace gamma info to bin
        if (write (fd, pgammaInfo->buffer, pgammaInfo->size) <= 0) {
            ALOGE("Write file(%s) error: %s\n", realPath, strerror(errno));
            goto __gen_bin_exit;
        }
    }

    ret = 0;

__gen_bin_exit:
    if (binbuf)
        free(binbuf);
    if (fd >= 0)
        close(fd);
    if (pgammaInfo)
        device->DestroyBuffer(pgammaInfo);
    return ret;
}

void CTconPGamma::PrintInfo(int fd)
{
    if (device)
        device->PrintInfo(fd);
}

void CTconPGamma::UnInit(void)
{
    if (device) {
        device->UnInit();
        delete device;
        device = NULL;
    }
}

bool CTconPGamma::GetPmuInfo(char *pmuPath)
{
    bool result = false;
    int fd = -1;
    struct stat statbuff;

    if (access(pmuPath, F_OK)) {
        ALOGW("Pmu path not exist: %s\n", pmuPath);
        goto __get_pmu_info_exit;
    }
    if (stat(pmuPath, &statbuff) < 0) {
        ALOGW("Cannot get pmu file(%s) stat\n", pmuPath);
        goto __get_pmu_info_exit;
    }

    pmuInfo.buf = (unsigned char *)calloc(1, statbuff.st_size);

    if (!pmuInfo.buf) {
        ALOGE("Alloc pmu buffer fail\n");
        goto __get_pmu_info_exit;
    }
    pmuInfo.len = statbuff.st_size;

    fd = open(pmuPath, O_RDONLY);
    if (fd < 0) {
        ALOGE("Open pmu file(%s) error: %s\n", pmuPath, strerror(errno));
        goto __get_pmu_info_exit;
    }

    if (read(fd, pmuInfo.buf, pmuInfo.len) <= 0) {
        ALOGE("Read pmu file(%s) error: %s\n", pmuPath, strerror(errno));
        goto __get_pmu_info_exit;
    }

    result = true;

__get_pmu_info_exit:
    if (fd >= 0)
        close(fd);

    if (!result) {
        if (pmuInfo.buf)
            free(pmuInfo.buf);
        pmuInfo.buf = NULL;
    }
    return result;
}

