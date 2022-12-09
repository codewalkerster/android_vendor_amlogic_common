/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "PGammaDeviceHkc"

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
#include "PGammaDeviceHkc.h"
struct __attribute__((packed)) HkcPGammaData {
    unsigned short crc;
    unsigned char vcom;
    unsigned short gamma[14];
};

#define HKC_PGAMMA_DEF_OFFSET 0x0
#define HKC_PGAMMA_SIZE 31
#define HKC_PGAMMA_GAMMA_BW   12   // gamma table 10 bit width
#define HKC_PGAMMA_VCOM_BW    12   // vcom 10 bit width

#define SHORT_SWAP_BYTE(val) (((val) >> 8) | ((unsigned short)((val) & 0xff) << 8))

PGammaDeviceHkc::PGammaDeviceHkc():
    pgammaInfo(NULL),
    flashData(NULL)
{
    memset(&pgammaBuf, 0, sizeof(pgammaBuf));
}

PGammaDeviceHkc::~PGammaDeviceHkc() { }

static unsigned short HkcPGammaCrc(unsigned char *buf_lut, int len)
{
    unsigned int i = 0, j = 0;
    unsigned short crc = 0x0;
    unsigned short crc_temp_1, crc_temp_2, crc_temp_3;
    unsigned char data_buffer;
    unsigned int checksum = 0;

    while (len--) {
        data_buffer = buf_lut[i++];
        checksum += data_buffer;
        crc_temp_1 = 0;
        crc_temp_2 = 0;
        crc_temp_3 = 0;
        for (j = 0; j < 8; j++) {
            crc_temp_1 = ((crc >> 15) ^ data_buffer) & 0x0001;
            crc_temp_2 = ((crc >> 1) ^ crc_temp_1) & 0x0001;
            crc_temp_3 = ((crc >> 14) ^ crc_temp_1) & 0x0001;
            crc = crc << 1;
            crc &= 0x7ffa;
            crc |= crc_temp_1;
            crc |= (crc_temp_2 << 2);
            crc |= (crc_temp_3 << 15);
            data_buffer >>= 1;
        }
    }
    return crc;
}

bool PGammaDeviceHkc::detect(char *flashPath)
{
    bool result = false;
    int fd = -1;
    struct HkcPGammaData pgammaData;
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

    if (read(fd, &pgammaData, sizeof(pgammaData)) > 0) {
        // check data valid
        if ((pgammaData.crc == 0x0) ||
             (pgammaData.crc == 0xffff)) {
            ALOGE("No data in flash...\n");
            goto __detect_exit;
        }

        // check crc
        crc_flash = SHORT_SWAP_BYTE(pgammaData.crc);
        crc_cal = (unsigned short)HkcPGammaCrc(((unsigned char*)&pgammaData) + 2, sizeof(pgammaData) - 2);
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

int PGammaDeviceHkc::Init(char *flashPath, unsigned char *pmu, int pmuLen)
{
    int fd = -1;
    int ret = -1;
    int i = 0;

    if (!flashPath || access(flashPath, F_OK)) {
        ALOGE("No path: %s, exit...\n", flashPath);
        // no flash node, exit
        goto __init_exit;
    }

    flashData = (unsigned char *)calloc(1, HKC_PGAMMA_SIZE);
    if (!flashData) {
        ALOGE("Alloc flash buffer fail\n");
        goto __init_exit;
    }

    pgammaInfo = (struct HkcPGammaData *)calloc(1, sizeof(struct HkcPGammaData));
    if (!pgammaInfo) {
        ALOGE("Alloc structure HkcPGammaData fail\n");
        goto __init_exit;
    }

    fd = open(flashPath, O_RDONLY);
    if (fd < 0) {
        ALOGE("Open file %s error: %s\n", flashPath, strerror(errno));
        goto __init_exit;
    }

    if (!GetFlashData(fd, HKC_PGAMMA_DEF_OFFSET,
             pgammaInfo, sizeof(struct HkcPGammaData))) {
        ALOGE("GetFlashData error: %s\n", strerror(errno));
        goto __init_exit;
    }

    memmove(flashData, pgammaInfo, HKC_PGAMMA_SIZE);

    // swap high and low byte
    pgammaInfo->crc = SHORT_SWAP_BYTE(pgammaInfo->crc);
    for (i = 0; i < 14; i++)
        pgammaInfo->gamma[i] = SHORT_SWAP_BYTE(pgammaInfo->gamma[i]);

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

void PGammaDeviceHkc::PrintInfo(int fd)
{
    int i = 0;
    int blk_prm_num = 0;
    char str[4096] = {0};
    char tmpstr[128] = {0};

    memset(str, 0, sizeof(str));
    memset(tmpstr, 0, sizeof(tmpstr));
    if (!pgammaInfo) {
        MYSTRCPY(str, tmpstr, ">>>>> There has something wrong with flash init...<<<<<\n");
        return;
    }
    MYSTRCPY(str, tmpstr, ">>>>>>>>> begin print HKC PGamma info <<<<<<<<<<<\n");
    MYSTRCPY(str, tmpstr, "Crc=%#x (%d)\n", pgammaInfo->crc, pgammaInfo->crc);
    MYSTRCPY(str, tmpstr, "VCOM=%#x (%d)\n", pgammaInfo->vcom, pgammaInfo->vcom);
    for (i = 0; i < 14; i++) {
        MYSTRCPY(str, tmpstr, "GAMMA[%d]=%#x (%d)\n", i+1,
            pgammaInfo->gamma[i], pgammaInfo->gamma[i]);
    }
    MYSTRCPY(str, tmpstr, ">>>>>>>>> begin print HKC PGamma TBL <<<<<<<<<<<\n");
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

PGammaBufferInfo_s * PGammaDeviceHkc::GenerateBuffer()
{
    int i = 0, j = 0;
    unsigned int vcom;
    unsigned int gamma_lut[14] = {0x0};

    if (!pgammaBuf.buffer)
        return NULL;

    if (pgammaInfo) {
        /* VCOM:
         * VCOM_VOL = (VCOM_MAX-VCOM.MIN)*VCOM_Code/127+VCOM.MIN
         * VCOM_MAX = 0x03_DEC*VCOM_LSB = 7.9984
         * VCOM_MIN = 0x04_DEC*VCOM_LSB = 4.9990
         * VCOM_LSB = G_VREF/1023 = 0.0714
         * G_VREF = 15.1161
         * VCOM_Code = VCOM_VOL/CS602_LSB
         * CS602_LSB = GLDO/1024 = 0.0141
         * GLDO=10*0.2+13=14.6V
         *
         * GAMMA:
         * GAMMA_VOL = GMA_DAC_n/1023*GMA_LSB
         * GMA_LSB = G_VREF/1023 = 0.0148
         * G_VREF = 15.1161
         * GMA_Code_n = G1621_GMA6/CS602_LSB-1
         * CS602_LSB = GLDO/1024 = 0.0141
         * GLDO=10*0.2+13=14.6V
         */
        vcom = (unsigned int)(((float)(7.9984 - 4.9990) * pgammaInfo->vcom / 127 + 4.9990)
                   / 0.0141);
        for (i = 0; i < 14; i++) {
             gamma_lut[i] = (unsigned int)(((float)((float)pgammaInfo->gamma[i] / 1023 * 15.1161))
                   / 0.0141 - 1);
        }

        for (i = 16, j = 0; i < 37; i += 3, j += 2) {
            pgammaBuf.buffer[i] = (gamma_lut[j] & 0xff0) >> 4;
            pgammaBuf.buffer[i + 1] = ((gamma_lut[j] & 0xf) << 4)
                       | ((gamma_lut[j + 1] & 0xf00) >> 8);
            pgammaBuf.buffer[i + 2] = (gamma_lut[j + 1] & 0xff);
        }

        pgammaBuf.buffer[37] = (vcom & 0xff0) >> 4;
        pgammaBuf.buffer[38] = ((vcom & 0xf) << 4) | (pgammaBuf.buffer[38] & 0xf);
        pgammaBuf.crc = GenerateCrc(pgammaBuf.buffer, pgammaBuf.size - 2);
        pgammaBuf.crclen = 2;
        pgammaBuf.buffer[40] = (unsigned char)(pgammaBuf.crc >> 8);
        pgammaBuf.buffer[41] = (unsigned char)(pgammaBuf.crc & 0xff);
    }
    return &pgammaBuf;
}

unsigned short PGammaDeviceHkc::GenerateCrc(unsigned char *buf, int buflen)
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
    while (buflen--) {
        dat = (unsigned char)(crcData >> 8);
        crcData <<= 8;
        crcData ^= CRC16_TABLE[dat^buf[offset++]];
    }

    return crcData;
}

bool PGammaDeviceHkc::DestroyBuffer(PGammaBufferInfo_s * buf)
{
    if (buf)
        free(buf);
    return true;
}

void PGammaDeviceHkc::UnInit()
{
    if (pgammaInfo)
        free(pgammaInfo);

    if (flashData)
        free(flashData);

    pgammaInfo = NULL;
    flashData = NULL;
}

bool PGammaDeviceHkc::GetFlashData(int fd, int offset, void *buf, int bufSize)
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

