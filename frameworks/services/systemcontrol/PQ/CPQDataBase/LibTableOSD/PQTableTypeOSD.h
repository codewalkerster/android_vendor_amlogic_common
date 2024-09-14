#ifndef _PQ_TABLE_TYPE_OSD_H_
#define _PQ_TABLE_TYPE_OSD_H_


#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NULL
#define _NULL                    (-1)
#endif

#ifndef _OFF
#define _OFF                     (0)
#endif

#ifndef _ON
#define _ON                      (1)
#endif

#ifndef _LOW
#define _LOW                     (1)
#endif

#ifndef _MID
#define _MID                     (2)
#endif

#ifndef _HIGH
#define _HIGH                    (3)
#endif

#ifndef _AUTO
#define _AUTO                    (4)
#endif

/*-----------------------------------------------------------------------------*/
/* PQ OSD Version*/
/*-----------------------------------------------------------------------------*/
typedef struct _TABLE_VER_OSD
{
    char ProjectVersion[64];
    char ChipVersion[64];
    char TableVersion[64];
    char oem_model[64];
    char PanelIndex[64];
    char reserved[64];
} TABLE_VER_OSD;

/*-----------------------------------------------------------------------------*/
/* Nonlinear MAP*/
/*-----------------------------------------------------------------------------*/
typedef struct {
    unsigned char Brightness_0;
    unsigned char Brightness_25;
    unsigned char Brightness_50;
    unsigned char Brightness_75;
    unsigned char Brightness_100;
    unsigned char Contrast_0;
    unsigned char Contrast_25;
    unsigned char Contrast_50;
    unsigned char Contrast_75;
    unsigned char Contrast_100;
    unsigned char Saturation_0;
    unsigned char Saturation_25;
    unsigned char Saturation_50;
    unsigned char Saturation_75;
    unsigned char Saturation_100;
    unsigned char Hue_0;
    unsigned char Hue_25;
    unsigned char Hue_50;
    unsigned char Hue_75;
    unsigned char Hue_100;
    unsigned char Sharpness_0;
    unsigned char Sharpness_25;
    unsigned char Sharpness_50;
    unsigned char Sharpness_75;
    unsigned char Sharpness_100;
    unsigned char Backlight_0;
    unsigned char Backlight_25;
    unsigned char Backlight_50;
    unsigned char Backlight_75;
    unsigned char Backlight_100;
} NonlinearModeType;

/*-----------------------------------------------------------------------------*/
/* Picture  Mode*/
/*-----------------------------------------------------------------------------*/
typedef enum _PICTURE_MODE {
    PICTURE_MODE_STANDARD = 0,
    PICTURE_MODE_BRIGHT,
    PICTURE_MODE_SOFT,
    PICTURE_MODE_USER,
    PICTURE_MODE_MOVIE,
    PICTURE_MODE_COLORFUL,
    PICTURE_MODE_MONITOR,
    PICTURE_MODE_GAME,
    PICTURE_MODE_SPORTS,
    PICTURE_MODE_SONY,
    PICTURE_MODE_SAMSUNG,
    PICTURE_MODE_SHARP,
    PICTURE_MODE_AMDOLBY_DARK,
    PICTURE_MODE_AMDOLBY_BRIGHT,
    PICTURE_MODE_AMDOLBY_IQ,
    PICTURE_MODE_AMDOLBY_PRECISION,
    PICTURE_MODE_FILMMAKER,
    PICTURE_MODE_MAX,
} PICTURE_MODE;

typedef struct _PICTURE_MODE_DATA {
    PICTURE_MODE  mode;

    int Brightness;
    int Contrast;
    int Saturation;
    int Hue;
    int Sharpness;
    int Backlight;
    int Nr;
    int DynamicContrast;
    int DynamicBacklight;
    int ColorGamut;
    int ColorTemperature;
    int LocalContrast;
    int BlackStretch;
    int BlueStretch;
    int MpegNr;
    int Deblock;
    int DeMoSquito;
    int ChromaCoring;
    int Memc;
    int Decontour;
    int SuperResolution;
    int GammaMidLuminance;
    int HdrTmo;
    int DvMode;
    int DvDarkDetail;
    int DvLightSensor;
    int AmDolbyPrcision;
    int RESERVED1;
}PICTURE_MODE_DATA;

typedef struct _MEMC_LEVEL_STRUCT {
    int DeJudderLevel;
    int DeBlurLevel;
}MEMC_LEVEL_STRUCT;

typedef enum _MEMC_MODE {
    MEMC_MODE_OFF,
    MEMC_MODE_LOW,
    MEMC_MODE_MID,
    MEMC_MODE_HIGH,
    MEMC_MODE_USER,
    MEMC_MODE_MAX,
} MEMC_MODE;

typedef struct _PICTURE_MODE_DEFAULT {
    int Picture;
    int Picture_AMDOLBY;
    int Picture_HDR;
    int Picture_HLG;
    int Picture_HDRP;
}PICTURE_MODE_DEFAULT;

typedef struct _PICTURE_SETTING_BY_SRC {
    MEMC_LEVEL_STRUCT memc[MEMC_MODE_MAX];
    int McDiMode;
    int DisplayMode;
}PICTURE_SETTING_BY_SRC;

typedef enum _AMDOLBY_APO_TYPE {
    AMDOLBY_APOO_TYPE_0 = 0,
    AMDOLBY_APOO_TYPE_1,
    AMDOLBY_APOO_TYPE_2,
    AMDOLBY_APOO_TYPE_3,
    AMDOLBY_APOO_TYPE_4,
    AMDOLBY_APOO_TYPE_MAX,
} AMDOLBY_APO_TYPE;

typedef struct _AMDOLBY_IQ_APO_STRUCT {
    int Sharp;
    int Sr;
    int Memc;
    int Nr;
}AMDOLBY_IQ_APO_STRUCT;

typedef struct _BACKLIGHT_STRUCT {
    int val_display;
    int val_display1;
    int val_display2;
}BACKLIGHT_STRUCT;

typedef struct _PICTURE_SETTING_GLOBAL {
    int colorbase;
    int aipq_enable;
    int aipq_mode;
    int aisr_enable;
    int aisr_mode;
    int EyeProtection;
    int LocalDimming;
    int ai_color;
    int FilmMakerEnable;
    int Sdr2Hdr;
    int osd_sharpness;
    BACKLIGHT_STRUCT Backlight;
}PICTURE_SETTING_GLOBAL;

typedef struct _TABLE_PICTURE_SETTING_EXT {
    PICTURE_MODE_DEFAULT PictureModeParam;
    PICTURE_SETTING_BY_SRC BySrcParam;
    PICTURE_SETTING_GLOBAL ByGlobalParam;
}TABLE_PICTURE_SETTING_EXT;

typedef enum Color_Node_Type_e
{
    CM_9_COLOR = 0,
    CM_14_COLOR,
    CM_MAX_COLOR,
}Color_Node_type_t;

typedef enum _CMS_COLOR {
    COLOR_RED = 0,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_CYAN,
    COLOR_MAGENTA,
    COLOR_YELLOW,
    COLOR_SKIN,
    COLOR_YELLOW_GREEN,
    COLOR_BLUE_GREEN,
    COLOR_MAX,
} CMS_COLOR;

typedef enum _CMS_14_COLOR
{
    COLOR_14_BLUE_PURPLE=  0,
    COLOR_14_PURPLE,
    COLOR_14_PURPLE_RED,
    COLOR_14_RED,
    COLOR_14_FLESHTONE_CHEEKS,
    COLOR_14_FLESHTONE_HAIR_CHEEKS,
    COLOR_14_FLESHTONE_YELLOW,
    COLOR_14_YELLOW,
    COLOR_14_YELLOW_GREEN,
    COLOR_14_GREEN,
    COLOR_14_GREEN_CYAN,
    COLOR_14_CYAN,
    COLOR_14_CYAN_BLUE,
    COLOR_14_BLUE,
    COLOR_14_MAX,
}CMS_14_COLOR;

typedef enum _CMS_TYPE {
    Type_Saturation = 0,
    Type_Hue,
    Type_Luma,
    Type_Max,
} CMS_TYPE;

typedef struct _CMS_COLOR_PARAM {
    int Saturation;
    int Hue;
    int Luma;
}CMS_COLOR_PARAM;

typedef struct _TABLE_CMS {
    int CmsEnable;
    CMS_COLOR_PARAM CmsColor[COLOR_MAX];
}TABLE_CMS;

typedef struct cms_color_md_s {
    Color_Node_type_t color_type;
    CMS_COLOR cm_9_color_md;
    CMS_14_COLOR cm_14_color_md;
    int color_val;
} cms_color_md_t;

/*-----------------------------------------------------------------------------*/
/* Color Temperature Table*/
/*-----------------------------------------------------------------------------*/
typedef enum rgb_type_e {
    R_TYPE        = 0,
    G_TYPE        = 1,
    B_TYPE        = 2,
    MAX_TYPE,
} rgb_type_t;

typedef enum rgb_index_e {
    PERCENT_0 = 0,
    PERCENT_5,
    PERCENT_10,
    PERCENT_15,
    PERCENT_20,
    PERCENT_25,
    PERCENT_30,
    PERCENT_35,
    PERCENT_40,
    PERCENT_45,
    PERCENT_50,
    PERCENT_55,
    PERCENT_60,
    PERCENT_65,
    PERCENT_70,
    PERCENT_75,
    PERCENT_80,
    PERCENT_85,
    PERCENT_90,
    PERCENT_95,
    PERCENT_100,
    PERCENT_MAX,
} rgb_index_t;

typedef struct _Multipoint_GAMMA_DATA {
    int R_offset[PERCENT_MAX];
    int G_offset[PERCENT_MAX];
    int B_offset[PERCENT_MAX];
} Multipoint_GAMMA_DATA;

typedef struct _GAMMA_OFFSET_DATA {
    int enable;
    int MultipointGammaMode;
    Multipoint_GAMMA_DATA Gamma;
} GAMMA_OFFSET_DATA;

typedef struct _RGB_GAIN_OFFSET {
    int RGAIN;
    int GGAIN;
    int BGAIN;
    int ROFFSET;
    int GOFFSET;
    int BOFFSET;
} RGB_GAIN_OFFSET;

typedef struct _COLORTEMP_OFFSET_DATA {
    int r_gain_value;
    int g_gain_value;
    int b_gain_value;
    int r_offset_value;
    int g_offset_value;
    int b_offset_value;
} COLORTEMP_OFFSET_DATA;

typedef enum _WB_GAMMA_DIAGRAM {
    GRAY_0   = 0,
    GRAY_5   = 13,
    GRAY_10  = 26,
    GRAY_15  = 38,
    GRAY_20  = 51,
    GRAY_25  = 64,
    GRAY_30  = 76,
    GRAY_35  = 90,
    GRAY_40  = 102,
    GRAY_45  = 115,
    GRAY_50  = 128,
    GRAY_55  = 141,
    GRAY_60  = 154,
    GRAY_65  = 166,
    GRAY_70  = 179,
    GRAY_75  = 192,
    GRAY_80  = 204,
    GRAY_85  = 218,
    GRAY_90  = 230,
    GRAY_95  = 243,
    GRAY_100 = 255,
    GRAY_100_257 = 256,
    GRAY_MAX,
} WB_GAMMA_DIAGRAM;

typedef enum _WB_GAMMA_MODE {
    WB_GAMMA_MODE_2POINT,
    WB_GAMMA_MODE_10POINT,
    WB_GAMMA_MODE_11POINT,
    WB_GAMMA_MODE_20POINT,
    WB_GAMMA_MODE_MAX,
} WB_GAMMA_MODE;

typedef struct _COLORTEMP_DATA {
    RGB_GAIN_OFFSET rgbgo;
    GAMMA_OFFSET_DATA GammaOffset;
    COLORTEMP_OFFSET_DATA ColorTempOffset;
    int gamma_curve_index;
} COLORTEMP_DATA;

typedef enum _TEMP_MODE {
    COLOR_TMP_MODE_STANDARD,
    COLOR_TMP_MODE_WARM,
    COLOR_TMP_MODE_COLD,
    COLOR_TMP_MODE_WARMER,//D65
    COLOR_TMP_MODE_COLDER,
    COLOR_TMP_MODE_USER,
    COLOR_TMP_MODE_MAX,
} TEMP_MODE;

typedef enum nonline_params_type_e {
    _BRIGHTNESS,
    _CONTRAST,
    _SATURATION,
    _HUE,
    _SHARPNESS,
    _VOLUME,
    _BACKLIGHT,
    _MAX,
} nonline_params_type_t;

typedef enum pq_source_input_e {
    PQ_SRC_DEFAULT = 0,
    PQ_SRC_TV,
    PQ_SRC_AV1,
    PQ_SRC_AV2,
    PQ_SRC_YPBPR1,
    PQ_SRC_YPBPR2,
    PQ_SRC_HDMI1,
    PQ_SRC_HDMI2,
    PQ_SRC_HDMI3,
    PQ_SRC_HDMI4,
    PQ_SRC_VGA,
    PQ_SRC_MPEG,
    PQ_SRC_DTV,
    PQ_SRC_SVIDEO,
    PQ_SRC_IPTV,
    PQ_SRC_DUMMY,
    PQ_SRC_SPDIF,
    PQ_SRC_ADTV,
    PQ_SRC_MAX,
} pq_source_input_t;

typedef enum pq_sig_fmt_e {
    PQ_SIGFMT_DEFAULT = 0,
    PQ_SIGFMT_SDR,
    PQ_SIGFMT_HDR,
    PQ_SIGFMT_HDRP,
    PQ_SIGFMT_HLG,
    PQ_SIGFMT_DV,
    PQ_SIGFMT_MAX,
} pq_sig_fmt_t;

typedef struct pq_src_param_s {
    pq_source_input_t pq_source_input;
    pq_sig_fmt_t pq_sig_fmt;
} pq_src_param_t;

typedef struct _TABLE_DATA_STRUCT {
    pq_source_input_t      source;
    pq_sig_fmt_t           timing;
    void                   *tableData;
    unsigned int           tableDataLen;
} TABLE_DATA_STRUCT;

#ifdef __cplusplus
}
#endif
#endif
