/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef CPQDATABASE_H_
#define CPQDATABASE_H_

#include <cutils/properties.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "CSqlite.h"

#include "PQTableLoaderOSD.h"
#include "PQTableLoaderTypesOSD.h"
#include "PQTableTypeOSD.h"
#include "CConfigFile.h"

class CPQDataBase: public CSqlite {
public:
    CPQDataBase();
    ~CPQDataBase();
    int Init(const char *path);
    int closePqDB(void);
    int reopenDB(const char *db_path);

    bool SetPictureMode(PICTURE_MODE_DEFAULT *pData, pq_source_input_t src, pq_sig_fmt_t timing);
    bool GetPictureMode(PICTURE_MODE_DEFAULT *pData, pq_source_input_t src, pq_sig_fmt_t timing);
    bool SetLastPictureMode(PICTURE_MODE_DEFAULT *pData, pq_source_input_t src, pq_sig_fmt_t timing);
    bool GetLastPictureMode(PICTURE_MODE_DEFAULT *pData, pq_source_input_t src, pq_sig_fmt_t timing);
    bool SetDefaultPictureMode(PICTURE_MODE_DEFAULT *pData, pq_source_input_t src, pq_sig_fmt_t timing);
    bool GetDefaultPictureMode(PICTURE_MODE_DEFAULT *pData, pq_source_input_t src, pq_sig_fmt_t timing);

    bool SetPictureModeData(PICTURE_MODE_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, PICTURE_MODE mode);
    bool GetPictureModeData(PICTURE_MODE_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, PICTURE_MODE mode);
    bool SetDefaultPictureModeData(PICTURE_MODE_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, PICTURE_MODE mode);
    bool GetDefaultPictureModeData(PICTURE_MODE_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, PICTURE_MODE mode);

    bool SetColorTemperatureData(COLORTEMP_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, int level);
    bool GetColorTemperatureData(COLORTEMP_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, int level);
    bool SetDefaultColorTemperatureData(COLORTEMP_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, int level);
    bool GetDefaultColorTemperatureData(COLORTEMP_DATA *pData, pq_source_input_t src, pq_sig_fmt_t timing, int level);

    bool SetNonlinearData(NonlinearModeType *pData, pq_source_input_t src, pq_sig_fmt_t timing);
    bool GetNonlinearData(NonlinearModeType *pData, pq_source_input_t src, pq_sig_fmt_t timing);
    bool SetDefaultNonlinearData(NonlinearModeType *pData, pq_source_input_t src, pq_sig_fmt_t timing);
    bool GetDefaultNonlinearData(NonlinearModeType *pData, pq_source_input_t src, pq_sig_fmt_t timing);

    bool SetPictureStructDataBySrc(PICTURE_SETTING_BY_SRC *pData, pq_source_input_t src);
    bool GetPictureStructDataBySrc(PICTURE_SETTING_BY_SRC *pData, pq_source_input_t src);
    bool SetDefaultPictureStructDataBySrc(PICTURE_SETTING_BY_SRC *pData, pq_source_input_t src);
    bool GetDefaultPictureStructDataBySrc(PICTURE_SETTING_BY_SRC *pData, pq_source_input_t src);

    bool SetPictureStructDataGlobal(PICTURE_SETTING_GLOBAL *pData);
    bool GetPictureStructDataGlobal(PICTURE_SETTING_GLOBAL *pData);
    bool SetDefaultPictureStructDataGlobal(PICTURE_SETTING_GLOBAL *pData);
    bool GetDefaultPictureStructDataGlobal(PICTURE_SETTING_GLOBAL *pData);

    bool SetColorCustomizeData(TABLE_CMS *pData, pq_source_input_t src, pq_sig_fmt_t timing);
    bool GetColorCustomizeData(TABLE_CMS *pData, pq_source_input_t src, pq_sig_fmt_t timing);
    bool SetDefaultColorCustomizeData(TABLE_CMS *pData, pq_source_input_t src, pq_sig_fmt_t timing);
    bool GetDefaultColorCustomizeData(TABLE_CMS *pData, pq_source_input_t src, pq_sig_fmt_t timing);

private:
    bool CreateNewDB(const char *db_path);
    bool PrepareTable(void);
    bool LoadOSDBin(void);
    bool LoadDefaultTable(void);
    void GetTable(unsigned char *pData, int offset, PQ_OSD_TABLE_STRUCT_HEADER **ppTableHeader,unsigned char **ppTableData, unsigned char **ppTableIndex);
    bool CheckTable(void);
    void InitialValue(void);

};
#endif
