
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "PQTableOSD.h"
#include "PQTable.h"
#include "pq/adap_pq.h"

extern TABLE_VER_OSD                mVerInfoOSD;
extern TABLE_DATA_STRUCT            mNonlinearMappingTable[];
extern TABLE_DATA_STRUCT            mPictureModeTable[];
extern TABLE_DATA_STRUCT            mColorTempTable[];

extern NonlinearModeType            mDefaultNonlinearMapping;
extern PICTURE_MODE_DATA            mDefaultPictureMode[];
extern COLORTEMP_DATA               mDefaultColorTemp[COLOR_TMP_MODE_MAX];


#define PQ_OSD_BIN_DEFAULT_PATH               "/vendor/etc/tvconfig/pq/pq_osd.bin"

static char PQOsdBinPath[128] = "\0";

PQTableOSD::PQTableOSD()
{
}

PQTableOSD::~PQTableOSD()
{

}

void PQTableOSD::Init()
{
    bool ret = false;
    char PQ_OsdTablePath[128];
    *PQ_OsdTablePath = '\0';

    Get_PQOsdBinName(PQ_OsdTablePath);
    ret = Load_PQOsdBin(PQ_OsdTablePath);

    if (!ret) {
        printf("[%s][%d] Load_PQOsdBin can not get = %s use default table!!!\n",__func__,__LINE__, PQ_OsdTablePath);
        IsLoadFromBinFile(false);
    } else {
        IsLoadFromBinFile(true);
    }

    return;
}

//api
bool PQTableOSD::Set_PQOsdBinPath(char *path)
{
    if (path == NULL) {
        return false;
    }

    sprintf(PQOsdBinPath, "%s", path);

    return true;
}

bool PQTableOSD::SetBrightness(int Data)
{
    PQTable_GetPictureModeData()->Brightness = Data;

    if (ADAP_PQ_SetBrightness(GetNonlinearOsdRemapVal(_BRIGHTNESS, Data)) != ADAP_OK) {
        return false;
    }

    return true;
}

bool PQTableOSD::SetBrightness_OSD(int Data)
{
    PQTable_GetPictureModeData()->Brightness = Data;

    if (ADAP_PQ_SetBrightness_OSD(GetNonlinearOsdRemapVal(_BRIGHTNESS, Data)) != ADAP_OK) {
        return false;
    }

    return true;
}

bool PQTableOSD::GetBrightness(int *pData)
{
    if (pData == NULL)
        return false;

    *pData =  GetNonlinearOsdRemapVal(_BRIGHTNESS, PQTable_GetPictureModeData()->Brightness);

    return true;
}

bool PQTableOSD::SetContrast(int Data)
{
    PQTable_GetPictureModeData()->Contrast = Data;

    if (ADAP_PQ_SetContrast(GetNonlinearOsdRemapVal(_CONTRAST, Data)) != ADAP_OK) {
        return false;
    }

    return true;
}

bool PQTableOSD::SetContrast_OSD(int Data)
{
    PQTable_GetPictureModeData()->Contrast = Data;

    if (ADAP_PQ_SetContrast_OSD(GetNonlinearOsdRemapVal(_CONTRAST, Data)) != ADAP_OK) {
        return false;
    }

    return true;
}

bool PQTableOSD::GetContrast(int *pData)
{
    if (pData == NULL)
        return false;

    *pData =  GetNonlinearOsdRemapVal(_SATURATION, PQTable_GetPictureModeData()->Contrast);

    return true;
}

bool PQTableOSD::SetSaturation(int Data)
{
    PQTable_GetPictureModeData()->Saturation = Data;

    int sat = GetNonlinearOsdRemapVal(_SATURATION, Data);
    int hue = GetNonlinearOsdRemapVal(_HUE, PQTable_GetPictureModeData()->Hue);

    if (ADAP_PQ_SetSaturationHue(sat, hue) != ADAP_OK) {
        LOGE("%s FAIL  Saturation = %d, Hue = %d\n", __FUNCTION__, sat, hue);
        return false;
    }

    return true;
}

bool PQTableOSD::SetSaturation_OSD(int Data)
{
    PQTable_GetPictureModeData()->Saturation = Data;

    int sat = GetNonlinearOsdRemapVal(_SATURATION, Data);
    int hue = GetNonlinearOsdRemapVal(_HUE, PQTable_GetPictureModeData()->Hue);

    if (ADAP_PQ_SetSaturationHue_OSD(sat, hue) != ADAP_OK) {
        LOGE("%s FAIL  Saturation osd = %d, Hue osd= %d\n", __FUNCTION__, sat, hue);
        return false;
    }

    return true;
}

bool PQTableOSD::GetSaturation(int *pData)
{
    if (pData == NULL)
        return false;

    *pData =  GetNonlinearOsdRemapVal(_SATURATION, PQTable_GetPictureModeData()->Saturation);

    return false;
}

bool PQTableOSD::SetHue(int Data)
{
    PQTable_GetPictureModeData()->Hue = Data;

    int sat = GetNonlinearOsdRemapVal(_SATURATION, PQTable_GetPictureModeData()->Saturation);
    int hue = GetNonlinearOsdRemapVal(_HUE, Data);

    if (ADAP_PQ_SetSaturationHue(sat, hue) != ADAP_OK) {
        LOGE("%s FAIL  Hue = %d, Saturation = %d\n", __FUNCTION__, hue, sat);
        return false;
    }

    return true;
}

bool PQTableOSD::SetHue_OSD(int Data)
{
    PQTable_GetPictureModeData()->Hue = Data;

    int sat = GetNonlinearOsdRemapVal(_SATURATION, PQTable_GetPictureModeData()->Saturation);
    int hue = GetNonlinearOsdRemapVal(_HUE, Data);

    if (ADAP_PQ_SetSaturationHue_OSD(sat, hue) != ADAP_OK) {
        LOGE("%s FAIL  Hue = %d, Saturation = %d\n", __FUNCTION__, hue, sat);
        return false;
    }

    return true;
}

bool PQTableOSD::GetHue(int *pData)
{
    if (pData != NULL) {
        *pData =  GetNonlinearOsdRemapVal(_HUE, PQTable_GetPictureModeData()->Hue);
        return true;
    }

    return false;
}

bool PQTableOSD::SetBackLight(int Data)
{
    PQTable_GetPictureModeData()->Hue = Data;

    int backlight = GetNonlinearOsdRemapVal(_BACKLIGHT, Data);

    //nedd ioctrl

    return true;
}

bool PQTableOSD::GetBackLight(int *pData)
{
    if (pData != NULL) {
        *pData =  GetNonlinearOsdRemapVal(_BACKLIGHT, PQTable_GetPictureModeData()->Backlight);
        return true;
    }

    return false;
}

bool PQTableOSD::SetColorTempData(COLORTEMP_DATA *pData)
{
    if (pData == NULL)
        return false;

    if (PQTable_SetColorTempData(pData) != true) {
        LOGE("PQTable_SetColorTempData FAIL \n");
    }

    if (ADAP_PQ_SetColorTemp((vpp_white_balance_s *)pData) != ADAP_OK) {
        LOGE("%s FAIL \n", __FUNCTION__);
        return false;
    }

    return true;
}

bool PQTableOSD::GetColorTempData(COLORTEMP_DATA *pData)
{
    if (pData == NULL)
        return false;

    pData = PQTable_GetColorTempData();

    if (pData != NULL) {
        return true;
    }

    return false;
}

bool PQTableOSD::SetColorTempData_RGain(unsigned int Data)
{
    PQTable_GetColorTempData()->R_val = Data; //save to memory

    if (ADAP_PQ_SetColorTemp((vpp_white_balance_s *)PQTable_GetColorTempData()) != ADAP_OK) {
        LOGE("%s FAIL \n", __FUNCTION__);
        return false;
    }

    return true;
}

bool PQTableOSD::SetColorTempData_GGain(unsigned int Data)
{
    PQTable_GetColorTempData()->G_val = Data;

    if (ADAP_PQ_SetColorTemp((vpp_white_balance_s *)PQTable_GetColorTempData()) != ADAP_OK) {
        LOGE("%s FAIL \n", __FUNCTION__);
        return false;
    }

    return true;
}

bool PQTableOSD::SetColorTempData_BGain(unsigned int Data)
{
    PQTable_GetColorTempData()->B_val = Data; //save to memory

    if (ADAP_PQ_SetColorTemp((vpp_white_balance_s *)PQTable_GetColorTempData()) != ADAP_OK) {
        LOGE("%s FAIL \n", __FUNCTION__);
        return false;
    }

    return true;
}

bool PQTableOSD::SetColorTempData_ROffset(int Data)
{
    PQTable_GetColorTempData()->R_offset_val = Data; //save to memory

    if (ADAP_PQ_SetColorTemp((vpp_white_balance_s *)PQTable_GetColorTempData()) != ADAP_OK) {
        LOGE("%s FAIL \n", __FUNCTION__);
        return false;
    }

    return true;
}

bool PQTableOSD::SetColorTempData_GOffset(int Data)
{
    PQTable_GetColorTempData()->G_offset_val = Data; //save to memory

    if (ADAP_PQ_SetColorTemp((vpp_white_balance_s *)PQTable_GetColorTempData()) != ADAP_OK) {
        LOGE("%s FAIL \n", __FUNCTION__);
        return false;
    }

    return true;
}

bool PQTableOSD::SetColorTempData_BOffset(int Data)
{
    PQTable_GetColorTempData()->B_offset_val = Data; //save to memory

    if (ADAP_PQ_SetColorTemp((vpp_white_balance_s *)PQTable_GetColorTempData()) != ADAP_OK) {
        LOGE("%s FAIL \n", __FUNCTION__);
        return false;
    }

    return true;
}

bool PQTableOSD::SetSharpnessLevel(int Data)
{
    if (PQTable_GetPictureModeData() == NULL)
        return false;

    PQTable_GetPictureModeData()->Sharpness = Data;

    if (PQTable::GetInstance()->Set_VPQ_SharpnessTable(GetNonlinearOsdRemapVal(_SHARPNESS, Data)) != true) {
        return false;
    }

    return true;
}

bool PQTableOSD::GetSharpnessLevel(int *pData)
{
    if (pData == NULL)
        return false;

    PICTURE_MODE_DATA *Params = PQTable_GetPictureModeData();

    *pData =  GetNonlinearOsdRemapVal(_SHARPNESS, Params->Sharpness);

    return false;
}

bool PQTableOSD::SetNonlinearModeType(NonlinearModeType *pData)
{
    if (pData == NULL)
        return false;

    if (PQTable_SetNonlinearModeType(pData) != true) {
        LOGE("%s FAIL \n", __FUNCTION__);
        return false;
    }

    return true;
}

bool PQTableOSD::GetNonlinearModeType(NonlinearModeType *pData)
{
    if (pData != NULL) {
        pData = PQTable_GetNonlinearModeType();
        return true;
    }

    return false;
}

bool PQTableOSD::SetPictureModeData(PICTURE_MODE_DATA *pData)
{
    if (pData != NULL) {
        return false;
    }

    if (PQTable_SetPictureModeData(pData) != true) {
        LOGE("%s FAIL \n", __FUNCTION__);
        return false;
    }

    return true;
}

bool PQTableOSD::GetPictureModeData(PICTURE_MODE_DATA *pData)
{
    if (pData != NULL) {
        pData = PQTable_GetPictureModeData();
        return true;
    }

    return false;
}

bool PQTableOSD::Get_PQOsdBinName(char *name)
{
    if (name == NULL) {
        return false;
    }

    if (strlen(PQOsdBinPath) > 0) {
        strcpy(name, PQOsdBinPath);
    } else {
        strcpy(name, PQ_OSD_BIN_DEFAULT_PATH);
    }

    return true;
}

bool PQTableOSD::Load_PQOsdBin(char *name)
{
    if (name == NULL) {
        return false;
    }

    printf("%s name = %s\n", __func__, name);

    FILE *pFILE = NULL;
    pFILE = fopen(name, "r");

    if (pFILE == NULL) {
        return false;
    }

    PQ_OSD_TABLE_STRUCT m_PQOsdTable_t[PQ_OSD_TABLE_MAX];
    memset(m_PQOsdTable_t, 0, sizeof(PQ_OSD_TABLE_STRUCT));

    for (int i = PQ_OSD_TABLE_VERSION; i < PQ_OSD_TABLE_MAX; i++) {
        if (PQOSD_TableLoader_GetTable(pFILE, (PQ_OSD_TABLE_TYPE)i, &m_PQOsdTable_t[i]) == false) {
            printf("%s index: %d\n", __func__, i);
            fclose(pFILE);
            return false;
        }
    }

    if (xLoadTableDataToMem(m_PQOsdTable_t) == false) {
        fclose(pFILE);
        return false;
    }

    fclose(pFILE);
    return true;
}

void* PQTableOSD::GetTableData(PQ_OSD_TABLE_TYPE type, unsigned int *indexTabLen)
{
    if (type > PQ_OSD_TABLE_MAX || indexTabLen == NULL)
        return NULL;

    return GetPQOSDTableData(type, indexTabLen);
}

PQTableOSD *PQTableOSD::mInstance = NULL;
PQTableOSD *PQTableOSD::GetInstance()
{
    if (NULL == mInstance) {
        mInstance = new PQTableOSD();
    }

    PQTable::GetInstance()->Init();

    return mInstance;
}
