/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "PGammaDeviceCsot"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils/Log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "tcondef.h"
#include "PGammaDeviceCsot.h"

#define CSOT_PGAMMA_GAMMA_OFFSET  0x10
#define CSOT_PGAMMA_VCOM_OFFSET   0x25
#define CSOT_PGAMMA_CRC_OFFSET    0x28

#define CSOT_PGAMMA_VCOM_BW    10
#define CSOT_PGAMMA_VCOM_NUM   2
#define CSOT_PGAMMA_CRC_BW     18
#define CSOT_PGAMMA_GAMMA_BW   10    // gamma table 10 bit width

#define CSOT_PGAMMA_BASE_ADDR  0xfe000
#define CSOT_PGAMMA_SIZE       0x2a

struct CsotPGammaData {
    unsigned int crc;
    unsigned short vcom1;
    unsigned short vcom2;
    unsigned short gamma[14];
};

PGammaDeviceCsot::PGammaDeviceCsot():
    pgammaInfo(NULL),
    flashData(NULL)
{
    memset(&pgammaBuf, 0, sizeof(pgammaBuf));
}

PGammaDeviceCsot::~PGammaDeviceCsot() { }

static unsigned short CsotPGammaCrc(unsigned char *buf_lut, int len)
{
    unsigned char dat;
    unsigned int offset = 0;
    unsigned short crcData = 0;
    const unsigned short CRC16_TABLE[256] = {
        0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011, 0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
        0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
        0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
        0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1, 0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
        0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
        0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
        0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
        0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132, 0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
        0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
        0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
        0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1, 0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
        0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
        0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
        0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
        0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252, 0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
        0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
    };
    while (len--) {
        dat = (unsigned char)(crcData >> 8);
        crcData <<= 8;
        crcData ^= CRC16_TABLE[dat^buf_lut[offset++]];
    }
    return crcData;
}

bool PGammaDeviceCsot::detect(char *flashPath)
{
    bool result = false;
    int fd = -1;
    unsigned char buf_flash[CSOT_PGAMMA_SIZE];
    unsigned int crc_cal, crc_flash;

    if (!flashPath || access(flashPath, F_OK)) {
        ALOGE("No path: %s, exit...\n", flashPath);
        // no flash node, exit
        goto __detect_exit;
    }

    fd = open(flashPath, O_RDONLY);
    if (fd < 0) {
        ALOGE("Open file %s error: %s\n", flashPath, strerror(errno));
        goto __detect_exit;
    }

    if (CSOT_PGAMMA_BASE_ADDR != lseek(fd, CSOT_PGAMMA_BASE_ADDR, SEEK_SET))
        goto __detect_exit;

    if (read(fd, &buf_flash, sizeof(buf_flash)) > 0) {
        // check data valid
        if ((buf_flash[0] == 0x0 && buf_flash[1] == 0x0 &&
            buf_flash[2] == 0x0 && buf_flash[3] == 0x0) ||
           (buf_flash[0] == 0xff && buf_flash[1] == 0xff &&
            buf_flash[2] == 0xff && buf_flash[3] == 0xff)){
            ALOGE("No data in flash...\n");
            goto __detect_exit;
        }
        // check crc
        crc_cal = CsotPGammaCrc(buf_flash, sizeof(buf_flash)-2);
        crc_flash = (unsigned short)((buf_flash[CSOT_PGAMMA_SIZE - 2] << 8)
                          | buf_flash[CSOT_PGAMMA_SIZE - 1]);
        if (crc_cal != crc_flash) {
            ALOGE("CRC check failed, crc_cal(%#x) != crc_flash(%#x)\n",
                  crc_cal, crc_flash);
            goto __detect_exit;
        }
        result = true;
    }

__detect_exit:
    if (fd >= 0)
        close(fd);
    return result;
}

int PGammaDeviceCsot::Init(char *flashPath, unsigned char *pmu, int pmuLen)
{
    int fd = -1;
    int ret = -1;

    if (!flashPath || access(flashPath, F_OK)) {
        ALOGE("No path: %s, exit...\n", flashPath);
        // no flash node, exit
        goto __init_exit;
    }

    flashData = (unsigned char *)calloc(1, CSOT_PGAMMA_SIZE);
    if (!flashData) {
        ALOGE("Alloc flash buffer fail\n");
        goto __init_exit;
    }

    pgammaInfo = (struct CsotPGammaData *)calloc(1, sizeof(struct CsotPGammaData));
    if (!pgammaInfo) {
        ALOGE("Alloc structure HkcPGammaData fail\n");
        goto __init_exit;
    }

    fd = open(flashPath, O_RDONLY);
    if (fd < 0) {
        ALOGE("Open file %s error: %s\n", flashPath, strerror(errno));
        goto __init_exit;
    }

    if (!GetFlashData(fd, CSOT_PGAMMA_BASE_ADDR,
             flashData, CSOT_PGAMMA_SIZE)) {
        ALOGE("GetFlashData error: %s\n", strerror(errno));
        goto __init_exit;
    }

    if (!DecodePGammaData(flashData, CSOT_PGAMMA_SIZE)) {
        ALOGE("Decode pgamma data fail!\n");
        goto __init_exit;
    }

    pgammaBuf.size = 42;
    pgammaBuf.buffer = (unsigned char *)calloc(1, pgammaBuf.size);
    if (!pgammaBuf.buffer) {
        ALOGE("Alloc pgamma buffer fail...\n");
        goto __init_exit;
    }

    // init buffer with pmu code
    if (pmu && pmuLen > 0) {
        int wrLen = pmuLen <= pgammaBuf.size ? pmuLen : pgammaBuf.size;
        memmove(pgammaBuf.buffer, pmu, wrLen);
        ALOGD("Init with pmu code ok...\n");
    }

    ret = 0;

__init_exit:
    if (fd >= 0)
        close(fd);
    return ret;
}

void PGammaDeviceCsot::PrintInfo(int fd)
{
    int i = 0, j = 0;
    int blk_prm_num = 0;
    char str[4096] = {0};
    char tmpstr[128] = {0};

    memset(str, 0, sizeof(str));
    memset(tmpstr, 0, sizeof(tmpstr));

    if (!pgammaInfo) {
        MYSTRCPY(str, tmpstr, ">>>>> There has something wrong with flash init...<<<<<\n");
        return;
    }

    MYSTRCPY(str, tmpstr, ">>>>>>>>> begin print CSOT PGamma info <<<<<<<<<<<\n");
    MYSTRCPY(str, tmpstr, "Crc=%#x (%d)\n", pgammaInfo->crc, pgammaInfo->crc);
    MYSTRCPY(str, tmpstr, "VCOM1=%#x (%d)\n", pgammaInfo->vcom1, pgammaInfo->vcom1);
    MYSTRCPY(str, tmpstr, "VCOM2=%#x (%d)\n", pgammaInfo->vcom2, pgammaInfo->vcom2);
    for (i = 0; i < 14; i++) {
        MYSTRCPY(str, tmpstr, "GAMMA[%d]=%#x (%d)\n", i+1,
            pgammaInfo->gamma[i], pgammaInfo->gamma[i]);
    }
    MYSTRCPY(str, tmpstr, ">>>>>>>>> begin print CSOT PGamma TBL <<<<<<<<<<<\n");
    for (i = 0; i < pgammaBuf.size; i++) {
        if (i % 16 == 0) {
            if (i > 0)
                MYSTRCPY(str, tmpstr, "\n");
            MYSTRCPY(str, tmpstr, "0x%08x:  ", i);
        }
        MYSTRCPY(str, tmpstr, "%02x ", pgammaBuf.buffer[i]);
    }
    MYSTRCPY(str, tmpstr, "\n");

    if (fd < 0)
        ALOGD("%s", str);
    else
        write(fd, str, strlen(str));
}

PGammaBufferInfo_s * PGammaDeviceCsot::GenerateBuffer()
{
    int i = 0;
    if (!pgammaBuf.buffer)
        return NULL;

    /* +-----------+-----------------------------+--------------------------+
     * |   addr    |           note              |        advice            |
     * +-----------+-----------------------------+--------------------------+
     * | 0x00~0x0f | pmu/levelshifter init code  |  write i2c, don't change |
     * | 0x10~0x24 | gamma code                  |  rewrite i2c             |
     * | 0x25~0x27 | vcom code                   |  rewrite i2c             |
     * | 0x28~0x29 | crc code                    |  just for check          |
     * +-----------+-----------------------------+--------------------------+
     */
    for (i = 16; i < 38; i++)
        pgammaBuf.buffer[i] = flashData[i];
    pgammaBuf.buffer[38] = (flashData[38] & 0xf0) | (pgammaBuf.buffer[38] & 0xf);
    pgammaBuf.crc = CsotPGammaCrc(pgammaBuf.buffer, pgammaBuf.size-2);
    pgammaBuf.crclen = 2;
    pgammaBuf.buffer[40] = (unsigned char)(pgammaBuf.crc >> 8);
    pgammaBuf.buffer[41] = (unsigned char)(pgammaBuf.crc & 0xff);
    return &pgammaBuf;
}

bool PGammaDeviceCsot::DestroyBuffer(PGammaBufferInfo_s * buf)
{
    if (buf->buffer)
        free(buf->buffer);
    memset(buf, 0, sizeof(PGammaBufferInfo_s));
    return true;
}

void PGammaDeviceCsot::UnInit()
{
    if (pgammaInfo)
        free(pgammaInfo);

    if (flashData)
        free(flashData);

    pgammaInfo = NULL;
    flashData = NULL;
}

bool PGammaDeviceCsot::DecodePGammaData(unsigned char *flashbuf, int bufSize)
{
    bool result = false;
    int i = 0, j = 0;
    unsigned char *gammabuf = flashbuf + CSOT_PGAMMA_GAMMA_OFFSET;
    unsigned char *vcombuf  = flashbuf + CSOT_PGAMMA_VCOM_OFFSET;
    unsigned char *crcbuf   = flashbuf + CSOT_PGAMMA_CRC_OFFSET;

    if (!flashbuf || bufSize <= 0 || !pgammaInfo)
        goto __decode_exit;

    // gamma 1-14
    for (i = 0, j = 0; i < 14; i+=2, j+=3) {
        pgammaInfo->gamma[i] =
           ((gammabuf[j] & 0x3f) << 4) | ((gammabuf[j+1] & 0x3c) >> 2);
        pgammaInfo->gamma[i+1] =
           ((gammabuf[j+1] & 0x3) << 8) | (gammabuf[j+2]);
    }

    // vcom
    pgammaInfo->vcom1  =
           (((unsigned short)(vcombuf[0] & 0x3f)) << 4) | ((vcombuf[1] & 0x3c) >> 2);
    pgammaInfo->vcom2 =
           (((unsigned short)(vcombuf[1] & 0x3)) << 8) | (vcombuf[2]);

    // crc
    pgammaInfo->crc = (unsigned short)((crcbuf[0] << 8) | crcbuf[1]);

    result = true;

__decode_exit:
    return result;
}

bool PGammaDeviceCsot::GetFlashData(int fd, int offset, void *buf, int bufSize)
{
    int ret = 0;
    int readedSize = 0;
    unsigned char *pbuf = (unsigned char *)buf;

    if (fd < 0 || offset < 0 || !buf || bufSize <= 0)
        return false;

    if (offset != lseek(fd, offset, SEEK_SET))
        return false;

    do {
        ret = read(fd, &pbuf[readedSize], bufSize-readedSize);
        if (ret < 0)
            break;
        readedSize += ret;
    } while (readedSize < bufSize);

    if (readedSize < bufSize)
        return false;
    return true;
}


