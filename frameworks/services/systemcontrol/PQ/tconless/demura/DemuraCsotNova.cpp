/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */
#define LOG_TAG "DemuraCsotNova"

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <utils/Log.h>
#include <errno.h>
#include "tcondef.h"
#include "DemuraCsotNova.h"

#define CSOT_NOVA_HEADER_MAGIC_NUM 0xa55a55aa
#define CSOT_NOVA_BLK_SIZE_4X  0x0
#define CSOT_NOVA_BLK_SIZE_8X  0x1
#define CSOT_NOVA_BLK_SIZE_16X 0x2

#define SHORT_SWAP_BYTE(val) (((val) >> 8) | ((unsigned short)((val) & 0xff) << 8))

DemuraCsotNova::DemuraCsotNova():
    demuraFlashLutLen(0)
{
    memset(&lutPlaneBuf, 0, sizeof(lutPlaneBuf));
    memset(&demuraFlashData, 0, sizeof(demuraFlashData));
}
DemuraCsotNova::~DemuraCsotNova() { }

bool DemuraCsotNova::GetFlashData(int fd, int offset, void *buf, int bufSize)
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
    } while(readedSize < bufSize);

    if (readedSize < bufSize)
        return false;
    return true;
}

bool DemuraCsotNova::detect(char *flashPath)
{
    int fd = -1;
    unsigned int magic = 0;
    bool result = false;

    if (access(flashPath, F_OK)) {
        // no flash node, exit
        ALOGE("No path: %s, exit...\n", flashPath);
        goto __detect_exit;
    }
    fd = open(flashPath, O_RDONLY);
    if (fd < 0)
        goto __detect_exit;
    lseek(fd, 0, SEEK_SET);
    if (read(fd, &magic, sizeof(magic)) > 0) {
        ALOGD("demura magic: %#x, needed magic:%#x\n", magic, CSOT_NOVA_HEADER_MAGIC_NUM);
        if (magic == CSOT_NOVA_HEADER_MAGIC_NUM)
            result = true;
    }

__detect_exit:
    if (fd >= 0)
        close(fd);
    return result;
}

unsigned short DemuraCsotNova::CalculateCrc16(unsigned char *buf, unsigned int len)
{
    unsigned int i, j;
    unsigned short crc = 0x0;
    unsigned short crc_temp_1, crc_temp_2, crc_temp_3;
    unsigned char data_buffer;
    unsigned int length;
    unsigned int checksum = 0;

    i = 0;
    length = len;
    while (length--) {
        data_buffer = buf[i++];
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

int DemuraCsotNova::Init(char *flashPath)
{
    int ret = -1, fd = -1;
    unsigned short crc = 0;
    unsigned int tmp = 0;

    fd = open(flashPath, O_RDONLY);
    if (fd < 0)
        return ret;

    // step 1. get header and demura para part from spi flash
    if (!GetFlashData(fd, 0, &demuraFlashData,
           sizeof(demuraFlashData)-sizeof(demuraFlashData.demuraLut))) {
        ALOGE("Get flash header&param error!\n");
        goto __init_exit;
    }

    // flash H of short value is in flash lower address
    demuraFlashData.paraCrc                = SHORT_SWAP_BYTE(demuraFlashData.paraCrc);
    demuraFlashData.lutCrc                 = SHORT_SWAP_BYTE(demuraFlashData.lutCrc);
    demuraFlashData.demuraPara.hLutNum     = SHORT_SWAP_BYTE(demuraFlashData.demuraPara.hLutNum);
    demuraFlashData.demuraPara.vLutNum     = SHORT_SWAP_BYTE(demuraFlashData.demuraPara.vLutNum);
    demuraFlashData.demuraPara.lowerBound  = SHORT_SWAP_BYTE(demuraFlashData.demuraPara.lowerBound);
    demuraFlashData.demuraPara.upperBound  = SHORT_SWAP_BYTE(demuraFlashData.demuraPara.upperBound);
    demuraFlashData.demuraPara.plane0Level = SHORT_SWAP_BYTE(demuraFlashData.demuraPara.plane0Level);
    demuraFlashData.demuraPara.plane1Level = SHORT_SWAP_BYTE(demuraFlashData.demuraPara.plane1Level);
    demuraFlashData.demuraPara.plane2Level = SHORT_SWAP_BYTE(demuraFlashData.demuraPara.plane2Level);
    demuraFlashData.demuraPara.plane3Level = SHORT_SWAP_BYTE(demuraFlashData.demuraPara.plane3Level);
    demuraFlashData.demuraPara.plane4Level = SHORT_SWAP_BYTE(demuraFlashData.demuraPara.plane4Level);

    // step 2. assign demura lut memory
    tmp = demuraFlashData.demuraPara.hLutNum
           * demuraFlashData.demuraPara.vLutNum
           * 12 * demuraFlashData.demuraPara.planeNum % 256;
    demuraFlashLutLen = (unsigned int)(demuraFlashData.demuraPara.hLutNum
           * demuraFlashData.demuraPara.vLutNum
           * 12 * demuraFlashData.demuraPara.planeNum / 256 + (tmp ? 1 : 0)) * 32;
    demuraFlashData.demuraLut = (unsigned char *)malloc(demuraFlashLutLen);
    if (!demuraFlashData.demuraLut) {
        ALOGE("Demura lut alloc fail, exit...\n");
        goto __init_exit;
    }
    memset(demuraFlashData.demuraLut, 0, demuraFlashLutLen);

    // step 3. get demura lut part from spi flash
    if (!GetFlashData(fd, sizeof(demuraFlashData)-sizeof(demuraFlashData.demuraLut),
          demuraFlashData.demuraLut, demuraFlashLutLen)) {
        ALOGE("Get flash demura lut error!\n");
        goto __init_exit;
    }

    // step 4. check demura lut crc
    crc = CalculateCrc16(demuraFlashData.demuraLut, demuraFlashLutLen);
    if (demuraFlashData.lutCrc != crc) {
        ALOGE("Lut crc error, flashCrc(%#x), calculateCrc(%#x)\n",
              demuraFlashData.lutCrc, crc);
        goto __init_exit;
    }

    tmp = (unsigned int)demuraFlashData.demuraPara.hLutNum
           * demuraFlashData.demuraPara.vLutNum
           * demuraFlashData.demuraPara.planeNum * 12 % 8;
    lutPlaneBuf.size = (unsigned int)demuraFlashData.demuraPara.hLutNum
           * demuraFlashData.demuraPara.vLutNum
           * demuraFlashData.demuraPara.planeNum * 12 / 8 + (tmp ? 1 : 0);
    lutPlaneBuf.buffer = (unsigned char *)malloc(lutPlaneBuf.size);
    if (!lutPlaneBuf.buffer)
        goto __init_exit;
    lutPlaneBuf.crc = demuraFlashData.lutCrc;
    lutPlaneBuf.crclen = 2;  //crc16

    ret = 0;

__init_exit:
    if (fd >= 0)
        close(fd);
    return ret;
}

DemuraBufferInfo_t * DemuraCsotNova::GenerateBuffer()
{
    unsigned char *lutBuf = lutPlaneBuf.buffer;
    unsigned int lutBufLen = lutPlaneBuf.size;
    int i = 0;
    unsigned int plane0 = 0, plane1 = 0;

    if (!lutBuf)
        return NULL;

    /*
     * flash storage format:
     *    addr    |         storage          |
     *  ----------+--------------------------|
     *  0x000000  |      p0(0)[11:4]         |
     *  0x000001  | p0(0)[3:0] | p1(0)[11:8] |
     *  0x000002  |      p1(0)[7:0]          |
     *  0x000003  |      p2(0)[11:4]         |
     *  0x000004  | p2(0)[3:0] | p0(1)[11:8] |
     *  0x000005  |      p0(1)[7:0]          |
     *    ....    |     ................     |
     *  0x08f352  |     p2(130350)[11:4]     |
     *  0x08f353  | p2(130350)[3:0] | dummy  |
     *  0x08f354  |          dummy           |
     *
     *  need to change to tcon ip recognized format, like
     *    addr    |         storage          |
     *  ----------+--------------------------|
     *  0x000000  |      p0(0)[7:0]          |
     *  0x000001  | p1(0)[3:0] | p0(0)[11:8] |
     *  0x000002  |      p1(0)[11:4]         |
     *  0x000003  |      p2(0)[7:0]          |
     *  0x000004  | p0(1)[3:0] | p2(0)[11:8] |
     *  0x000005  |      p0(1)[11:4]         |
     *    ....    |    ................      |
     *  0x08f352  |     p2(130350)[7:0]      |
     *  0x08f353  | dummy | p2(130350)[11:8] |
     *  0x08f354  |          dummy           |
     */
    for (i = 0; i < lutBufLen; i+=3) {
        if (((i + 1) < lutBufLen) && ((i + 2) < lutBufLen)) {
            plane0 = (lutBuf[i] << 4) | (lutBuf[i+1] >> 4);
            plane1 = ((lutBuf[i+1] & 0xf) << 8) | lutBuf[i+2];
            lutBuf[i] = plane0 & 0xff;
            lutBuf[i+1] = ((plane0 >> 8) & 0xf) | ((plane1 & 0xf) << 4);
            lutBuf[i+2] = (plane1 >> 4) & 0xff;
        } else if (((i + 1) < lutBufLen) && ((i + 2) >= lutBufLen)) {
            ALOGD("Last index: %#x\n", i + 1);
            plane0 = (lutBuf[i] << 4) | (lutBuf[i+1] >> 4);
            lutBuf[i] = plane0 & 0xff;
            lutBuf[i+1] = (plane0 >> 8) & 0xf;
        }
    }

    return &lutPlaneBuf;
}

bool DemuraCsotNova::DestroyBuffer(DemuraBufferInfo_t * buf)
{
    if (buf->buffer)
        free(buf->buffer);
    buf->buffer = NULL;
    buf->size =0;
    return true;
}

char *DemuraCsotNova::BlkSize2Str(unsigned char blkSize)
{
    switch (blkSize) {
    case CSOT_NOVA_BLK_SIZE_4X: return (char *)"4X";
    case CSOT_NOVA_BLK_SIZE_8X: return (char *)"8X";
    case CSOT_NOVA_BLK_SIZE_16X: return (char *)"16X";
    default: return (char *)"Unknown";
    }
}

void DemuraCsotNova::PrintInfo(int fd)
{
    char str[4096] = {0};
    char tmpstr[128] = {0};
    int offset = sizeof(demuraFlashData)-sizeof(demuraFlashData.demuraLut);

    memset(str, 0, sizeof(str));
    memset(tmpstr, 0, sizeof(tmpstr));

    MYSTRCPY(str, tmpstr, ">>>>>>>>> begin print info <<<<<<<<<<<\n");
    if (!demuraFlashData.demuraLut)
        MYSTRCPY(str, tmpstr, ">>>>> There are something wrong with decoding...<<<<<\n");
    MYSTRCPY(str, tmpstr, "-------Crc info------\n");
    MYSTRCPY(str, tmpstr, "Magic(Header) = %#x\n", demuraFlashData.header);
    MYSTRCPY(str, tmpstr, "Param crc     = %#x\n", demuraFlashData.paraCrc);
    MYSTRCPY(str, tmpstr, "Lut crc       = %#x\n", demuraFlashData.lutCrc);
    MYSTRCPY(str, tmpstr, "-------Demura param------\n");
    MYSTRCPY(str, tmpstr, "plane Num     = %d\n", demuraFlashData.demuraPara.planeNum);
    MYSTRCPY(str, tmpstr, "hLutNum       = %d\n", demuraFlashData.demuraPara.hLutNum);
    MYSTRCPY(str, tmpstr, "vLutNum       = %d\n", demuraFlashData.demuraPara.vLutNum);
    MYSTRCPY(str, tmpstr, "hBlockSize    = %d (%s)\n", demuraFlashData.demuraPara.hBlockSize,
          BlkSize2Str(demuraFlashData.demuraPara.hBlockSize));
    MYSTRCPY(str, tmpstr, "vBlockSize    = %d (%s)\n", demuraFlashData.demuraPara.vBlockSize,
          BlkSize2Str(demuraFlashData.demuraPara.vBlockSize));
    MYSTRCPY(str, tmpstr, "lowBound      = %d\n", demuraFlashData.demuraPara.lowerBound);
    MYSTRCPY(str, tmpstr, "upBound       = %d\n", demuraFlashData.demuraPara.upperBound);
    MYSTRCPY(str, tmpstr, "plane_0_level = %d\n", demuraFlashData.demuraPara.plane0Level);
    MYSTRCPY(str, tmpstr, "plane_1_level = %d\n", demuraFlashData.demuraPara.plane1Level);
    MYSTRCPY(str, tmpstr, "plane_2_level = %d\n", demuraFlashData.demuraPara.plane2Level);
    MYSTRCPY(str, tmpstr, "plane_3_level = %d\n", demuraFlashData.demuraPara.plane3Level);
    MYSTRCPY(str, tmpstr, "plane_4_level = %d\n", demuraFlashData.demuraPara.plane4Level);
    MYSTRCPY(str, tmpstr, "-------Demura Lut------\n");
    MYSTRCPY(str, tmpstr, "Lut Offset    = %d (%#x)\n", offset, offset);
    MYSTRCPY(str, tmpstr, "Lut len       = %d (%#x)\n", demuraFlashLutLen, demuraFlashLutLen);
    MYSTRCPY(str, tmpstr, "Lut End Addr  = %#x\n", demuraFlashLutLen + offset - 1);
    MYSTRCPY(str, tmpstr, "lowBound      = %d\n", demuraFlashData.demuraPara.lowerBound);
    if (lutPlaneBuf.buffer) {
        MYSTRCPY(str, tmpstr, "Lut Active len = %d (%#x)\n", lutPlaneBuf.size, lutPlaneBuf.size);
        MYSTRCPY(str, tmpstr, "Lut Active End Addr = %#x\n", lutPlaneBuf.size + offset - 1);
    }
    MYSTRCPY(str, tmpstr, ">>>>>>>>> end print info <<<<<<<<<<<\n");

    if (fd < 0)
        ALOGD("%s", str);
    else
        write(fd, str, strlen(str));
}

void DemuraCsotNova::UnInit(void)
{
    if (demuraFlashData.demuraLut)
        free(demuraFlashData.demuraLut);
    DestroyBuffer(&lutPlaneBuf);
    demuraFlashData.demuraLut = NULL;
}

