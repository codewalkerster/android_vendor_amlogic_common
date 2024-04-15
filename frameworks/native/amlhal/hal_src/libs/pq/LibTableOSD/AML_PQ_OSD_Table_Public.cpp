#include "PQTableTypeOSD.h"

TABLE_VER_OSD mVerInfoOSD =
{
    "AML_20221008",                 /*ProjectVersion*/
    "Public",                       /*ChipVersion*/
    "AML_TV_PICTURE",               /*TableVersion*/
    "None",                         /*oem_model*/
    "None",                         /*PanelIndex*/
    ""                              /*reserved*/
};

/* NonlinearMapping start*/
NonlinearModeType mDefaultNonlinearMapping = {
/* osd0 osd25 osd50 osd75 osd100*/
    0,  64,   128,  192,  255,    //_BRIGHTNESS 0~255
    0,  64,   128,  192,  255,    //_CONTRAST   0~255
    0,  64,   128,  192,  255,    //_SATURATION 0~255
    0,  64,   128,  192,  255,    //_HUE        0~255
    0,  64,   128,  192,  255,    //_SHARPNESS  0~255
    0,  64,   128,  192,  255,    //_BACKLIGHT  0~255
};

TABLE_DATA_STRUCT mNonlinearMappingTable[] = {
    {PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, (void *) &mDefaultNonlinearMapping, sizeof(mDefaultNonlinearMapping) / sizeof(NonlinearModeType)},
};
/* NonlinearMapping end*/

/* Picture Mode start*/
PICTURE_MODE_DATA mDefaultPictureMode[] = {
    /*pqmode                bri con sta hue shp bl   nr dnlp       xvycc colortemp                lc black blue mnr cc memc DVmode DVdarkdetail */
    {PICTURE_MODE_STANDARD, 50, 50, 50, 50, 50, 100, 2, DNLP_HIGH, 0,    COLOR_TMP_MODE_STANDARD, 3, 0,     0,  1,  0, 0,   0,     0},
    {PICTURE_MODE_BRIGHT,   50, 50, 70, 50, 60, 100, 2, DNLP_HIGH, 0,    COLOR_TMP_MODE_STANDARD, 3, 0,     0,  1,  0, 0,   0,     0},
    {PICTURE_MODE_SOFT,     50, 45, 60, 50, 50, 100, 2, DNLP_MID,  0,    COLOR_TMP_MODE_WARM,     3, 0,     0,  1,  0, 0,   0,     0},
    {PICTURE_MODE_USER,     45, 40, 40, 50, 25, 90,  2, DNLP_LOW,  0,    COLOR_TMP_MODE_USER,     3, 0,     0,  1,  0, 0,   0,     0},
    {PICTURE_MODE_MOVIE,    50, 40, 50, 50, 0,  50,  2, DNLP_OFF,  0,    COLOR_TMP_MODE_WARM,     3, 0,     0,  1,  0, 0,   0,     0},
    {PICTURE_MODE_COLORFUL, 50, 40, 50, 50, 50, 80,  2, DNLP_HIGH, 0,    COLOR_TMP_MODE_COLD,     3, 0,     0,  1,  0, 0,   0,     0},
    {PICTURE_MODE_MONITOR,  50, 40, 50, 50, 50, 70,  0, DNLP_OFF,  0,    COLOR_TMP_MODE_STANDARD, 0, 0,     0,  0,  0, 0,   0,     0},
    {PICTURE_MODE_GAME,     50, 50, 50, 50, 50, 100, 0, DNLP_OFF,  0,    COLOR_TMP_MODE_STANDARD, 0, 0,     0,  0,  0, 0,   0,     0},
    {PICTURE_MODE_SPORTS,   50, 50, 70, 50, 60, 100, 2, DNLP_HIGH, 0,    COLOR_TMP_MODE_STANDARD, 3, 0,     0,  1,  0, 0,   0,     0},
    {PICTURE_MODE_SONY,     50, 50, 50, 50, 50, 100, 2, DNLP_MID,  0,    COLOR_TMP_MODE_STANDARD, 2, 0,     0,  1,  0, 0,   0,     0},
    {PICTURE_MODE_SAMSUNG,  50, 50, 50, 50, 50, 100, 2, DNLP_MID,  0,    COLOR_TMP_MODE_STANDARD, 2, 0,     0,  1,  0, 0,   0,     0},
    {PICTURE_MODE_SHARP,    50, 50, 50, 50, 50, 100, 2, DNLP_MID,  0,    COLOR_TMP_MODE_STANDARD, 2, 0,     0,  1,  0, 0,   0,     0},
    {PICTURE_MODE_AMDOLBY_BRIGHT,50, 50, 50, 50, 50, 100, 0, DNLP_OFF,  0,    COLOR_TMP_MODE_STANDARD, 0, 0,     0,  0,  0, 0,   0,     0},
    {PICTURE_MODE_AMDOLBY_DARK,  50, 50, 50, 50, 50, 100, 0, DNLP_OFF,  0,    COLOR_TMP_MODE_STANDARD, 0, 0,     0,  0,  0, 0,   1,     0},
};


TABLE_DATA_STRUCT mPictureModeTable[] = {
    {PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, (void *) &mDefaultPictureMode[0], sizeof(mDefaultPictureMode) / sizeof(PICTURE_MODE_DATA) },
};
/* Picture Mode end*/

/* Color Temperature start*/
#define MANUAL_GAMMA {\
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},\
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},\
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}

COLORTEMP_DATA mDefaultColorTemp[COLOR_TMP_MODE_MAX] = {
    /*RG   GG    BG    RO GO BO MANUAL_GA     GAMMA*/
    {1040, 980,  1030, 0, 0, 0, MANUAL_GAMMA, 0,},/*NORMAL*/
    {1040, 900,  900,  0, 0, 0, MANUAL_GAMMA, 0,},/*WARM*/
    {1060, 980,  1080, 0, 0, 0, MANUAL_GAMMA, 0,},/*COOL*/
    {1024, 1024, 1024, 0, 0, 0, MANUAL_GAMMA, 0,},/*USER*/
};

TABLE_DATA_STRUCT mColorTempTable[] = {
    {PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, (void *) &mDefaultColorTemp[0], sizeof(mDefaultColorTemp) / sizeof(COLORTEMP_DATA)},
};
/* Color Temperature end*/

int GetNonlinearMappingTableSize(void)
{
    return sizeof(mNonlinearMappingTable)/sizeof(TABLE_DATA_STRUCT);
}

int GetColorTempTableSize(void)
{
    return sizeof(mColorTempTable)/sizeof(TABLE_DATA_STRUCT);
}

int GetPictureModeTableSize(void)
{
    return sizeof(mPictureModeTable)/sizeof(TABLE_DATA_STRUCT);
}
