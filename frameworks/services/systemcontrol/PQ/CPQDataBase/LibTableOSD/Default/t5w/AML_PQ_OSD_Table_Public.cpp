#include "PQTableTypeOSD.h"

TABLE_VER_OSD mVerInfoOSD =
{
    "AML_Public_PQ_OSD_20240513",   /*ProjectVersion*/
    "AML_Public_T5W",               /*ChipVersion*/
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
    {PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, (void *)&mDefaultNonlinearMapping, sizeof(mDefaultNonlinearMapping) / sizeof(NonlinearModeType)},
};
/* NonlinearMapping end*/

/* Picture Mode start*/
PICTURE_MODE_DATA mDefaultPictureMode[] = {
    /*pqmode                         bri con sat hue shp bl     nr    dnlp   gd     gamut colortem                 lc     black blue  mnr   deb   dem   Cor   memc   Deco  sr    gamma tmo    dv_mod  dv_dd  dv_ls  Prcision*/
    {PICTURE_MODE_STANDARD,          50, 50, 50, 50, 50, _NULL, _MID, _MID,  _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _MID,  _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _LOW, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_BRIGHT,            50, 50, 60, 50, 60, _NULL, _MID, _HIGH, _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _HIGH, _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_SOFT,              50, 45, 60, 50, 50, _NULL, _MID, _MID,  _OFF,  _OFF, COLOR_TMP_MODE_WARM,     _HIGH, _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_USER,              45, 40, 40, 50, 25, _NULL, _MID, _LOW,  _OFF,  _OFF, COLOR_TMP_MODE_USER,     _HIGH, _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_MOVIE,             50, 40, 50, 50, 50, _NULL, _MID, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_WARM,     _HIGH, _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_COLORFUL,          50, 40, 50, 50, 50, _NULL, _MID, _HIGH, _OFF,  _OFF, COLOR_TMP_MODE_COLD,     _HIGH, _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_MONITOR,           50, 40, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _HIGH, _OFF, _OFF, _OFF, _OFF, _OFF, _LOW, _OFF,  _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_GAME,              50, 50, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _HIGH, _OFF, _OFF, _OFF, _OFF, _OFF, _LOW, _OFF,  _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_SPORTS,            48, 50, 45, 50, 50, _NULL, _MID, _HIGH, _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _HIGH, _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_SONY,              50, 50, 50, 50, 50, _NULL, _MID, _MID,  _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _MID,  _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_SAMSUNG,           50, 50, 50, 50, 50, _NULL, _MID, _MID,  _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _MID,  _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_SHARP,             50, 50, 50, 50, 50, _NULL, _MID, _MID,  _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _MID,  _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_AMDOLBY_DARK,      50, 50, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_WARMER,   _OFF,  _OFF, _OFF, _OFF, _OFF, _OFF, _LOW, _OFF,  _OFF, _OFF, 6,    _NULL, 0,      0,     0,     0,     0},
    {PICTURE_MODE_AMDOLBY_BRIGHT,    50, 50, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_WARMER,   _OFF,  _OFF, _OFF, _OFF, _OFF, _OFF, _LOW, _OFF,  _OFF, _OFF, 6,    _NULL, 1,      1,     0,     1,     0},
    {PICTURE_MODE_AMDOLBY_IQ,        50, 50, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_WARMER,   _OFF,  _OFF, _OFF, _OFF, _OFF, _OFF, _LOW, _OFF,  _OFF, _OFF, 6,    _NULL, 3,      0,     1,     1,     0},
    {PICTURE_MODE_FILMMAKER,         50, 50, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_WARMER,   _OFF,  _OFF, _OFF, _OFF, _OFF, _OFF, _LOW, _OFF,  _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, 0,     _NULL, 0},
};

PICTURE_MODE_DATA mDVPictureMode[] = {
    /*pqmode                         bri con sat hue shp bl     nr    dnlp   gd     gamut colortem                 lc     black blue  mnr   deb   dem   Cor   memc   Deco  sr    gamma tmo    dv_mod  dv_dd  dv_ls  Prcision*/
    {PICTURE_MODE_AMDOLBY_DARK,      50, 50, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_WARMER,   _OFF,  _OFF, _OFF, _OFF, _OFF, _OFF, _OFF, _OFF,  _OFF, _OFF, 6,    _NULL, 0,      0,     0,     0,     0},
    {PICTURE_MODE_AMDOLBY_BRIGHT,    50, 50, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_WARMER,   _OFF,  _OFF, _OFF, _OFF, _OFF, _OFF, _OFF, _OFF,  _OFF, _OFF, 6,    _NULL, 1,      1,     0,     1,     0},
    {PICTURE_MODE_GAME,              50, 50, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_WARMER,   _OFF,  _OFF, _OFF, _OFF, _OFF, _OFF, _OFF, _OFF,  _OFF, _OFF, 6,    _NULL, 2,      0,     0,     0,     0},
    {PICTURE_MODE_AMDOLBY_IQ,        50, 50, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_WARMER,   _OFF,  _OFF, _OFF, _OFF, _OFF, _OFF, _OFF, _OFF,  _OFF, _OFF, 6,    _NULL, 3,      0,     1,     1,     0},
    {PICTURE_MODE_FILMMAKER,         50, 50, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_WARMER,   _OFF,  _OFF, _OFF, _OFF, _OFF, _OFF, _OFF, _OFF,  _OFF, _OFF, 6,    _NULL, _NULL,  _NULL, _NULL, _NULL, 0},
};

PICTURE_MODE_DATA mHDR10PictureMode[] = {
    /*pqmode                         bri con sat hue shp bl     nr    dnlp   gd     gamut colortem                 lc     black blue  mnr   deb   dem   Cor   memc   Deco  sr    gamma tmo    dv_mod  dv_dd  dv_ls  Prcision*/
    {PICTURE_MODE_STANDARD,          50, 50, 50, 50, 50, _NULL, _MID, _MID,  _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _MID,  _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _LOW, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_BRIGHT,            50, 50, 60, 50, 60, _NULL, _MID, _HIGH, _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _HIGH, _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_SOFT,              50, 45, 60, 50, 50, _NULL, _MID, _MID,  _OFF,  _OFF, COLOR_TMP_MODE_WARM,     _HIGH, _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_USER,              45, 40, 40, 50, 25, _NULL, _MID, _LOW,  _OFF,  _OFF, COLOR_TMP_MODE_USER,     _HIGH, _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_MOVIE,             50, 40, 50, 50, 50, _NULL, _MID, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_WARM,     _HIGH, _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_COLORFUL,          50, 40, 50, 50, 50, _NULL, _MID, _HIGH, _OFF,  _OFF, COLOR_TMP_MODE_COLD,     _HIGH, _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_MONITOR,           50, 40, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _HIGH, _OFF, _OFF, _OFF, _OFF, _OFF, _LOW, _OFF,  _OFF, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_GAME,              50, 50, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _HIGH, _OFF, _OFF, _OFF, _OFF, _OFF, _LOW, _OFF,  _OFF, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_SPORTS,            48, 50, 45, 50, 50, _NULL, _MID, _HIGH, _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _HIGH, _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_SONY,              50, 50, 50, 50, 50, _NULL, _MID, _MID,  _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _MID,  _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_SAMSUNG,           50, 50, 50, 50, 50, _NULL, _MID, _MID,  _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _MID,  _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_SHARP,             50, 50, 50, 50, 50, _NULL, _MID, _MID,  _OFF,  _OFF, COLOR_TMP_MODE_STANDARD, _MID,  _OFF, _OFF, _LOW, _LOW, _LOW, _LOW, _HIGH, _OFF, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
    {PICTURE_MODE_FILMMAKER,         50, 50, 50, 50, 50, _NULL, _OFF, _OFF,  _OFF,  _OFF, COLOR_TMP_MODE_WARMER,   _OFF,  _OFF, _OFF, _OFF, _OFF, _OFF, _LOW, _OFF,  _OFF, _OFF, 6,    1,     _NULL,  _NULL, _NULL, _NULL, 0},
};

TABLE_DATA_STRUCT mPictureModeTable[] = {
    {PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, (void *)&mDefaultPictureMode[0], sizeof(mDefaultPictureMode) / sizeof(PICTURE_MODE_DATA)},
    {PQ_SRC_HDMI1,   PQ_SIGFMT_DV,      (void *)&mDVPictureMode[0],      sizeof(mDVPictureMode) / sizeof(PICTURE_MODE_DATA)},
    {PQ_SRC_HDMI2,   PQ_SIGFMT_DV,      (void *)&mDVPictureMode[0],      sizeof(mDVPictureMode) / sizeof(PICTURE_MODE_DATA)},
    {PQ_SRC_HDMI3,   PQ_SIGFMT_DV,      (void *)&mDVPictureMode[0],      sizeof(mDVPictureMode) / sizeof(PICTURE_MODE_DATA)},
    {PQ_SRC_HDMI4,   PQ_SIGFMT_DV,      (void *)&mDVPictureMode[0],      sizeof(mDVPictureMode) / sizeof(PICTURE_MODE_DATA)},
    {PQ_SRC_MPEG,    PQ_SIGFMT_DV,      (void *)&mDVPictureMode[0],      sizeof(mDVPictureMode) / sizeof(PICTURE_MODE_DATA)},
    {PQ_SRC_DTV,     PQ_SIGFMT_DV,      (void *)&mDVPictureMode[0],      sizeof(mDVPictureMode) / sizeof(PICTURE_MODE_DATA)},
    {PQ_SRC_HDMI1,   PQ_SIGFMT_HDR,     (void *)&mHDR10PictureMode[0],   sizeof(mHDR10PictureMode) / sizeof(PICTURE_MODE_DATA)},
    {PQ_SRC_HDMI2,   PQ_SIGFMT_HDR,     (void *)&mHDR10PictureMode[0],   sizeof(mHDR10PictureMode) / sizeof(PICTURE_MODE_DATA)},
    {PQ_SRC_HDMI3,   PQ_SIGFMT_HDR,     (void *)&mHDR10PictureMode[0],   sizeof(mHDR10PictureMode) / sizeof(PICTURE_MODE_DATA)},
    {PQ_SRC_HDMI4,   PQ_SIGFMT_HDR,     (void *)&mHDR10PictureMode[0],   sizeof(mHDR10PictureMode) / sizeof(PICTURE_MODE_DATA)},
    {PQ_SRC_MPEG,    PQ_SIGFMT_HDR,     (void *)&mHDR10PictureMode[0],   sizeof(mHDR10PictureMode) / sizeof(PICTURE_MODE_DATA)},
    {PQ_SRC_DTV,     PQ_SIGFMT_HDR,     (void *)&mHDR10PictureMode[0],   sizeof(mHDR10PictureMode) / sizeof(PICTURE_MODE_DATA)},
};
/* Picture Mode end*/

/*Color Temperature start*/
//20 point
#define MANUAL_GAMMA {\
    _ON,\
    WB_GAMMA_MODE_10POINT,\
    {\
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},\
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},\
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},\
    },\
}

//                        RGO GGO BGO ROO GOO BOO
#define MANUAL_COLORTEMP {0,  0,  0,  0,  0,  0}

COLORTEMP_DATA mDefaultColorTemp[COLOR_TMP_MODE_MAX] = {
//             RG    GG    BG    RO GO BO   GAMMA_OFFSET  COLORTEMP_OFFSET  Gammaindex
/* NORMAL */ {{1040, 980,  1030, 0, 0, 0,}, MANUAL_GAMMA, MANUAL_COLORTEMP, 0,},
/* WARM   */ {{1040, 900,  900,  0, 0, 0,}, MANUAL_GAMMA, MANUAL_COLORTEMP, 0,},
/* COOL   */ {{1060, 980,  1080, 0, 0, 0,}, MANUAL_GAMMA, MANUAL_COLORTEMP, 0,},
/* WARMER */ {{1040, 900,  900,  0, 0, 0,}, MANUAL_GAMMA, MANUAL_COLORTEMP, 0,},
/* COOLER */ {{1060, 1024, 1080, 0, 0, 0,}, MANUAL_GAMMA, MANUAL_COLORTEMP, 0,},
/* USER   */ {{1024, 1024, 1024, 0, 0, 0,}, MANUAL_GAMMA, MANUAL_COLORTEMP, 0,},
};

TABLE_DATA_STRUCT mColorTempTable[] = {
    {PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, (void *)&mDefaultColorTemp[0], sizeof(mDefaultColorTemp) / sizeof(COLORTEMP_DATA)},
};
/*Color Temperature end*/

/*Color Customize start*/
TABLE_CMS mDefaultCmsTable = {
/*CMS Enable*/    _ON,
{
/*color      Saturation  Hue  Luma*/
/*Red           */ {0,   0,   0},
/*Green         */ {0,   0,   0},
/*Blue          */ {0,   0,   0},
/*Cyan          */ {0,   0,   0},
/*Magenta       */ {0,   0,   0},
/*Yellow        */ {0,   0,   0},
/*Skin          */ {0,   0,   0},
/*Yellow Green  */ {0,   0,   0},
/*Blue Green    */ {0,   0,   0},
},
};

TABLE_DATA_STRUCT mColorCustomizeTable[] = {
    {PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, (void *)&mDefaultCmsTable, sizeof(mDefaultCmsTable) / sizeof(TABLE_CMS)},
};

/*Color Customize end*/

//other ui setting
TABLE_PICTURE_SETTING_EXT mPictureSettingTable = {
//Default Picture mode
{
/*Picture       */        PICTURE_MODE_STANDARD,
/*Picture_DV    */        PICTURE_MODE_AMDOLBY_BRIGHT,
/*Picture_HDR   */        PICTURE_MODE_STANDARD,
/*Picture_HLG   */        PICTURE_MODE_STANDARD,
/*Picture_HDRP  */        PICTURE_MODE_STANDARD,
},
//by src
{
{
/*MEMC                    DeJudder   DeBlur*/
/*OFF           */        {0,        0},
/*LOW           */        {3,        3},
/*MID           */        {6,        6},
/*HIGH          */        {10,       10},
},

/*McDiMode      */        1,
/*DisplayMode   */        0,
},

// GLOBAL
{
/*ColorBase     */        1,
/*AIPQ_ENABLE   */        _OFF,
/*AIPQ_MODE     */        0,
/*AISR_ENABLE   */        _OFF,
/*AISR_MODE     */        3,
/*EyeProtection */        _OFF,
/*LocalDimming  */        _OFF,
/*ai_color      */        0,
/*FilmMakerEnable*/       _OFF,
/*Sdr2Hdr        */       _OFF,
/*BackLight               dis  dis1 dis2*/
                         {100, 100, 100,},
},
};

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

int GetColorCustomizeTableSize(void)
{
    return sizeof(mColorCustomizeTable)/sizeof(TABLE_DATA_STRUCT);
}
