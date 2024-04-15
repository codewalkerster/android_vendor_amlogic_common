/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "SystemControl"
#define LOG_TV_TAG "WhitebalanceDataBase"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <utils/String8.h>

#include "WhitebalanceDataBase.h"
#include "CPQLog.h"


static char mFilePath[64] = "\0";

WhitebalanceDataBase::WhitebalanceDataBase()
{
    Init();
}

WhitebalanceDataBase::~WhitebalanceDataBase()
{
}

int WhitebalanceDataBase::Init(void)
{
    static const char *db_path = NULL;
    db_path = CConfigFile::GetInstance()->GetString(CFG_SECTION_PQ, CFG_PQ_WB_PATH, WB_FILE_PATH);
    if (db_path == NULL) {
        SYS_LOGE("%s is NULL\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s Get Path: %s\n",__FUNCTION__, db_path);

    sprintf(mFilePath, "%s", db_path);
    return 0;
}

bool WhitebalanceDataBase::SetRGBGainOffsetData(RGB_GAIN_OFFSET *pData, int level, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    int Offset = CRI_DATA_RGBGO_START + ((src * (int)PQ_SIGFMT_MAX * (int)COLOR_TMP_MODE_MAX + timing * (int)COLOR_TMP_MODE_MAX + level) * CRI_DATE_RGBOGO_DATA_LEN);
    int OffsetFlag = Offset + (CRI_DATE_RGBOGO_DATA_LEN - sizeof(bool));

    if (!SaveDataToFile(mFilePath, Offset, CRI_DATE_RGBOGO_DATA_LEN - sizeof(int), (void *)pData)) {
        return false;
    }

    bool Flag = 1;
    if (!SaveDataToFile(mFilePath, OffsetFlag, sizeof(Flag), (void *)&Flag)) {
        return false;
    }

    return true;
}

bool WhitebalanceDataBase::GetRGBGainOffsetData(RGB_GAIN_OFFSET *pData, int level, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    int Offset = CRI_DATA_RGBGO_START + ((src * (int)PQ_SIGFMT_MAX * (int)COLOR_TMP_MODE_MAX + timing * (int)COLOR_TMP_MODE_MAX + level) * CRI_DATE_RGBOGO_DATA_LEN);
    int OffsetFlag = Offset + (CRI_DATE_RGBOGO_DATA_LEN - sizeof(bool));

    bool Flag = false;
    if (!ReadDataFromFile(mFilePath, OffsetFlag, sizeof(Flag), (void *)&Flag)) {
        return false;
    }

    if (!Flag ) {
        return false;
    }

    if (!ReadDataFromFile(mFilePath, Offset, CRI_DATE_RGBOGO_DATA_LEN - sizeof(bool), (void *)pData)) {
        return false;
    }

    return true;
}

bool WhitebalanceDataBase::SetWhitebalanceGammaData(Multipoint_GAMMA_DATA *pData, int level, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    int Offset = CRI_DATA_WB_GAMMA_OFFSET + ((src * (int)PQ_SIGFMT_MAX * (int)COLOR_TMP_MODE_MAX + timing * (int)COLOR_TMP_MODE_MAX + level) * CRI_DATE_WB_GAMMA_DATA_LEN);
    int OffsetFlag = Offset + (CRI_DATE_WB_GAMMA_DATA_LEN - sizeof(bool));

    if (!SaveDataToFile(mFilePath, Offset, CRI_DATE_WB_GAMMA_DATA_LEN - sizeof(bool), (void *)pData)) {
        return false;
    }

    bool Flag = true;
    if (!SaveDataToFile(mFilePath, OffsetFlag, sizeof(bool), (void *)&Flag)) {
        return false;
    }

    return true;
}

bool WhitebalanceDataBase::GetWhitebalanceGammaData(Multipoint_GAMMA_DATA *pData, int level, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    int Offset = CRI_DATA_WB_GAMMA_OFFSET + ((src * (int)PQ_SIGFMT_MAX * (int)COLOR_TMP_MODE_MAX + timing * (int)COLOR_TMP_MODE_MAX + level) * CRI_DATE_WB_GAMMA_DATA_LEN);
    int OffsetFlag = Offset + (CRI_DATE_WB_GAMMA_DATA_LEN - sizeof(bool));

    bool Flag = false;
    if (!ReadDataFromFile(mFilePath, OffsetFlag, sizeof(bool), (void *)&Flag)) {
        return false;
    }

    if (!Flag) {
        return false;
    }

    if (!ReadDataFromFile(mFilePath, Offset, CRI_DATE_WB_GAMMA_DATA_LEN - sizeof(bool), (void *)pData)) {
        return false;
    }

    return true;
}

bool WhitebalanceDataBase::SetGammaTableData(unsigned short *pData, int level, int type)
{
    if (pData == NULL) {
        return false;
    }

    int Offset = CRI_DATA_GAMMA_OFFSET + ((level * (int)MAX_TYPE + type) * CRI_DATE_GAMMA_DATA_LEN);
    int OffsetFlag = Offset + (CRI_DATE_GAMMA_DATA_LEN - sizeof(bool));

    if (!SaveDataToFile(mFilePath, Offset, CRI_DATE_GAMMA_DATA_LEN - sizeof(bool), (void *)pData)) {
        return false;
    }

    bool Flag = true;
    if (!SaveDataToFile(mFilePath, OffsetFlag, sizeof(bool), (void *)&Flag)) {
        return false;
    }

    return true;
}

bool WhitebalanceDataBase::GetGammaTableData(unsigned short *pData, int level, int type)
{
    if (pData == NULL) {
        return false;
    }

    int Offset = CRI_DATA_GAMMA_OFFSET + ((level * MAX_TYPE + type) * CRI_DATE_GAMMA_DATA_LEN);
    int OffsetFlag = Offset + (CRI_DATE_GAMMA_DATA_LEN - sizeof(bool));

    bool Flag = false;
    if (!ReadDataFromFile(mFilePath, OffsetFlag, sizeof(bool), (void *)&Flag)) {
        return false;
    }

    if (!Flag) {
        return false;
    }

    if (!ReadDataFromFile(mFilePath, Offset, CRI_DATE_GAMMA_DATA_LEN - sizeof(bool), (void *)pData)) {
        return false;
    }

    return true;
}

bool WhitebalanceDataBase::ReadDataFromFile(const char *file_name, int offset, int nsize, void *data_buf)
{
    int device_fd = -1;
    int ret = 0;
    int bytesRead = 0;

    if (data_buf == NULL) {
        SYS_LOGE("data_buf is NULL!!!\n");
        return false;
    }

    if (file_name == NULL) {
        SYS_LOGE("%s is NULL!\n",file_name);
        return false;
    }

    device_fd = open(file_name, O_RDWR | O_SYNC | O_CREAT, S_IRUSR | S_IWUSR);
    if (device_fd < 0) {
        SYS_LOGE("open file \"%s\" error(%s).\n", file_name, strerror(errno));
        return false;
    }

    if (lseek(device_fd, offset, SEEK_SET) < 0 ) {
        SYS_LOGE("lseek file \"%s\" error(%s).\n", file_name, strerror(errno));
        ret = -1;
    } else if ((bytesRead = read(device_fd, data_buf, nsize)) <= 0 ) {
        ret = -1;
    }

    close(device_fd);
    device_fd = -1;

    if (ret < 0)
        return false;

    return true;
}

bool WhitebalanceDataBase::SaveDataToFile(const char *file_name, int offset, int nsize, void *data_buf)
{
    int device_fd = -1;
    int wr_size;

    if (data_buf == NULL) {
        SYS_LOGE("%s, data_buf is NULL!!!\n", __FUNCTION__);
        return false;
    }

    if (file_name == NULL) {
        SYS_LOGE("%s IS NULL!\n",file_name);
        return false;
    }

    device_fd = open(file_name, O_RDWR | O_SYNC);
    if (device_fd < 0) {
        SYS_LOGE("open file \"%s\" error(%s).\n", file_name, strerror(errno));
        return false;
    }

    if (lseek(device_fd, offset, SEEK_SET) < 0) {
        SYS_LOGE("lseek file \"%s\" error(%s).\n", file_name, strerror(errno));
        close(device_fd);
        return false;
    }

    wr_size = write(device_fd, data_buf, nsize);
    if (wr_size < 0) {
        SYS_LOGE("write error = %s\n", strerror(errno));
    }

    fsync(device_fd);
    close(device_fd);
    device_fd = -1;

    return true;
}
