#ifndef _PQ_TABLE_TYPE_OSD_H_
#define _PQ_TABLE_TYPE_OSD_H_


#ifdef __cplusplus
extern "C" {
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
    PICTURE_MODE_DV_BRIGHT,
    PICTURE_MODE_DV_DARK,
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
    int ColorGamut;
    int ColorTemperature;
    int LocalContrast;
    int BlackStretch;
    int BlueStretch;
    int MpegNr;
    int ChromaCoring;
    int Memc;
    int DvMode;
    int DvDarkDetail;
}PICTURE_MODE_DATA;

/*-----------------------------------------------------------------------------*/
/* Color Temperature Table*/
/*-----------------------------------------------------------------------------*/
typedef struct _MAGIC_COLORTEMP_OFFSET_DATA {
    short R_offset[11];
    short G_offset[11];
    short B_offset[11];
} MAGIC_COLORTEMP_OFFSET_DATA;

typedef struct _COLORTEMP_DATA {
    int R_val;
    int G_val;
    int B_val;
    int R_offset_val;
    int G_offset_val;
    int B_offset_val;
    MAGIC_COLORTEMP_OFFSET_DATA magicOffset;
    unsigned char  gamma_curve_index;
} COLORTEMP_DATA;

typedef enum _TEMP_MODE {
    COLOR_TMP_MODE_STANDARD,
    COLOR_TMP_MODE_WARM,
    COLOR_TMP_MODE_COLD,
    COLOR_TMP_MODE_USER,
    COLOR_TMP_MODE_MAX,
} TEMP_MODE;

typedef struct _TBL_WHITE_BALANCE {
    unsigned int R_gain;  /*u1.10, range 0~2047, default is 1024 (1.0x)*/
    unsigned int G_gain;  /*u1.10, range 0~2047, default is 1024 (1.0x)*/
    unsigned int B_gain;  /*u1.10, range 0~2047, default is 1024 (1.0x)*/
    int R_offset;    /*s11.0, range -1024~+1023, default is 0*/
    int G_offset;    /*s11.0, range -1024~+1023, default is 0*/
    int B_offset;    /*s11.0, range -1024~+1023, default is 0*/
} TBL_WHITE_BALANCE;

typedef enum _DNLP_STATUS {
    DNLP_OFF,
    DNLP_LOW,
    DNLP_MID,
    DNLP_HIGH,
} DNLP_STATUS;

typedef enum noline_params_type_e {
    _BRIGHTNESS,
    _CONTRAST,
    _SATURATION,
    _HUE,
    _SHARPNESS,
    _VOLUME,
    _BACKLIGHT,
    _MAX,
} noline_params_type_t;

/*
typedef struct _CMS_DATA {
    unsigned char blue_purple;
    unsigned char purple;
    unsigned char purple_red;
    unsigned char red;
    unsigned char flashtone_cheeks;
    unsigned char flashtone_hair_cheeks;
    unsigned char flashtone_yellow;
    unsigned char yellow;
    unsigned char yellow_green;
    unsigned char green;
    unsigned char green_cyan;
    unsigned char cyan;
    unsigned char cyan_blue;
    unsigned char blue;
} CMS_DATA;

typedef enum _GAMMA_LEVEL {
    GAMMA_CURVE_0 = 0,
    GAMMA_CURVE_1,
    GAMMA_CURVE_2,
    GAMMA_CURVE_3,
    GAMMA_CURVE_4,
    GAMMA_CURVE_5,
    GAMMA_CURVE_6,
    GAMMA_CURVE_7,
    GAMMA_CURVE_8,
    GAMMA_CURVE_9,
    GAMMA_CURVE_MAX_NUM,
} GAMMA_LEVEL;

typedef enum vpp_noise_reduction_mode_e {
    VPP_NOISE_REDUCTION_MODE_OFF,
    VPP_NOISE_REDUCTION_MODE_LOW,
    VPP_NOISE_REDUCTION_MODE_MID,
    VPP_NOISE_REDUCTION_MODE_HIGH,
    VPP_NOISE_REDUCTION_MODE_AUTO,
    VPP_NOISE_REDUCTION_MODE_MAX,
} vpp_noise_reduction_mode_t;

typedef enum vpp_pq_level_e {
    VPP_PQ_LV_OFF,
    VPP_PQ_LV_LOW,
    VPP_PQ_LV_MID,
    VPP_PQ_LV_HIGH,
    VPP_PQ_LV_MAX,
} vpp_pq_level_t;
*/

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
    PQ_SIGFMT_DOBLY,
    PQ_SIGFMT_MAX,
} pq_sig_fmt_t;

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
