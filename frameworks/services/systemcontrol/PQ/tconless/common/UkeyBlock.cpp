/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "UkeyBlock"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <utils/Log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "tcondef.h"
#include "UkeyBlock.h"

#define UNIFYKEY_LCD_TCON_SPI "lcd_tcon_spi"

// unifykey node
#define UNIFYKEY_ATTACH      "/sys/class/unifykeys/attach"
#define UNIFYKEY_NAME        "/sys/class/unifykeys/name"
#define UNIFYKEY_WRITE       "/sys/class/unifykeys/write"
#define UNIFYKEY_READ        "/sys/class/unifykeys/read"
#define UNIFYKEY_EXIST       "/sys/class/unifykeys/exist"
#define UNIFYKEY_LOCK        "/sys/class/unifykeys/lock"

UKeyBlock *UKeyBlock::mInstance = NULL;

UKeyBlock::UKeyBlock():
    ukeyblock(NULL)
{
}

UKeyBlock::~UKeyBlock()
{
    ukeyblock = NULL;
}

UKeyBlock *UKeyBlock::GetInstance()
{
    if (NULL == mInstance)
        mInstance = new UKeyBlock();
    return mInstance;
}

int UKeyBlock::Init(void)
{
    int ret = -1;

    if (!ukeyblock)
        ukeyblock = (struct UKeySpiBlock *)calloc(1, sizeof(struct UKeySpiBlock));

    if (ukeyblock && (readUkey(UNIFYKEY_LCD_TCON_SPI,
                (char *)ukeyblock, sizeof(struct UKeySpiBlock)) <= 0)) {
        ALOGE("read ukey:%s fail, exit...\n", UNIFYKEY_LCD_TCON_SPI);
        goto __init_exit;
    }

    //TODO: Check crc

    ret = 0;

__init_exit:
    if (ret < 0) {
        if (ukeyblock)
            free(ukeyblock);
        ukeyblock = NULL;
    }
    return ret;
}

UKeyBlockInfo_s *UKeyBlock::getDemuraUkeyInfo()
{
    if (ukeyblock)
        return &ukeyblock->demura_info;
    return NULL;
}

UKeyBlockInfo_s *UKeyBlock::getPgammaUkeyInfo()
{
    if (ukeyblock)
        return &ukeyblock->pgamma_info;
    return NULL;
}

UKeyBlockInfo_s *UKeyBlock::getAutoaccUkeyInfo()
{
    if (ukeyblock)
        return &ukeyblock->autoacc_info;
    return NULL;
}

UKeyBlockInfo_s *UKeyBlock::getAutoflickerUkeyInfo()
{
    if (ukeyblock)
        return &ukeyblock->autoflicker_info;
    return NULL;
}

void UKeyBlock::PrintInfo(int fd)
{
    int i = 0;
    int blk_prm_num = 0;
    char str[4096] = {0};
    char tmpstr[128] = {0};

    memset(str, 0, sizeof(str));
    memset(tmpstr, 0, sizeof(tmpstr));

    if (!ukeyblock) {
        MYSTRCPY(str, tmpstr, ">>>>> There has something wrong with unifykey decoding...<<<<<\n");
        return;
    }
    MYSTRCPY(str, tmpstr, ">>>>>>>>> begin print info <<<<<<<<<<<\n");
    MYSTRCPY(str, tmpstr, "-------Crc Header------\n");
    MYSTRCPY(str, tmpstr, "Crc=%#x\n", ukeyblock->crc);
    MYSTRCPY(str, tmpstr, "Data size=%d\n", ukeyblock->data_size);
    MYSTRCPY(str, tmpstr, "Version=%#x\n", ukeyblock->version);
    MYSTRCPY(str, tmpstr, "Block cnt=%d\n", ukeyblock->block_cnt);

    // demura
    MYSTRCPY(str, tmpstr, "Demura offset=%#x(%d)\n", ukeyblock->demura_info.offset,
                   ukeyblock->demura_info.offset);
    MYSTRCPY(str, tmpstr, "Demura size=%#x(%d)\n", ukeyblock->demura_info.size,
                   ukeyblock->demura_info.size);
    blk_prm_num = sizeof(ukeyblock->demura_info.blk_prm) / sizeof(unsigned int);
    for (i = 0; i < blk_prm_num; i++)
        MYSTRCPY(str, tmpstr, "Demura param[%d]=%#x(%d)\n", i,
           ukeyblock->demura_info.blk_prm[i], ukeyblock->demura_info.blk_prm[i]);

    // pgamma
    MYSTRCPY(str, tmpstr, "PGamma offset=%#x(%d)\n", ukeyblock->pgamma_info.offset,
                   ukeyblock->pgamma_info.offset);
    MYSTRCPY(str, tmpstr, "PGamma size=%#x(%d)\n", ukeyblock->pgamma_info.size,
                   ukeyblock->pgamma_info.size);
    blk_prm_num = sizeof(ukeyblock->pgamma_info.blk_prm) / sizeof(unsigned int);
    for (i = 0; i < blk_prm_num; i++)
        MYSTRCPY(str, tmpstr, "PGamma param[%d]=%#x(%d)\n", i,
           ukeyblock->pgamma_info.blk_prm[i], ukeyblock->pgamma_info.blk_prm[i]);

    // autoacc
    MYSTRCPY(str, tmpstr, "AutoAcc offset=%#x(%d)\n", ukeyblock->autoacc_info.offset,
                   ukeyblock->autoacc_info.offset);
    MYSTRCPY(str, tmpstr, "AutoAcc size=%#x(%d)\n", ukeyblock->autoacc_info.size,
                   ukeyblock->autoacc_info.size);
    blk_prm_num = sizeof(ukeyblock->autoacc_info.blk_prm) / sizeof(unsigned int);
    for (i = 0; i < blk_prm_num; i++)
        MYSTRCPY(str, tmpstr, "AutoAcc param[%d]=%#x(%d)\n", i,
           ukeyblock->autoacc_info.blk_prm[i], ukeyblock->autoacc_info.blk_prm[i]);

    // autoflicker
    MYSTRCPY(str, tmpstr, "AutoFlicker offset=%#x(%d)\n", ukeyblock->autoflicker_info.offset,
                   ukeyblock->autoflicker_info.offset);
    MYSTRCPY(str, tmpstr, "AutoFlicker size=%#x(%d)\n", ukeyblock->autoflicker_info.size,
                   ukeyblock->autoflicker_info.size);
    blk_prm_num = sizeof(ukeyblock->autoflicker_info.blk_prm) / sizeof(unsigned int);
    for (i = 0; i < blk_prm_num; i++)
        MYSTRCPY(str, tmpstr, "AutoFlickersAutoFlicker param[%d]=%#x(%d)\n", i,
           ukeyblock->autoflicker_info.blk_prm[i], ukeyblock->autoflicker_info.blk_prm[i]);

    MYSTRCPY(str, tmpstr, ">>>>>>>>> end print info <<<<<<<<<<<\n");

    if (fd < 0)
        ALOGD("%s", str);
    else
        write(fd, str, strlen(str));
}

void UKeyBlock::UnInit(void)
{
    if (ukeyblock)
        free(ukeyblock);
    ukeyblock = NULL;
}

///////////////////////// Private ///////////////////////////
void UKeyBlock::writeFs(const char *path, const char *val)
{
    int fd = -1;
    if ((fd = open(path, O_RDWR)) < 0) {
        ALOGE("Open file %s error: %s\n", path, strerror(errno));
        goto _write_fs_exit;
    }

    write(fd, val, strlen(val));

_write_fs_exit:
    if (fd >= 0)
        close(fd);
}

int UKeyBlock::readFs(const char *path, char *buf, int count)
{
    int fd = -1, len = -1;

    if (!buf)
        return len;

    if ((fd = open(path, O_RDONLY)) < 0) {
        ALOGE("Open file %s error: %s\n", path, strerror(errno));
        return len;
    }

    len = read(fd, (void *)buf, count);
    close(fd);
    return len;
}

int UKeyBlock::readUkey(const char *path, char *value, int count)
{
    int keyLen = 0;
    char existKey[10] = {0};

    writeFs(UNIFYKEY_ATTACH, "1");
    writeFs(UNIFYKEY_NAME, path);
    readFs(UNIFYKEY_EXIST, (char*)existKey, sizeof(existKey));

    if (0 == strcmp(existKey, "0") || strstr(existKey, "none")) {
        ALOGE("Ukey: %s not exist (%s)\n", path, existKey);
        goto _read_ukey_exit;
    }

    keyLen = readFs(UNIFYKEY_READ, value, count);

_read_ukey_exit:
    return keyLen;
}
///////////////////////// Private End ///////////////////////////

