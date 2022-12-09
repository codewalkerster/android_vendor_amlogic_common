
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

    bool SetBrightness(int Data);
    bool SetBrightness_OSD(int Data);
    bool GetBrightness(int *pData);
    bool SetContrast(int Data);
    bool SetContrast_OSD(int Data);
    bool GetContrast(int *pData);
    bool SetSaturation(int Data);
    bool SetSaturation_OSD(int Data);
    bool GetSaturation(int *pData);
    bool SetHue(int Data);
    bool SetHue_OSD(int Data);
    bool GetHue(int *pData);
    bool SetBackLight(int Data);
    bool GetBackLight(int *pData);
    bool SetSharpnessLevel(int Data);
    bool GetSharpnessLevel(int *pData);

    bool SetColorTempData_RGain(unsigned int Data);
    bool SetColorTempData_GGain(unsigned int Data);
    bool SetColorTempData_BGain(unsigned int Data);
    bool SetColorTempData_ROffset(int Data);
    bool SetColorTempData_GOffset(int Data);
    bool SetColorTempData_BOffset(int Data);

    bool Set_PQOsdBinPath(char *path);

    bool SetNonlinearModeType(NonlinearModeType *pData);
    bool GetNonlinearModeType(NonlinearModeType *pData);

    bool SetColorTempData(COLORTEMP_DATA *pData);
    bool GetColorTempData(COLORTEMP_DATA *pData);

    bool SetPictureModeData(PICTURE_MODE_DATA *pData);
    bool GetPictureModeData(PICTURE_MODE_DATA *pData);

    void* GetTableData(PQ_OSD_TABLE_TYPE type, unsigned int *indexTabLen);

private:
    static PQTableOSD *mInstance;
    bool Get_PQOsdBinName(char *name);
    bool Load_PQOsdBin(char *name);
};

#endif

