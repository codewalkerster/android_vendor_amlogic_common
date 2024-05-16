/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "SystemControl"
#define LOG_TV_TAG "CPQDataBase"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <utils/String8.h>

#include "CPQDataBase.h"
#include "CPQLog.h"
#include "CFile.h"
#include "CConfigFile.h"

extern TABLE_VER_OSD                mVerInfoOSD;
extern TABLE_DATA_STRUCT            mNonlinearMappingTable[];
extern TABLE_DATA_STRUCT            mPictureModeTable[];
extern TABLE_DATA_STRUCT            mColorTempTable[];
extern TABLE_DATA_STRUCT            mColorCustomizeTable[];

extern TABLE_PICTURE_SETTING_EXT    mPictureSettingTable;

#define PICTURE_TABLE_NAME                     "PICTURE_TABLE"

#define KEY_PQ_MODE                            "KEY_PQ_MODE"
#define KEY_LAST_PQ_MODE                       "KEY_LAST_PQ_MODE"
#define KEY_DEFAULT_PQ_MODE                    "KEY_DEFAULT_PQ_MODE"

#define KEY_PICTURE_BY_SRC                     "KEY_PICTURE_BY_SRC"
#define KEY_DEFAULT_PICTURE_BY_SRC             "KEY_DEFAULT_PICTURE_BY_SRC"

#define KEY_PICTURE_GLOBAL                     "KEY_PICTURE_GLOBAL"
#define KEY_DEFAULT_PICTURE_GLOBAL             "KEY_DEFAULT_PICTURE_GLOBAL"

#define KEY_PICTURE_MODE                       "KEY_PICTURE_MODE"
#define KEY_DEFAULT_PICTURE_MODE               "KEY_DEFAULT_PICTURE_MODE"

#define KEY_NONLINEAR_MAPPING                  "KEY_NONLINEAR_MAPPING"
#define KEY_DEFAULT_NONLINEAR_MAPPING          "KEY_DEFAULT_NONLINEAR_MAPPING"

#define KEY_COLOR_TEMPERATURE                  "KEY_COLOR_TEMPERATURE"
#define KEY_DEFAULT_COLOR_TEMPERATURE          "KEY_DEFAULT_COLOR_TEMPERATURE"

#define KEY_CMS                                "KEY_CMS"
#define KEY_DEFAULT_CMS                        "KEY_DEFAULT_CMS"

#define PQ_SQL_DEFAULT_PATH                    "/mnt/vendor/param/pq/TV_PICTURE"

CPQDataBase::CPQDataBase()
{
}

CPQDataBase::~CPQDataBase()
{
}

int CPQDataBase::Init(const char *path)
{
    static char db_path[128] = "\0";

    if (path == NULL) {
        SYS_LOGE("%s Path is NULL\n",__FUNCTION__);
        sprintf(db_path, "%s", PQ_SQL_DEFAULT_PATH);
    } else {
        sprintf(db_path, "%s", path);
    }
    SYS_LOGD("%s Gen Sql Path: %s\n",__FUNCTION__, db_path);

    closeDb();
    if (!(access(db_path, F_OK) == 0)) {
        SYS_LOGD("%s no TV_PICTURE process\n");
        if (CreateNewDB(db_path) != true) {
            SYS_LOGE("%s CreateNewDB fail!\n", db_path);
            return -1;
        }
        //Prepare SQL set/get statements
        if (PrepareTable() != true) {
            SYS_LOGE("PrepareTable fail!\n");
            closeDb();
            return -1;
        }

        InitialValue();
    } else { // db is access
        SYS_LOGD("%s has TV_PICTURE process\n");
        if (openDb(db_path) < 0) {
            SYS_LOGE("%s openDb fail!\n", db_path);
            closeDb();
            return -1;
        }
        //Prepare SQL set/get statements
        if (PrepareTable() != true) {
            SYS_LOGE("PrepareTable fail!\n");
            closeDb();
            return -1;
        }
        //check database structure
        if (!CheckTable()) {
            SYS_LOGE("%s DataBase Structural changes, Init DataBase!!!\n", __FUNCTION__);
            InitialValue();
        }
    }

    return 0;
}

int CPQDataBase::closePqDB(void)
{
    return closeDb();
}

int CPQDataBase::reopenDB(const char *db_path)
{
    int  rval = openDb(db_path);
    return rval;
}

bool CPQDataBase::CreateNewDB(const char *db_path)
{
    if (openDb(db_path) != 0) {// Create a new db file
        SYS_LOGE("%s Create db file fail!\n",__FUNCTION__);
        closeDb();
        return false;
    }

    char key[128];
    sprintf(key, "CREATE TABLE %s(Key TEXT PRIMARY KEY, Value BLOB)", PICTURE_TABLE_NAME);
    if (exeSql(key) != true) {
        SYS_LOGE("%s CREATE SQL: %s fail!\n", key);
        return false;
    }

    return true;
}

bool CPQDataBase::PrepareTable(void)
{
    //for get
    char key_get[128];
    sprintf(key_get, "SELECT Value FROM %s WHERE Key=?", PICTURE_TABLE_NAME);
    PrepareSqlSelect(key_get);

    //for set
    char key_set[128];
    sprintf(key_set, "REPLACE INTO %s(Key,Value) VALUES(?,?)", PICTURE_TABLE_NAME);
    PrepareSqlUpdate(key_set);

    if (exeSql("BEGIN TRANSACTION") != true) {
        return false;
    }

    return true;
}

bool CPQDataBase::LoadOSDBin(void)
{
    static const char *OSD_bin_path = NULL;
    OSD_bin_path = CConfigFile::GetInstance()->GetString(CFG_SECTION_PQ, CFG_PQ_UI_SETTING_CFG_PATH, PQ_OSD_BIN_PATH);

    if (OSD_bin_path == NULL) {
        SYS_LOGE("%s sql path is NULL\n",__FUNCTION__);
        return false;
    }
    SYS_LOGD("%s Load OSD Bin: %s\n",__FUNCTION__, OSD_bin_path);

    if (CConfigFile::GetInstance()->isFileExist(OSD_bin_path)) {
        CFile FilePq(OSD_bin_path);
        if (FilePq.copyTo(PQ_OSD_BIN_PATH) != 0) {
            SYS_LOGE("copy file to %s error!\n", PQ_OSD_BIN_PATH);
        }
    } else if (CConfigFile::GetInstance()->isFileExist(PQ_OSD_BIN_DEFAULT_PATH_0)) {
        CFile FilePq(PQ_OSD_BIN_DEFAULT_PATH_0);
        if (FilePq.copyTo(PQ_OSD_BIN_PATH) != 0) {
            SYS_LOGE("copy file to %s error!\n", PQ_OSD_BIN_PATH);
        }
    } else if (CConfigFile::GetInstance()->isFileExist(PQ_OSD_BIN_PATH)) {
        SYS_LOGD("has %s\n", PQ_OSD_BIN_PATH);
    } else {
        SYS_LOGE("no %s and no %s\n", OSD_bin_path, PQ_OSD_BIN_DEFAULT_PATH_0);
    }

    FILE *pFile = fopen(PQ_OSD_BIN_PATH, "r");
    if (pFile == NULL) {
        SYS_LOGE("%s pFile is NULL, no OSD bin: %s access! retun to load default Table\n", __FUNCTION__, OSD_bin_path);
        return false;
    }

    fseek(pFile, 0L, SEEK_END);
    long size = ftell(pFile);
    rewind(pFile);

    if (size <= 0) {
        SYS_LOGE("%s Load  %s fail! size = %ld\n",__FUNCTION__, OSD_bin_path, size);
        fclose(pFile);
        return false;
    }

    unsigned char *buffer = (unsigned char *)malloc(size);
    if (buffer == NULL) {
        SYS_LOGE("%s malloc buffer size = %d fail\n",__FUNCTION__, OSD_bin_path, size);
        fclose(pFile);
        return false;
    }

    unsigned int readSize = fread(buffer, 1, size, pFile);
    if (readSize != (unsigned int)size) {
        SYS_LOGE("%s fread readSize = %d fail\n",__FUNCTION__, readSize);
        free(buffer);
        fclose(pFile);
        return false;
    }

    // Check CRC
    PQ_OSD_FILE_HEADER *pHeader = (PQ_OSD_FILE_HEADER *)buffer;
    unsigned char *pData = buffer + sizeof(PQ_OSD_FILE_HEADER);
    unsigned int crcSize = size - sizeof(PQ_OSD_FILE_HEADER);
    unsigned short int crc = get_crc16((unsigned const char *)pData, crcSize);

    if (crc != pHeader->crc) {
        SYS_LOGE("%s CRC: %d != pHeader->crc: %d, bin not match, return false\n",__FUNCTION__, crc, pHeader->crc);
        free(buffer);
        fclose(pFile);
        return false;
    } else {
        SYS_LOGD("%s CRC %d OK\n",__FUNCTION__, crc);
    }

    PQ_OSD_TABLE_STRUCT_HEADER *pTableHeader = NULL;
    unsigned char *pTableData = NULL;
    unsigned char *pTableIndex = NULL;
    PQ_OSD_TABLE_DATA_STRUCT_SAVE *pIndex = NULL;

    //Ver Information
    GetTable(pData, pHeader->PqOsdVerOffset, &pTableHeader, &pTableData, &pTableIndex);
    TABLE_VER_OSD *pVerOsdData = (TABLE_VER_OSD *)pTableData;
    SYS_LOGD("%s ProjectVersion %s\n",__FUNCTION__, pVerOsdData->ProjectVersion);
    SYS_LOGD("%s ChipVersion %s\n",__FUNCTION__, pVerOsdData->ChipVersion);

    //Nonlinear Table
    GetTable(pData, pHeader->NonlinearMappingOffset, &pTableHeader, &pTableData, &pTableIndex);
    NonlinearModeType *pNonlinearData = (NonlinearModeType *)pTableData;
    pIndex = (PQ_OSD_TABLE_DATA_STRUCT_SAVE *)pTableIndex;
    for (int i = 0; i < pTableHeader->TableNum; i++) {
        int idx = pIndex[i].tableDataIdx;
        SetNonlinearData(&pNonlinearData[idx], (pq_source_input_t)pIndex[i].source, (pq_sig_fmt_t)pIndex[i].timing);
        SetDefaultNonlinearData(&pNonlinearData[idx], (pq_source_input_t)pIndex[i].source, (pq_sig_fmt_t)pIndex[i].timing);
    }

    // Picture mode table
    GetTable(pData, pHeader->PictureModeOffset, &pTableHeader, &pTableData, &pTableIndex);
    PICTURE_MODE_DATA *pPicData = (PICTURE_MODE_DATA *)pTableData;
    pIndex = (PQ_OSD_TABLE_DATA_STRUCT_SAVE *)pTableIndex;
    for (int i = 0; i < pTableHeader->TableNum; i++) {
        int idx = 0;
        for (short j = 0; j < pIndex[i].tableDataIdx; j++) {
            idx += pIndex[j].tableDataLen;
        }
        for (unsigned int j = 0; j < pIndex[i].tableDataLen; j++) {
            SetPictureModeData(&pPicData[idx + j], (pq_source_input_t)pIndex[i].source, (pq_sig_fmt_t)pIndex[i].timing, (PICTURE_MODE)pPicData[idx + j].mode);
            SetDefaultPictureModeData(&pPicData[idx + j], (pq_source_input_t)pIndex[i].source, (pq_sig_fmt_t)pIndex[i].timing, (PICTURE_MODE)pPicData[idx + j].mode);
        }
    }

    // Color temperature table
    GetTable(pData, pHeader->ColorTempOffset, &pTableHeader, &pTableData, &pTableIndex);
    COLORTEMP_DATA *pColorTemp = (COLORTEMP_DATA *)pTableData;
    pIndex = (PQ_OSD_TABLE_DATA_STRUCT_SAVE *)pTableIndex;
    for (int i = 0; i < pTableHeader->TableNum; i++) {
        int idx = pIndex[i].tableDataIdx;
        for (int j = 0; j < pIndex[i].tableDataLen; j++) {
            SetColorTemperatureData(&pColorTemp[idx + j], (pq_source_input_t)pIndex[i].source, (pq_sig_fmt_t)pIndex[i].timing, j);
            SetDefaultColorTemperatureData(&pColorTemp[idx + j], (pq_source_input_t)pIndex[i].source, (pq_sig_fmt_t)pIndex[i].timing, j);
        }
    }

    // Color Customize table
    GetTable(pData, pHeader->ColorTempOffset, &pTableHeader, &pTableData, &pTableIndex);
    TABLE_CMS *pColorCustomize = (TABLE_CMS *)pTableData;
    pIndex = (PQ_OSD_TABLE_DATA_STRUCT_SAVE *)pTableIndex;
    for (int i = 0; i < pTableHeader->TableNum; i++) {
        int idx = pIndex[i].tableDataIdx;
        SetColorCustomizeData(&pColorCustomize[idx], (pq_source_input_t)pIndex[i].source, (pq_sig_fmt_t)pIndex[i].timing);
        SetDefaultColorCustomizeData(&pColorCustomize[idx], (pq_source_input_t)pIndex[i].source, (pq_sig_fmt_t)pIndex[i].timing);
    }

    // Picture Setting Ext Table
    GetTable(pData, pHeader->PictureSettingExt, &pTableHeader, &pTableData, &pTableIndex);
    TABLE_PICTURE_SETTING_EXT *pPictureExt = (TABLE_PICTURE_SETTING_EXT *)pTableData;

    PICTURE_MODE_DEFAULT *pPictureMode = &pPictureExt->PictureModeParam;
    SetPictureMode(pPictureMode, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT);
    SetLastPictureMode(pPictureMode, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT);
    SetDefaultPictureMode(pPictureMode, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT);

    PICTURE_SETTING_BY_SRC *bySrc = &pPictureExt->BySrcParam;
    SetPictureStructDataBySrc(bySrc, PQ_SRC_DEFAULT);
    SetDefaultPictureStructDataBySrc(bySrc, PQ_SRC_DEFAULT);

    PICTURE_SETTING_GLOBAL *byGlobal = &pPictureExt->ByGlobalParam;
    SetPictureStructDataGlobal(byGlobal);
    SetDefaultPictureStructDataGlobal(byGlobal);

    if (buffer)
        free(buffer);
    if (pFile)
        fclose(pFile);

    return true;
}

bool CPQDataBase::LoadDefaultTable(void)
{
    SYS_LOGD("%s Star to load default Table\n",__FUNCTION__);

    SYS_LOGD("%s ProjectVersion %s\n",__FUNCTION__, mVerInfoOSD.ProjectVersion);
    SYS_LOGD("%s ChipVersion %s\n",__FUNCTION__, mVerInfoOSD.ChipVersion);

    for (unsigned int i = 0; i < GetNonlinearMappingTableSize(); i++) {
        TABLE_DATA_STRUCT* pTable = &mNonlinearMappingTable[i];
        NonlinearModeType *pData = (NonlinearModeType *)pTable->tableData;
        for (unsigned int j = 0; j < pTable->tableDataLen; j++) {
            SetNonlinearData(&pData[j], pTable->source, pTable->timing);
            SetDefaultNonlinearData(&pData[j], pTable->source, pTable->timing);
        }
    }

    for (unsigned int i = 0; i < GetPictureModeTableSize(); i++) {
        TABLE_DATA_STRUCT* pTable = &mPictureModeTable[i];
        PICTURE_MODE_DATA *pData = (PICTURE_MODE_DATA *)pTable->tableData;
        for (unsigned int j = 0; j < pTable->tableDataLen; j++) {
            SetPictureModeData(&pData[j], pTable->source, pTable->timing, pData[j].mode);
            SetDefaultPictureModeData(&pData[j], pTable->source, pTable->timing, pData[j].mode);
        }
    }

    for (unsigned int i = 0; i < GetColorTempTableSize(); i++) {
        TABLE_DATA_STRUCT* pTable = &mColorTempTable[i];
        COLORTEMP_DATA *pData = (COLORTEMP_DATA *)pTable->tableData;
        for (int j = 0; j < pTable->tableDataLen; j++) {
            SetColorTemperatureData(&pData[j], pTable->source, pTable->timing, j);
            SetDefaultColorTemperatureData(&pData[j], pTable->source, pTable->timing, j);
        }
    }

    for (unsigned int i = 0; i < GetColorCustomizeTableSize(); i++) {
        TABLE_DATA_STRUCT* pTable = &mColorCustomizeTable[i];
        TABLE_CMS *pData = (TABLE_CMS *)pTable->tableData;
        for (unsigned int j = 0; j < pTable->tableDataLen; j++) {
            SetColorCustomizeData(&pData[j], pTable->source, pTable->timing);
            SetDefaultColorCustomizeData(&pData[j], pTable->source, pTable->timing);
        }
    }

    SetPictureMode(&mPictureSettingTable.PictureModeParam, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT);
    SetLastPictureMode(&mPictureSettingTable.PictureModeParam, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT);
    SetDefaultPictureMode(&mPictureSettingTable.PictureModeParam, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT);

    SetPictureStructDataBySrc(&mPictureSettingTable.BySrcParam, PQ_SRC_DEFAULT);
    SetDefaultPictureStructDataBySrc(&mPictureSettingTable.BySrcParam, PQ_SRC_DEFAULT);

    SetPictureStructDataGlobal(&mPictureSettingTable.ByGlobalParam);
    SetDefaultPictureStructDataGlobal(&mPictureSettingTable.ByGlobalParam);

    return true;
}

void CPQDataBase::GetTable(unsigned char *pData,
                         int offset,
                         PQ_OSD_TABLE_STRUCT_HEADER **ppTableHeader,
                         unsigned char **ppTableData,
                         unsigned char **ppTableIndex)
{
    *ppTableHeader = (PQ_OSD_TABLE_STRUCT_HEADER *)(pData + offset);
    *ppTableData = pData + offset + sizeof(PQ_OSD_TABLE_STRUCT_HEADER);
    *ppTableIndex = pData + offset + sizeof(PQ_OSD_TABLE_STRUCT_HEADER) + (*ppTableHeader)->TableSize;
}

void CPQDataBase::InitialValue(void)
{
    if (!LoadOSDBin()) {
        LoadDefaultTable();
    }

    return;
}

bool CPQDataBase::CheckTable(void)
{
    NonlinearModeType NonlinearMode;
    if (!GetNonlinearData(&NonlinearMode, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT))
    {
        SYS_LOGE("[%s] Check Nonlinear Table DB fail, start reset DB\n", __FUNCTION__);
        return false;
    }

    PICTURE_MODE_DATA Picture;
    if (!GetPictureModeData(&Picture, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, PICTURE_MODE_STANDARD))
    {
        SYS_LOGE("[%s] Check Picture Table DB fail, start reset DB\n", __FUNCTION__);
        return false;
    }

    COLORTEMP_DATA ColorTemp;
    if (!GetColorTemperatureData(&ColorTemp, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, 0))
    {
        SYS_LOGE("[%s] Check ColorTemp Table DB fail, start reset DB\n", __FUNCTION__);
        return false;
    }

    PICTURE_MODE_DEFAULT PictureMode;
    if (!GetPictureMode(&PictureMode, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT))
    {
        SYS_LOGE("[%s] Check GetPictureMode Table DB fail, start reset DB\n", __FUNCTION__);
        return false;
    }

    PICTURE_SETTING_BY_SRC BySrcParam;
    if (!GetPictureStructDataBySrc(&BySrcParam, PQ_SRC_DEFAULT))
    {
        SYS_LOGE("[%s] Check GetPictureStructDataBySrc Table DB fail, start reset DB\n", __FUNCTION__);
        return false;
    }

    PICTURE_SETTING_GLOBAL GlobalParam;
    if (!GetPictureStructDataGlobal(&GlobalParam))
    {
        SYS_LOGE("[%s] Check GetPictureStructDataGlobal Table DB fail, start reset DB\n", __FUNCTION__);
        return false;
    }

    TABLE_CMS CmsTable;
    if (!GetColorCustomizeData(&CmsTable, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT))
    {
        SYS_LOGE("[%s] Check GetColorCustomizeData Table DB fail, start reset DB\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool CPQDataBase::SetPictureMode(PICTURE_MODE_DEFAULT *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    char key[64];
    sprintf(key, "%s_%d_%d", KEY_PQ_MODE, src, timing);

    return SetBlobValueToTable(key, (void *)pData, sizeof(PICTURE_MODE_DEFAULT));
}

bool CPQDataBase::GetPictureMode(PICTURE_MODE_DEFAULT *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d", KEY_PQ_MODE, src, timing);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(PICTURE_MODE_DEFAULT));
}

bool CPQDataBase::SetLastPictureMode(PICTURE_MODE_DEFAULT *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    char key[64];
    sprintf(key, "%s_%d_%d", KEY_LAST_PQ_MODE, src, timing);

    return SetBlobValueToTable(key, (void *)pData, sizeof(PICTURE_MODE_DEFAULT));
}

bool CPQDataBase::GetLastPictureMode(PICTURE_MODE_DEFAULT *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d", KEY_LAST_PQ_MODE, src, timing);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(PICTURE_MODE_DEFAULT));
}

bool CPQDataBase::SetDefaultPictureMode(PICTURE_MODE_DEFAULT *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    char key[64];
    sprintf(key, "%s_%d_%d", KEY_DEFAULT_PQ_MODE, src, timing);

    return SetBlobValueToTable(key, (void *)pData, sizeof(PICTURE_MODE_DEFAULT));
}

bool CPQDataBase::GetDefaultPictureMode(PICTURE_MODE_DEFAULT *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d", KEY_DEFAULT_PQ_MODE, src, timing);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(PICTURE_MODE_DEFAULT));
}

bool CPQDataBase::SetPictureStructDataBySrc(PICTURE_SETTING_BY_SRC *pData, pq_source_input_t src)
{
    char key[64];
    sprintf(key, "%s_%d", KEY_PICTURE_BY_SRC, src);

    return SetBlobValueToTable(key, (void *)pData, sizeof(PICTURE_SETTING_BY_SRC));
}

bool CPQDataBase::GetPictureStructDataBySrc(PICTURE_SETTING_BY_SRC *pData, pq_source_input_t src)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d", KEY_PICTURE_BY_SRC, src);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(PICTURE_SETTING_BY_SRC));
}

bool CPQDataBase::SetDefaultPictureStructDataBySrc(PICTURE_SETTING_BY_SRC *pData, pq_source_input_t src)
{
    char key[64];
    sprintf(key, "%s_%d", KEY_DEFAULT_PICTURE_BY_SRC, src);

    return SetBlobValueToTable(key, (void *)pData, sizeof(PICTURE_SETTING_BY_SRC));
}

bool CPQDataBase::GetDefaultPictureStructDataBySrc(PICTURE_SETTING_BY_SRC *pData, pq_source_input_t src)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d", KEY_DEFAULT_PICTURE_BY_SRC, src);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(PICTURE_SETTING_BY_SRC));
}

bool CPQDataBase::SetPictureStructDataGlobal(PICTURE_SETTING_GLOBAL *pData)
{
    char key[64];
    sprintf(key, "%s", KEY_PICTURE_GLOBAL);

    return SetBlobValueToTable(key, (void *)pData, sizeof(PICTURE_SETTING_GLOBAL));
}

bool CPQDataBase::GetPictureStructDataGlobal(PICTURE_SETTING_GLOBAL *pData)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s", KEY_PICTURE_GLOBAL);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(PICTURE_SETTING_GLOBAL));
}

bool CPQDataBase::SetDefaultPictureStructDataGlobal(PICTURE_SETTING_GLOBAL *pData)
{
    char key[64];
    sprintf(key, "%s", KEY_DEFAULT_PICTURE_GLOBAL);

    return SetBlobValueToTable(key, (void *)pData, sizeof(PICTURE_SETTING_GLOBAL));
}

bool CPQDataBase::GetDefaultPictureStructDataGlobal(PICTURE_SETTING_GLOBAL *pData)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s", KEY_DEFAULT_PICTURE_GLOBAL);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(PICTURE_SETTING_GLOBAL));
}

bool CPQDataBase::SetPictureModeData(PICTURE_MODE_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, PICTURE_MODE mode)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d_%d", KEY_PICTURE_MODE, src, timing, mode);

    return SetBlobValueToTable(key, (void *)pData, sizeof(PICTURE_MODE_DATA));
}

bool CPQDataBase::SetDefaultPictureModeData(PICTURE_MODE_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, PICTURE_MODE mode)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d_%d", KEY_DEFAULT_PICTURE_MODE, src, timing, mode);

    return SetBlobValueToTable(key, (void *)pData, sizeof(PICTURE_MODE_DATA));
}

bool CPQDataBase::GetPictureModeData(PICTURE_MODE_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, PICTURE_MODE mode)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d_%d", KEY_PICTURE_MODE, src, timing, mode);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(PICTURE_MODE_DATA));
}

bool CPQDataBase::GetDefaultPictureModeData(PICTURE_MODE_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, PICTURE_MODE mode)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d_%d", KEY_DEFAULT_PICTURE_MODE, src, timing, mode);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(PICTURE_MODE_DATA));
}

bool CPQDataBase::SetColorTemperatureData(COLORTEMP_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, int level)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d_%d", KEY_COLOR_TEMPERATURE, src, timing, level);

    return SetBlobValueToTable(key, (void *)pData, sizeof(COLORTEMP_DATA));
}

bool CPQDataBase::SetDefaultColorTemperatureData(COLORTEMP_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, int level)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d_%d", KEY_DEFAULT_COLOR_TEMPERATURE, src, timing, level);

    return SetBlobValueToTable(key, (void *)pData, sizeof(COLORTEMP_DATA));
}

bool CPQDataBase::GetColorTemperatureData(COLORTEMP_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, int level)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d_%d", KEY_COLOR_TEMPERATURE, src, timing, level);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(COLORTEMP_DATA));
}

bool CPQDataBase::GetDefaultColorTemperatureData(COLORTEMP_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, int level)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d_%d", KEY_DEFAULT_COLOR_TEMPERATURE, src, timing, level);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(COLORTEMP_DATA));
}

bool CPQDataBase::SetColorCustomizeData(TABLE_CMS *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d", KEY_CMS, src, timing);

    return SetBlobValueToTable(key, (void *)pData, sizeof(TABLE_CMS));
}

bool CPQDataBase::SetDefaultColorCustomizeData(TABLE_CMS *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d", KEY_DEFAULT_CMS, src, timing);

    return SetBlobValueToTable(key, (void *)pData, sizeof(TABLE_CMS));
}

bool CPQDataBase::GetColorCustomizeData(TABLE_CMS *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d", KEY_CMS, src, timing);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(TABLE_CMS));
}

bool CPQDataBase::GetDefaultColorCustomizeData(TABLE_CMS *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d", KEY_DEFAULT_CMS, src, timing);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(TABLE_CMS));
}

bool CPQDataBase::SetNonlinearData(NonlinearModeType *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d", KEY_NONLINEAR_MAPPING, src, timing);

    return SetBlobValueToTable(key, (void *)pData, sizeof(NonlinearModeType));
}

bool CPQDataBase::SetDefaultNonlinearData(NonlinearModeType *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d", KEY_DEFAULT_NONLINEAR_MAPPING, src, timing);

    return SetBlobValueToTable(key, (void *)pData, sizeof(NonlinearModeType));
}

bool CPQDataBase::GetNonlinearData(NonlinearModeType *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d", KEY_NONLINEAR_MAPPING, src, timing);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(NonlinearModeType));
}

bool CPQDataBase::GetDefaultNonlinearData(NonlinearModeType *pData, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (pData == NULL) {
        return false;
    }

    char key[64];
    sprintf(key, "%s_%d_%d", KEY_DEFAULT_NONLINEAR_MAPPING, src, timing);

    return GetBlobValueFromTable(key, (void *)pData, sizeof(NonlinearModeType));
}
