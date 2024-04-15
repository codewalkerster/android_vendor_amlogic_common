
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "PQTableOSD.h"

extern TABLE_VER_OSD                mVerInfoOSD;
extern TABLE_DATA_STRUCT            mNonlinearMappingTable[];
extern TABLE_DATA_STRUCT            mPictureModeTable[];
extern TABLE_DATA_STRUCT            mColorTempTable[];
extern TABLE_DATA_STRUCT            mColorCustomizeTable[];

extern NonlinearModeType            mDefaultNonlinearMapping;
extern PICTURE_MODE_DATA            mDefaultPictureMode[];
extern COLORTEMP_DATA               mDefaultColorTemp[COLOR_TMP_MODE_MAX];
extern TABLE_CMS                    mDefaultCmsTable;
extern TABLE_PICTURE_SETTING_EXT    mPictureSettingTable;

static bool                         IsloadFromOsdBinFile = false;


#define PQ_OSD_BIN_DEFAULT_PATH               "/mnt/vendor/param/pq/UI_PQSetting.bin"

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
        IsloadFromOsdBinFile = false;
    } else {
        IsloadFromOsdBinFile = true;
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

PICTURE_MODE_DATA* PQTableOSD::GetPictureModeData(void)
{
    return &mDefaultPictureMode[0];
}

bool PQTableOSD::SetPictureModeData(PICTURE_MODE_DATA* pData)
{
    memcpy(&mDefaultPictureMode[0], pData, sizeof(PICTURE_MODE_DATA));
    return true;
}

NonlinearModeType* PQTableOSD::GetNonlinearModeType(void)
{
    return &mDefaultNonlinearMapping;
}

bool PQTableOSD::SetNonlinearModeType(NonlinearModeType *pData)
{
    memcpy(&mDefaultNonlinearMapping, pData, sizeof(NonlinearModeType));
    return true;
}

COLORTEMP_DATA* PQTableOSD::GetColorTempData(void)
{
    return &mDefaultColorTemp[0];
}

bool PQTableOSD::SetColorTempData(COLORTEMP_DATA *pData)
{
    memcpy(&mDefaultColorTemp[0], pData, sizeof(COLORTEMP_DATA));
    return true;
}

bool PQTableOSD::SetColorTempGammaData(GAMMA_OFFSET_DATA *pData)
{
    memcpy(&mDefaultColorTemp[0].GammaOffset, pData, sizeof(GAMMA_OFFSET_DATA));
    return true;
}

GAMMA_OFFSET_DATA* PQTableOSD::GetColorTempGammaData(void)
{
    return &mDefaultColorTemp[0].GammaOffset;
}

bool PQTableOSD::SetColorCustomizeData(TABLE_CMS *pData)
{
    memcpy(&mDefaultCmsTable, pData, sizeof(TABLE_CMS));
    return true;
}

TABLE_CMS* PQTableOSD::GetColorCustomizeData(void)
{
    return &mDefaultCmsTable;
}

TABLE_PICTURE_SETTING_EXT* PQTableOSD::GetPictureSettingExtData(void)
{
    return &mPictureSettingTable;
}

bool PQTableOSD::SetPictureSettingExtData(TABLE_PICTURE_SETTING_EXT* pData)
{
    memcpy(&mPictureSettingTable, pData, sizeof(TABLE_PICTURE_SETTING_EXT));
    return true;
}

int PQTableOSD::GetNonlinearOsdRemapVal(nonline_params_type_t type, int Value)
{
    int temp = 0;
    unsigned char startPoint = 0, endPoint = 0;
    unsigned char point0 = 0;
    unsigned char point25 = 0;
    unsigned char point50 = 0;
    unsigned char point75 = 0;
    unsigned char point100 = 0;

    switch (type) {
        case _BRIGHTNESS:
            point0   = mDefaultNonlinearMapping.Brightness_0;
            point25  = mDefaultNonlinearMapping.Brightness_25;
            point50  = mDefaultNonlinearMapping.Brightness_50;
            point75  = mDefaultNonlinearMapping.Brightness_75;
            point100 = mDefaultNonlinearMapping.Brightness_100;
            break;
        case _CONTRAST:
            point0   = mDefaultNonlinearMapping.Contrast_0;
            point25  = mDefaultNonlinearMapping.Contrast_25;
            point50  = mDefaultNonlinearMapping.Contrast_50;
            point75  = mDefaultNonlinearMapping.Contrast_75;
            point100 = mDefaultNonlinearMapping.Contrast_100;
            break;
        case _SATURATION:
            point0   = mDefaultNonlinearMapping.Saturation_0;
            point25  = mDefaultNonlinearMapping.Saturation_25;
            point50  = mDefaultNonlinearMapping.Saturation_50;
            point75  = mDefaultNonlinearMapping.Saturation_75;
            point100 = mDefaultNonlinearMapping.Saturation_100;
            break;
        case _HUE:
            point0   = mDefaultNonlinearMapping.Hue_0;
            point25  = mDefaultNonlinearMapping.Hue_25;
            point50  = mDefaultNonlinearMapping.Hue_50;
            point75  = mDefaultNonlinearMapping.Hue_75;
            point100 = mDefaultNonlinearMapping.Hue_100;
            break;
        case _SHARPNESS:
            point0   = mDefaultNonlinearMapping.Sharpness_0;
            point25  = mDefaultNonlinearMapping.Sharpness_25;
            point50  = mDefaultNonlinearMapping.Sharpness_50;
            point75  = mDefaultNonlinearMapping.Sharpness_75;
            point100 = mDefaultNonlinearMapping.Sharpness_100;
            break;
        case _BACKLIGHT:
            point0   = mDefaultNonlinearMapping.Backlight_0;
            point25  = mDefaultNonlinearMapping.Backlight_25;
            point50  = mDefaultNonlinearMapping.Backlight_50;
            point75  = mDefaultNonlinearMapping.Backlight_75;
            point100 = mDefaultNonlinearMapping.Backlight_100;
            break;
    default:
            break;
    }

    if (Value < 25) {
        startPoint = point0;
        endPoint = point25;
        temp = Value;
    } else if ((Value >= 25) && (Value < 50)) {
        startPoint = point25;
        endPoint = point50;
        temp = Value - 25;
    } else if ((Value >= 50) && (Value < 75)) {
        startPoint = point50;
        endPoint = point75;
        temp = Value - 50;
    } else if (Value >= 75) {
        startPoint = point75;
        endPoint = point100;
        temp = Value - 75;
    }

    if (endPoint >= startPoint) {
        temp = (endPoint - startPoint) * temp / 25;
        temp +=  startPoint;
    } else if ((endPoint < startPoint)) {
        temp = (startPoint-endPoint) * temp / 25;
        temp = startPoint - temp;
    }

    return temp;
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

    memset(m_PQOsdTable, 0, sizeof(PQ_OSD_TABLE_STRUCT));

    for (int i = PQ_OSD_TABLE_VERSION; i < PQ_OSD_TABLE_MAX; i++) {
        if (PQOSD_TableLoader_GetTable(pFILE, (PQ_OSD_TABLE_TYPE)i, &m_PQOsdTable[i]) == false) {
            fclose(pFILE);
            return false;
        }
    }

    if (xLoadTableDataToMem(m_PQOsdTable) == false) {
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

TABLE_DATA_STRUCT* PQTableOSD::GetDefaultNonlinearData(void)
{
    return mNonlinearMappingTable;
}

TABLE_DATA_STRUCT* PQTableOSD::GetDefaultPictureModeData(void)
{
    return mPictureModeTable;
}

TABLE_DATA_STRUCT* PQTableOSD::GetDefaultColorTempData(void)
{
    return mColorTempTable;
}

PQTableOSD *PQTableOSD::mInstance = NULL;
PQTableOSD *PQTableOSD::GetInstance()
{
    if (NULL == mInstance) {
        mInstance = new PQTableOSD();
    }

    return mInstance;
}
