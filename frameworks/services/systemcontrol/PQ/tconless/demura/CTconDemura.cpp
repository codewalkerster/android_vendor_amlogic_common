/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "CTconDemura"

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <utils/Log.h>
#include <errno.h>

#include "CTconDemura.h"
#include "DemuraCsotStandard.h"
#include "DemuraCsotNova.h"
#include "tcondef.h"

CTconDemura *CTconDemura::mInstance = NULL;
CTconDemura *CTconDemura::GetInstance()
{
    if (NULL == mInstance)
        mInstance = new CTconDemura();
    return mInstance;
}

CTconDemura::CTconDemura()
{
    memset(&ukeyDemuraInfo, 0, sizeof(ukeyDemuraInfo));
    memset(&demuraFlashPath, 0, sizeof(demuraFlashPath));
    mUkeyBlockInstance = NULL;
    device = NULL;
    plat = NULL;
}

CTconDemura::~CTconDemura() { }

int CTconDemura::Init(char *demuraPath)
{
    int ret = -1;
    plat = PlatMisc::GetInstance();

    if (!plat || !plat->IsTconlesssPlatform()) {
        ALOGE("It's not tconless panel...\n");
        goto __init_exit;
    }

    ALOGD("Tconless panel detected\n");

    if (demuraPath && (!access(demuraPath, F_OK))) {
        strncpy(demuraFlashPath, demuraPath, sizeof(demuraFlashPath));
    } else {
        if (!access(LCD_TCON_SPI_FLASH_PATH_K54, F_OK))
            strncpy(demuraFlashPath, LCD_TCON_SPI_FLASH_PATH_K54, sizeof(demuraFlashPath));
        else if (!access(LCD_TCON_SPI_FLASH_PATH_K49, F_OK))
            strncpy(demuraFlashPath, LCD_TCON_SPI_FLASH_PATH_K49, sizeof(demuraFlashPath));
        else {
            ALOGE("No flash path...\n");
            goto __init_exit;
        }
    }
    ALOGD("Use demuraPath: %s\n", demuraFlashPath);

    mUkeyBlockInstance = UKeyBlock::GetInstance();
    if ((!mUkeyBlockInstance) || (mUkeyBlockInstance->Init() < 0)) {
        ALOGE("Ukey init fail, exit...\n");
        goto __init_exit;
    }

    if (ukeyDemuraInfo.size != 0 && strlen(demuraFlashPath) > 0) {
        if (DemuraCsotStandard::detect(demuraFlashPath)) {
            device = new DemuraCsotStandard();
            ALOGD("Demura csot standard detect.\n");
        } else if (DemuraCsotNova::detect(demuraFlashPath)) {
            device = new DemuraCsotNova();
            ALOGD("Demura csot nova detect.\n");
        }
        if (!device) {
            ALOGE("Demura dev alloc fail, exit...\n");
            goto __init_exit;
        }
        if (0 != device->Init(demuraFlashPath)) {
            ALOGE("Demura dev init fail, exit...\n");
            goto __init_exit;
        }
    }
    ret = 0;

__init_exit:
    if (mUkeyBlockInstance)
        mUkeyBlockInstance->UnInit();
    return ret;
}

int CTconDemura::GenerateBin(char *binPath)
{
    DemuraBufferInfo_t *demuraBufInfo = NULL;
    int ret = -1;

    if (!device)
        goto __gen_bin_exit;
    demuraBufInfo = device->GenerateBuffer();
    if (!demuraBufInfo) {
        ALOGE("Demura buffer get fail, exit...\n");
        goto __gen_bin_exit;
    }
    if (!generateFomatBin(binPath, demuraBufInfo->buffer, demuraBufInfo->size,
              (unsigned char *)&demuraBufInfo->crc, demuraBufInfo->crclen)) {
        ALOGE("Demura gen %s fail, exit...\n", binPath);
        goto __gen_bin_exit;
    }
    ret = 0;

__gen_bin_exit:
    if (device && demuraBufInfo)
        device->DestroyBuffer(demuraBufInfo);
    return ret;
}

void CTconDemura::PrintInfo(int fd)
{
    if (device)
        device->PrintInfo(fd);
}

void CTconDemura::UnInit(void)
{
    if (device) {
        device->UnInit();
        delete device;
        device = NULL;
    }
}

unsigned int CTconDemura::generateFormatCrc32(unsigned int crc, const unsigned char *ptr, int buf_len)
{
    unsigned int crcu32 = crc;
    unsigned char b;
    const unsigned int crc32[16] = {
        0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
    };

    if (buf_len <= 0 || !ptr)
        return 0;

    crcu32 = ~crcu32;
    while (buf_len--) {
        b = *ptr++;
        crcu32 = (crcu32 >> 4) ^ crc32[(crcu32 & 0xF) ^ (b & 0xF)];
        crcu32 = (crcu32 >> 4) ^ crc32[(crcu32 & 0xF) ^ (b >> 4)];
    }

    return ~crcu32;
}

bool CTconDemura::generateFomatBin(char *binPath, unsigned char *lutBuf, unsigned int lutSize,
                                          unsigned char *crcBuf, unsigned int crcLen)
{
    unsigned char *fileBuffer = NULL;
    unsigned int i = 0, fileBufLen = 0;
    bool success = false;
    std::vector<unsigned char> header;
    std::vector<unsigned char> extheader;
    std::vector<unsigned char> body;
    std::vector<unsigned int> bodyoffset;
    unsigned int bodyCounter = 0, realBytes = 0;
    unsigned int val = 0;
    ssize_t bytes = 0;
    int fd = -1;
    unsigned char *binDataCrc = NULL;

    if (!binPath || !lutBuf || lutSize <= 0 ||
          !crcBuf || crcLen <= 0)
        goto __gen_format_bin_exit;

    if (!access(binPath, F_OK)) {
        // bin file exist, need to check crc
        binDataCrc = (unsigned char *)malloc(sizeof(unsigned char) * crcLen);
        if (!binDataCrc)
            goto __gen_format_bin_exit;
        fd = open(binPath, O_RDONLY);
        if (fd < 0) {
            ALOGE("Open file %s error: %s\n", binPath, strerror(errno));
            goto __gen_format_bin_exit;
        }
        if (lseek(fd, 4, SEEK_SET) < 0) {  //header:01_raw_data_check storage crc buf
            ALOGE("Seek file %s error: %s\n", binPath, strerror(errno));
            goto __gen_format_bin_exit;
        }
        if (read(fd, binDataCrc, crcLen) == crcLen) {
            success = true;
            for (i = 0; i < crcLen; i++) {
                if (binDataCrc[i] != crcBuf[i])
                    success = false;
            }
            if (success) {
                // crc matched, no need to re-generated
                ALOGD("Bin crc matched, no need to regenerate\n");
                goto __gen_format_bin_exit;
            } else {
                ALOGD("Bin crc miss matched, need to regenerate\n");
            }
        }
        close(fd);
        fd = -1;
    }

    /*********************************body*********************************/
    /*
     * body 0
     */
    bodyoffset.push_back(0);

    // 00_part_name
    bytes = 48;
    fileBuffer = (unsigned char *)"demura_lut_data";
    realBytes = strlen((char *)fileBuffer);
    for (i = 0; i < bytes; i++) {
        if (i < realBytes) body.push_back(fileBuffer[i]);
        else body.push_back(0);
    }

    // 01_part_id
    body.push_back(bodyCounter & 0xff);
    body.push_back((bodyCounter >> 8) & 0xff);

    // 02_tuning_flag
    body.push_back(0x10);

    // 03_part_type
    body.push_back(0xdd);

    // 04_axi_buf_id
    val = 0x1;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);

    // 05_data_byte
    val = 0x1;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);

    // 06_data_cnt
    body.push_back(lutSize & 0xff);
    body.push_back((lutSize >> 8) & 0xff);
    body.push_back((lutSize >> 16) & 0xff);
    body.push_back((lutSize >> 24) & 0xff);

    // 07_data_array
    for (i = 0; i < lutSize; i++)
        body.push_back(lutBuf[i]);

    bodyCounter++;

    /*
     * body 1
     */
    bodyoffset.push_back(body.size());
    // 00_part_name
    bytes = 48;
    fileBuffer = (unsigned char *)"demura_ddrif_en";
    realBytes = strlen((char *)fileBuffer);
    for (i = 0; i < bytes; i++) {
        if (i < realBytes) body.push_back(fileBuffer[i]);
        else body.push_back(0);
    }

    // 01_part_id
    body.push_back(bodyCounter & 0xff);
    body.push_back((bodyCounter >> 8) & 0xff);

    // 02_tuning_flag
    body.push_back(0x11);

    // 03_part_type
    body.push_back(0xb0);

    // 04_reg_addr_byte_width
    body.push_back(0x2);

    // 05_reg_data_byte_width
    body.push_back(0x4);

    // 06_reg_addr
    val = 0x1a3;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);

    // 07_data_mask
    val = 0x80000000;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);
    body.push_back((val >> 16) & 0xff);
    body.push_back((val >> 24) & 0xff);

    // 08_data_value
    val = 0x80000000;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);
    body.push_back((val >> 16) & 0xff);
    body.push_back((val >> 24) & 0xff);

    bodyCounter ++;

    /*
     * body 2
     */
    bodyoffset.push_back(body.size());
    // 00_part_name
    bytes = 48;
    fileBuffer = (unsigned char *)"demura_en";
    realBytes = strlen((char *)fileBuffer);
    for (i = 0; i < bytes; i++) {
        if (i < realBytes) body.push_back(fileBuffer[i]);
        else body.push_back(0);
    }

    // 01_part_id
    body.push_back(bodyCounter & 0xff);
    body.push_back((bodyCounter >> 8) & 0xff);

    // 02_tuning_flag
    body.push_back(0x11);

    // 03_part_type
    body.push_back(0xb0);

    // 04_reg_addr_byte_width
    body.push_back(0x2);

    // 05_reg_data_byte_width
    body.push_back(0x4);

    // 06_reg_addr
    val = 0x190;
    body.push_back(val  & 0xff);
    body.push_back((val  >> 8) & 0xff);

    // 07_data_mask
    val = 0x80000000;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);
    body.push_back((val >> 16) & 0xff);
    body.push_back((val >> 24) & 0xff);

    // 08_data_value
    val = 0x80000000;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);
    body.push_back((val >> 16) & 0xff);
    body.push_back((val >> 24) & 0xff);

    bodyCounter ++;

    /*
     * body 3
     */
    bodyoffset.push_back(body.size());
    // 00_part_name
    bytes = 48;
    fileBuffer = (unsigned char *)"dalay_2frames";
    realBytes = strlen((char *)fileBuffer);
    for (i = 0; i < bytes; i++) {
        if (i < realBytes) body.push_back(fileBuffer[i]);
        else body.push_back(0);
    }

    // 01_part_id
    body.push_back(bodyCounter & 0xff);
    body.push_back((bodyCounter >> 8) & 0xff);

    // 02_tuning_flag
    body.push_back(0x12);

    // 03_part_type
    body.push_back(0xfd);

    // 04_delay_val_us
    val = 50000;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);
    body.push_back((val >> 16) & 0xff);
    body.push_back((val >> 24) & 0xff);

    bodyCounter ++;

    /*
     * body 4
     */
    bodyoffset.push_back(body.size());
    // 00_part_name
    bytes = 48;
    fileBuffer = (unsigned char *)"demura_bypass_off";
    realBytes = strlen((char *)fileBuffer);
    for (i = 0; i < bytes; i++) {
        if (i < realBytes) body.push_back(fileBuffer[i]);
        else body.push_back(0);
    }

    // 01_part_id
    body.push_back(bodyCounter & 0xff);
    body.push_back((bodyCounter >> 8) & 0xff);

    // 02_tuning_flag
    body.push_back(0x11);

    // 03_part_type
    body.push_back(0xb0);

    // 04_reg_addr_byte_width
    body.push_back(0x2);

    // 05_reg_data_byte_width
    body.push_back(0x4);

    // 06_reg_addr
    val = 0x202;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);

    // 07_data_mask
    val = 0x40000000;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);
    body.push_back((val >> 16) & 0xff);
    body.push_back((val >> 24) & 0xff);

    // 08_data_value
    val = 0;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);
    body.push_back((val >> 16) & 0xff);
    body.push_back((val >> 24) & 0xff);

    bodyCounter ++;

    /*
     * body 5
     */
    bodyoffset.push_back(body.size());
    // 00_part_name
    bytes = 48;
    fileBuffer = (unsigned char *)"demura_demo_off";
    realBytes = strlen((char *)fileBuffer);
    for (i = 0; i < bytes; i++) {
        if (i < realBytes) body.push_back(fileBuffer[i]);
        else body.push_back(0);
    }

    // 01_part_id
    body.push_back(bodyCounter & 0xff);
    body.push_back((bodyCounter >> 8) & 0xff);

    // 02_tuning_flag
    body.push_back(0x11);

    // 03_part_type
    body.push_back(0xb0);

    // 04_reg_addr_byte_width
    body.push_back(0x2);

    // 05_reg_data_byte_width
    body.push_back(0x4);

    // 06_reg_addr
    val = 0x190;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);

    // 07_data_mask
    val = 0x40000000;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);
    body.push_back((val >> 16) & 0xff);
    body.push_back((val >> 24) & 0xff);

    // 08_data_value
    val = 0;
    body.push_back(val & 0xff);
    body.push_back((val >> 8) & 0xff);
    body.push_back((val >> 16) & 0xff);
    body.push_back((val >> 24) & 0xff);

    bodyCounter ++;

    /*********************************ext header*********************************/
    // 00_part_cnt
    extheader.push_back(bodyCounter & 0xff);
    extheader.push_back((bodyCounter >> 8) & 0xff);

    // 01_part_mapping_size
    val = 0x4;
    extheader.push_back(val);

    // 02_reserved
    for (i = 0; i < 13; i++)
        extheader.push_back(0);

    // 03_part_position_array
    for (i = 0; i < bodyCounter; i++) {
        unsigned int bdoffset = bodyoffset.at(i);
        extheader.push_back(bdoffset & 0xff);
        extheader.push_back((bdoffset >> 8) & 0xff);
        extheader.push_back((bdoffset >> 16) & 0xff);
        extheader.push_back((bdoffset >> 24) & 0xff);
    }

    /*********************************header*********************************/
    // ATTENTION: !!! header vector has not include CRC !!!
    // 00_data_check,  skip

    // 01_raw_data_check
    bytes = 4;
    for (i = 0; i < bytes; i++) {
        if (i < crcLen) header.push_back(crcBuf[i]);
        else header.push_back(0);
    }

    // 02_block_size
    val = 0x40 + extheader.size() + body.size();
    header.push_back(val & 0xff);
    header.push_back((val >> 8) & 0xff);
    header.push_back((val >> 16) & 0xff);
    header.push_back((val >> 24) & 0xff);

    // 03_header_size
    val = 0x40;
    header.push_back(val & 0xff);
    header.push_back((val >> 8) & 0xff);

    // 04_ext_header_size
    val = extheader.size();
    header.push_back(val & 0xff);
    header.push_back((val >> 8) & 0xff);

    // 05_block_type
    val = 0x2;
    header.push_back(val & 0xff);
    header.push_back((val >> 8) & 0xff);

    // 06_block_ctrl
    val = 0x0;
    header.push_back(val & 0xff);
    header.push_back((val >> 8) & 0xff);

    // 07_block_flag
    val = 0x2;
    header.push_back(val & 0xff);
    header.push_back((val >> 8) & 0xff);
    header.push_back((val >> 16) & 0xff);
    header.push_back((val >> 24) & 0xff);

    // 08_init_priority
    val = 0x0;
    header.push_back(val & 0xff);
    header.push_back((val >> 8) & 0xff);

    // 09_chip_id
    val = 0x1;
    header.push_back(val & 0xff);
    header.push_back((val >> 8) & 0xff);

    // 10_name
    bytes = 36;
    fileBuffer = (unsigned char *)"tcon_demura_lut";
    realBytes = strlen((char *)fileBuffer);
    for (i = 0; i < bytes; i++) {
        if (i < realBytes) header.push_back(fileBuffer[i]);
        else header.push_back(0);
    }

    /********************************alloc buffer to gen bin******************************/
    fileBufLen = 0x4 + header.size() + extheader.size() + body.size();
    ALOGD("---> file size = %d (%#x)\n", fileBufLen, fileBufLen);
    ALOGD("---> header size =%d\n", header.size());
    ALOGD("---> extheader size=%d\n", extheader.size());
    ALOGD("---> body size=%d\n", body.size());
    fileBuffer = (unsigned char *)malloc(sizeof(unsigned char) * fileBufLen);
    if (!fileBuffer)
        goto __gen_format_bin_exit;
    memset(fileBuffer, 0, fileBufLen);
    val = 4;

    // header
    for (i = 0; i < header.size(); i++)
        fileBuffer[val+i] = header[i];
    val += header.size();

    // extheader
    for (i = 0; i < extheader.size(); i++)
        fileBuffer[val+i] = extheader[i];
    val += extheader.size();

    // body
    for (i = 0; i < body.size(); i++)
        fileBuffer[val+i] = body[i];

    // calculate crc: header+extheader+body
    // header:00_data_check
    val = generateFormatCrc32(0, (fileBuffer + 4), fileBufLen - 4);
    fileBuffer[0] = val & 0xff;
    fileBuffer[1] = (val >> 8) & 0xff;
    fileBuffer[2] = (val >> 16) & 0xff;
    fileBuffer[3] = (val >> 24) & 0xff;

    fd = open(binPath, O_CREAT|O_WRONLY, 0666);
    if (fd < 0) {
        ALOGE("Open file %s error: %s\n", binPath, strerror(errno));
        goto __gen_format_bin_exit;
    }
    val = 0;
    do {
        bytes = write(fd, &fileBuffer[val], fileBufLen - val);
        if (bytes < 0)
            break;
        val += bytes;
    } while(val < fileBufLen);

    if (bytes < 0)
        goto __gen_format_bin_exit;
    success = true;

__gen_format_bin_exit:
    if (binDataCrc)
        free(binDataCrc);
    if (fd >= 0)
        close(fd);
    if (fileBuffer)
        free(fileBuffer);
    return success;
}

