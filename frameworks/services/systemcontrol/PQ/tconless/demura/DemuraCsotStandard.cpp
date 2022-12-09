/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */
#define LOG_TAG "DemuraCsotStandard"

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <errno.h>

#include "tcondef.h"
#include "DemuraCsotStandard.h"

#define FILE_HEADER_MAGIC_NUM    0x544F5343  //ASCII: CSOT
#define SECTION_HEADER_TYPE_PARA 0x00000001
#define SECTION_HEADER_TYPE_LUT  0x00000002
#define SECTION_HEADER_TYPE_EXT  0x00000003  //extern section
#define DEMURA_PARA_MODE_MONO    0x00
#define DEMURA_PARA_MODE_RGB     0x01
#define DEMURA_PARA_MODE_EXT     0x02  //extern mode

DemuraCsotStandard::DemuraCsotStandard():
    planeBuffer(NULL)
{
    memset(&demuraFlashData, 0, sizeof(demuraFlashData));
    memset(&lutPlaneBuf, 0, sizeof(lutPlaneBuf));
}
DemuraCsotStandard::~DemuraCsotStandard() { }

bool DemuraCsotStandard::detect(char *flashPath)
{
    int fd = -1;
    struct CSOTSTD_FileHeader header;
    bool result = false;

    if (access(flashPath, F_OK)) {
        ALOGE("No path: %s, exit...\n", flashPath);
        // no flash node, exit
        return false;
    }
    fd = open(flashPath, O_RDONLY);
    if (fd < 0) {
        ALOGE("Open file %s error: %s\n", flashPath, strerror(errno));
        return false;
    }
    if (read(fd, &header, sizeof(header)) > 0) {
        ALOGD("demura magic: %#x\n", header.magicNum);
        if (header.magicNum == FILE_HEADER_MAGIC_NUM)
            result = true;
    }
    if (fd >= 0)
        close(fd);
    return result;
}

bool DemuraCsotStandard::GetFlashData(int fd, int offset, void *buf, int bufSize)
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

unsigned short DemuraCsotStandard::CalculateCrc16(unsigned char *buf, unsigned int len)
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
        crcData ^= CRC16_TABLE[dat^buf[offset++]];
    }
    return crcData;
}

bool DemuraCsotStandard::FillPlaneBuffer(void)
{
    bool success = false;
    int i = 0, j = 0, k = 0;

    /*
     * plane every line, fill 481 data
     * these data are 12bit packed storage
     * every 128bit as a group, like:
     * ------------------------
     *  plane line 0     bits
     * ------------------------
     *  plane_p0[11:0]    12
     *  plane_p1[23:12]   12
     *  plane_p2[35:24]   12
     *  plane_p3[47:36]   12
     *  plane_p4[59:48]   12
     *  plane_p5[71:60]   12
     *  plane_p6[83:72]   12
     *  plane_p7[95:84]   12
     *  plane_p8[107:96]  12
     *  plane_p9[119:108] 12
     *  reserved[127:120] 8
     *  plane_p10[11:0]   12
     *  ......
     * -----------------------
     */
    //every 10  12bit data + 8bit reserved as a group
    int vnum = demuraFlashData.demuraPara.hLutNum / 10;
    unsigned char *flashLut = demuraFlashData.demuraLut;
    int flutIdx = 0;
    int pBufIdx = 0;
    int planeIdx = 0;

    if (!flashLut || !planeBuffer)
        goto __fill_buffer_exit;

    for (i = 0; i < demuraFlashData.demuraPara.vLutNum; i++) {
        for (pBufIdx = 0; pBufIdx < demuraFlashData.demuraPara.planeNum; pBufIdx++) {
            for (j = 0; j < vnum; j++) {
                /*
                 * 5 cycle will fill 10 12bits plane buffer
                 * and then need skip 8bit(reserved)
                 */
                for (k = 0; k < 5; k++) {
                    // 8bit buffer fill as 12bit plane buffer
                    planeBuffer[planeIdx][pBufIdx++] =
                      (unsigned int)(flashLut[flutIdx]|((flashLut[flutIdx+1]&0x0f)<<8));
                    planeBuffer[planeIdx][pBufIdx++] =
                      (unsigned int)((((flashLut[flutIdx+1])&0xf0)>>4)|(flashLut[flutIdx+2]<<4));
                    flutIdx += 3;
                }
                pBufIdx++;  //skip reserved byte
            }

            if (demuraFlashData.demuraPara.hLutNum % 10) {
                // handle left flash lut
                planeBuffer[planeIdx][pBufIdx++] =
                  (unsigned int)(flashLut[flutIdx]|((flashLut[flutIdx+1]&0x0f)<<8));
            }
            flutIdx += 16;
        }
    }

    success = true;

__fill_buffer_exit:
    return success;
}

bool DemuraCsotStandard::FillLutPlaneBuffer(void)
{
    bool result = false;
    int i = 0, j = 0;
    int lutBufIdx = 0;
    int planeIdx = 0;
    int *planeBufIdx = NULL;  //record multi planes buffer index
    struct CSOTSTD_DemuraPara *pParam = &demuraFlashData.demuraPara;

    if (!planeBuffer || (!lutPlaneBuf.buffer || lutPlaneBuf.size <= 0))
        goto __fill_lut_buf_exit;

    planeBufIdx = (int *)calloc (1, sizeof(int)*pParam->planeNum);
    if (!planeBufIdx)
        goto __fill_lut_buf_exit;

    for (i = 0; i < pParam->vLutNum; i++) {
        for (planeIdx = 0; planeIdx < pParam->planeNum; planeIdx++) {
            int curPBufIdx = planeBufIdx[planeIdx];

            // handle one line
            for (j = 0; j < pParam->hLutNum / 2; j++) {
//                ALOGD("lutBufIdx=%d, planeIdx=%d, curPBufIdx=%d\n",
//                    lutBufIdx, planeIdx, curPBufIdx);
                lutPlaneBuf.buffer[lutBufIdx++] =
                   (unsigned char)(planeBuffer[planeIdx][curPBufIdx]&0xff);
                lutPlaneBuf.buffer[lutBufIdx++] =
                   (unsigned char)(((planeBuffer[planeIdx][curPBufIdx+1]&0xf)<<4)|(planeBuffer[planeIdx][curPBufIdx]>>8));
                lutPlaneBuf.buffer[lutBufIdx++] =
                   (unsigned char)(planeBuffer[planeIdx][curPBufIdx+1]>>4);
                planeBufIdx[planeIdx] += 2;
            }
            // handle one line left, check whether need to combine next line
            if ((pParam->hLutNum % 2) && !(planeIdx % 2)) {
                int nextPlaneIdx = (planeIdx+1)%pParam->planeNum;
                int nextPBufIdx = planeBufIdx[nextPlaneIdx];
//                ALOGD("nextPlaneIdx=%d, nextPBufIdx=%d\n", nextPlaneIdx, nextPBufIdx);
                if (((i+1) == pParam->vLutNum) && ((planeIdx+1) == pParam->planeNum)) {
                    // end of plane, no next data
                    lutPlaneBuf.buffer[lutBufIdx++] =
                       (unsigned char)(planeBuffer[planeIdx][curPBufIdx]&0xff);
                    lutPlaneBuf.buffer[lutBufIdx++] = (unsigned char)(planeBuffer[planeIdx][curPBufIdx]>>8);
                } else {
                    // combine with next line
                    lutPlaneBuf.buffer[lutBufIdx++] =
                       (unsigned char)(planeBuffer[planeIdx][curPBufIdx]&0xff);
                    lutPlaneBuf.buffer[lutBufIdx++] =
                       (unsigned char)(((planeBuffer[nextPlaneIdx][nextPBufIdx]&0xf)<<4)|(planeBuffer[planeIdx][curPBufIdx]>>8));
                    lutPlaneBuf.buffer[lutBufIdx++] =
                       (unsigned char)(planeBuffer[nextPlaneIdx][nextPBufIdx]>>4);
                    planeBufIdx[nextPlaneIdx]++;
                }
            }
        }
    }

    result = true;

__fill_lut_buf_exit:
    if (planeBufIdx)
        free(planeBufIdx);
    return result;
}

int DemuraCsotStandard::Init(char *flashPath)
{
    int ret = -1;
    int fd = 0;
    int i = 0;
    unsigned int left = 0;
    struct CSOTSTD_DemuraPara *pParam = NULL;

    fd = open(flashPath, O_RDONLY);
    if (fd < 0) {
        ALOGE("Open file %s error: %s\n", flashPath, strerror(errno));
        return ret;
    }

    // step 1. get header and demura para part from spi flash
    if (!GetFlashData(fd, 0, &demuraFlashData,
           sizeof(demuraFlashData)-sizeof(demuraFlashData.demuraLut))) {
        ALOGE("Get flash header failed\n");
        goto __init_exit;
    }

    // step 2. assign demura lut memory
    demuraFlashData.demuraLut = (unsigned char *)malloc(demuraFlashData.sectionLut.size);
    if (!demuraFlashData.demuraLut) {
        ALOGE("Alloc demuraLut fail\n");
        goto __init_exit;
    }
    memset(demuraFlashData.demuraLut, 0, demuraFlashData.sectionLut.size);
    pParam = &demuraFlashData.demuraPara;

    // step 3. get demura lut part from spi flash
    if (!GetFlashData(fd, demuraFlashData.sectionLut.offset,
          demuraFlashData.demuraLut, demuraFlashData.sectionLut.size))  {
        ALOGE("Get flash lut part fail\n");
        goto __init_exit;
    }

    // step 4. check demura lut crc
    if (demuraFlashData.sectionLut.SectionCrc !=
            CalculateCrc16(demuraFlashData.demuraLut, demuraFlashData.sectionLut.size)) {
        ALOGE("Get flash data header fail\n");
        goto __init_exit;
    }

    // step 5. alloc plane buffer memory
    planeBuffer = (unsigned int **)malloc(sizeof(unsigned int *) *
                   pParam->planeNum);
    if (!planeBuffer) {
        ALOGE("Alloc plane buffer fail\n");
        goto __init_exit;
    }
    memset(planeBuffer, 0, sizeof(unsigned int *) * pParam->planeNum);
    for (i = 0; i < pParam->planeNum; i++) {
        int size = sizeof(unsigned int) * pParam->hBlockSize * pParam->vBlockSize;
        planeBuffer[i] = (unsigned int *)malloc(size);
        if (!planeBuffer[i]) {
            ALOGE("Alloc plane buffer[%d] fail\n", i);
            goto __init_exit;
        }
        memset(planeBuffer[i], 0, size);
    }

    left = (unsigned int)pParam->hLutNum * pParam->vLutNum
               * pParam->planeNum * 12 % 8;
    lutPlaneBuf.size = (unsigned int)pParam->hLutNum * pParam->vLutNum
               * pParam->planeNum * 12 / 8 + (left ? 1 : 0);
    lutPlaneBuf.buffer = (unsigned char *)malloc(lutPlaneBuf.size);
    if (!lutPlaneBuf.buffer) {
        ALOGE("Alloc out plane buffer fail\n");
        goto __init_exit;
    }

    lutPlaneBuf.crc = demuraFlashData.sectionLut.SectionCrc;
    lutPlaneBuf.crclen = 2;  //crc16
    ret = 0;

__init_exit:
    if (fd >= 0)
        close(fd);
    return ret;
}

DemuraBufferInfo_t * DemuraCsotStandard::GenerateBuffer()
{
    bool result = false;
    if (!demuraFlashData.demuraLut)
        goto __gen_bin_exit;

    // fill plane buffer and generate bin
    if (!FillPlaneBuffer()) {
        ALOGE("Fill plane buffer fail\n");
        goto __gen_bin_exit;
    }

    if (!FillLutPlaneBuffer()) {
        ALOGE("Fill out plane buffer fail\n");
        goto __gen_bin_exit;
    }

    result = true;

__gen_bin_exit:
    if (result)
        return &lutPlaneBuf;
    return NULL;
}

bool DemuraCsotStandard::DestroyBuffer(DemuraBufferInfo_t * buf)
{
    if (buf->buffer)
        free(buf->buffer);
    buf->buffer = NULL;
    buf->size =0;
    return true;
}

void DemuraCsotStandard::PrintInfo(int fd)
{
    char str[4096] = {0};
    char tmpstr[128] = {0};

    memset(str, 0, sizeof(str));
    memset(tmpstr, 0, sizeof(tmpstr));
    MYSTRCPY(str, tmpstr, ">>>>>>>>> begin print info <<<<<<<<<<<\n");
    if (!demuraFlashData.demuraLut) {
        MYSTRCPY(str, tmpstr, ">>>>> There has something wrong with decoding...<<<<<\n");
    }
    MYSTRCPY(str, tmpstr, "-------File Header------\n");
    MYSTRCPY(str, tmpstr, "Magic=%#x\n", demuraFlashData.fileHeader.magicNum);
    MYSTRCPY(str, tmpstr, "Name =%s\n", demuraFlashData.fileHeader.name);
    MYSTRCPY(str, tmpstr, "Version=%#x\n", demuraFlashData.fileHeader.version);
    MYSTRCPY(str, tmpstr, "Size=%#x (dec:%d)\n", demuraFlashData.fileHeader.size, demuraFlashData.fileHeader.size);
    MYSTRCPY(str, tmpstr, "-------Section Para------\n");
    MYSTRCPY(str, tmpstr, "Type=%d\n", demuraFlashData.sectionPara.type);
    MYSTRCPY(str, tmpstr, "Offset=%#x\n", demuraFlashData.sectionPara.offset);
    MYSTRCPY(str, tmpstr, "Size=%#x (dec:%d)\n", demuraFlashData.sectionPara.size, demuraFlashData.sectionPara.size);
    MYSTRCPY(str, tmpstr, "-------Section Lut------\n");
    MYSTRCPY(str, tmpstr, "Type=%d\n", demuraFlashData.sectionLut.type);
    MYSTRCPY(str, tmpstr, "Offset=%#x\n", demuraFlashData.sectionLut.offset);
    MYSTRCPY(str, tmpstr, "Size=%#x (dec:%d)\n", demuraFlashData.sectionLut.size, demuraFlashData.sectionLut.size);
    MYSTRCPY(str, tmpstr, "-------Demura Para------\n");
    MYSTRCPY(str, tmpstr, "Demura Mode=%d\n", demuraFlashData.demuraPara.mode);
    MYSTRCPY(str, tmpstr, "Plane Num=%d\n", demuraFlashData.demuraPara.planeNum);
    MYSTRCPY(str, tmpstr, "H Block Size=%d\n", demuraFlashData.demuraPara.hBlockSize);
    MYSTRCPY(str, tmpstr, "V Block Size=%d\n", demuraFlashData.demuraPara.vBlockSize);
    MYSTRCPY(str, tmpstr, "H Lut Num=%d\n", demuraFlashData.demuraPara.hLutNum);
    MYSTRCPY(str, tmpstr, "V Lut Num=%d\n", demuraFlashData.demuraPara.vLutNum);
    MYSTRCPY(str, tmpstr, "Integer bitwidth=%d\n", demuraFlashData.demuraPara.intBitWidth);
    MYSTRCPY(str, tmpstr, "Decimals bitwidth=%d\n", demuraFlashData.demuraPara.decimalsBitWidth);
    MYSTRCPY(str, tmpstr, ">>>>>>>>> end print info <<<<<<<<<<<\n");

    if (fd < 0)
        ALOGD("%s", str);
    else
        write(fd, str, strlen(str));
}

void DemuraCsotStandard::UnInit()
{
    if (lutPlaneBuf.buffer)
        free(lutPlaneBuf.buffer);
    if (planeBuffer) {
        int i = 0;
        for (i = 0; i < demuraFlashData.demuraPara.planeNum; i++) {
            if (planeBuffer[i])
                free(planeBuffer[i]);
            planeBuffer[i] = NULL;
        }
        free(planeBuffer);
    }
    if (demuraFlashData.demuraLut)
        free(demuraFlashData.demuraLut);
    demuraFlashData.demuraLut = NULL;
    planeBuffer = NULL;
    lutPlaneBuf.buffer = NULL;
    lutPlaneBuf.size = 0;
}

