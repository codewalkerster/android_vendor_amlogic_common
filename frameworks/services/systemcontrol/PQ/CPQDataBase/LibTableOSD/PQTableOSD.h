
#ifndef _PQ_TABLE_OSD_H
#define _PQ_TABLE_OSD_H

#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "PQTableTypeOSD.h"
#include "PQTableLoaderOSD.h"

class PQTableOSD        {
public:
    PQTableOSD();
    ~PQTableOSD();
    static PQTableOSD *GetInstance();
    void Init();

    PICTURE_MODE_DATA* GetPictureModeData(void);
    bool SetPictureModeData(PICTURE_MODE_DATA* pData);

    NonlinearModeType* GetNonlinearModeType(void);
    bool SetNonlinearModeType(NonlinearModeType *pData);

    COLORTEMP_DATA* GetColorTempData(void);
    bool SetColorTempData(COLORTEMP_DATA *pData);

    GAMMA_OFFSET_DATA* GetColorTempGammaData(void);
    bool SetColorTempGammaData(GAMMA_OFFSET_DATA *pData);

    TABLE_CMS* GetColorCustomizeData(void);
    bool SetColorCustomizeData(TABLE_CMS *pData);

    TABLE_PICTURE_SETTING_EXT* GetPictureSettingExtData(void);
    bool SetPictureSettingExtData(TABLE_PICTURE_SETTING_EXT* pData);

    int GetNonlinearOsdRemapVal(nonline_params_type_t type, int Value);

    bool Set_PQOsdBinPath(char *path);

    void* GetTableData(PQ_OSD_TABLE_TYPE type, unsigned int *indexTabLen);

    TABLE_DATA_STRUCT* GetDefaultNonlinearData(void);
    TABLE_DATA_STRUCT* GetDefaultPictureModeData(void);
    TABLE_DATA_STRUCT* GetDefaultColorTempData(void);

private:
    static PQTableOSD *mInstance;
    bool Get_PQOsdBinName(char *name);
    bool Load_PQOsdBin(char *name);

private:
    PQ_OSD_TABLE_STRUCT m_PQOsdTable[PQ_OSD_TABLE_MAX];

};

#endif

