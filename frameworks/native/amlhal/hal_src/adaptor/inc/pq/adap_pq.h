#ifndef _ADAP_PQ_H_
#define _ADAP_PQ_H_

#include <adap_common.h>


/**
*** definition
**/
#define VADJ1_EN    0 //todo
#define HALPQ_DO_MAPPING    0 //todo

#define BRIGHTNESS_MIN    -512
#define BRIGHTNESS_MAX     512

#define CONTRAST_MIN      -1024
#define CONTRAST_MAX       1023

#define SATURATION_MIN    -128
#define SATURATION_MAX     127

#define HUE_MIN           -25
#define HUE_MAX            25

#define DRV_REQUEST_SHARPNESS_MIN      0
#define DRV_REQUEST_SHARPNESS_MAX      255

#define VPP_PRE_GAMMA_TABLE_LEN        65
#define VPP_GAMMA_TABLE_LEN            256
#define VPP_DNLP_SCURV_LEN             65
#define VPP_DNLP_GAIN_VAR_LUT_LEN      49
#define VPP_DNLP_WEXT_GAIN_LEN         48
#define VPP_DNLP_ADP_THRD_LEN          33
#define VPP_DNLP_REG_BLK_BOOST_LEN     13
#define VPP_DNLP_REG_ADP_OFSET_LEN     20
#define VPP_DNLP_REG_MONO_PROT_LEN     6
#define VPP_DNLP_TREND_WHT_EXP_LUT_LEN 9
#define VPP_DNLP_HIST_GAIN_LEN         65
#define VPP_HIST_BIN_COUNT             64
#define VPP_COLOR_HIST_BIN_COUNT       32
#define VPP_MTRX_OFFSET_LEN            3
#define VPP_MTRX_COEF_LEN              9

#define ADAP_REGS_MAX_NUMBER           900


/******************************************halpq itself define******************************************************/
typedef enum _adap_pq_ctmp_mode_e{
    ADAP_PQ_CTEMP_MODE_STANDARD = 0,
    ADAP_PQ_CTEMP_MODE_WARM,
    ADAP_PQ_CTEMP_MODE_COLD,
    ADAP_PQ_CTEMP_MODE_USER,
    ADAP_PQ_CTEMP_MODE_MAX,
} adap_pq_ctmp_mode_e;

typedef enum _adap_pq_nonlinear_Type_e {
    ADAP_PQ_NONLINEAR_BRIGHTNESS = 0,
    ADAP_PQ_NONLINEAR_CONTRAST,
    ADAP_PQ_NONLINEAR_SATURATION,
    ADAP_PQ_NONLINEAR_HUE,
    ADAP_PQ_NONLINEAR_SHARPNESS,
    ADAP_PQ_NONLINEAR_BACKLIGHT,
    ADAP_PQ_NONLINEAR_MAX,
} adap_pq_nonlinear_Type_e;

typedef enum _adap_pq_gamma_curve_e {
    ADAP_PQ_GAMMA_CURVE_1 = 0,
    ADAP_PQ_GAMMA_CURVE_2,
    ADAP_PQ_GAMMA_CURVE_3,
    ADAP_PQ_GAMMA_CURVE_4,
    ADAP_PQ_GAMMA_CURVE_5,
    ADAP_PQ_GAMMA_CURVE_6,
    ADAP_PQ_GAMMA_CURVE_7,
    ADAP_PQ_GAMMA_CURVE_8,
    ADAP_PQ_GAMMA_CURVE_9,
    ADAP_PQ_GAMMA_CURVE_10,
    ADAP_PQ_GAMMA_CURVE_11,
    ADAP_PQ_GAMMA_CURVE_MAX,
} adap_pq_gamma_curve_e;

typedef enum _adap_pq_dnlp_mode_e {
    ADAP_PQ_DNLP_OFF = 0,
    ADAP_PQ_DNLP_LOW,
    ADAP_PQ_DNLP_MID,
    ADAP_PQ_DNLP_HIGH,
} adap_pq_dnlp_mode_e;

typedef enum _adap_pq_lc_mode_e {
    ADAP_PQ_LC_OFF = 0,
    ADAP_PQ_LC_LOW,
    ADAP_PQ_LC_MID,
    ADAP_PQ_LC_HIGH,
    ADAP_PQ_LC_MAX,
} adap_pq_lc_mode_e;

typedef enum _adap_pq_source_timing_e {
    ADAP_PQ_SRC_INDEX_VGA = 0,

    ADAP_PQ_SRC_INDEX_ATV_NTSC,
    ADAP_PQ_SRC_INDEX_ATV_PAL,
    ADAP_PQ_SRC_INDEX_ATV_PAL_M,
    ADAP_PQ_SRC_INDEX_ATV_SECAN,
    ADAP_PQ_SRC_INDEX_ATV_NTSC443,
    ADAP_PQ_SRC_INDEX_ATV_PAL60,
    ADAP_PQ_SRC_INDEX_ATV_NTSC50,
    ADAP_PQ_SRC_INDEX_ATV_PALN,

    ADAP_PQ_SRC_INDEX_AV_NTSC,
    ADAP_PQ_SRC_INDEX_AV_PAL,
    ADAP_PQ_SRC_INDEX_AV_PAL_M,
    ADAP_PQ_SRC_INDEX_AV_SECAN,
    ADAP_PQ_SRC_INDEX_AV_NTSC443,
    ADAP_PQ_SRC_INDEX_AV_PAL60,
    ADAP_PQ_SRC_INDEX_AV_NTSC50,
    ADAP_PQ_SRC_INDEX_AV_PALN,

    ADAP_PQ_SRC_INDEX_SV_NTSC,
    ADAP_PQ_SRC_INDEX_SV_PAL,
    ADAP_PQ_SRC_INDEX_SV_PAL_M,
    ADAP_PQ_SRC_INDEX_SV_SECAM,

    ADAP_PQ_SRC_INDEX_YCbCr_480I,
    ADAP_PQ_SRC_INDEX_YCbCr_576I,
    ADAP_PQ_SRC_INDEX_YCbCr_480P,
    ADAP_PQ_SRC_INDEX_YCbCr_576P,
    ADAP_PQ_SRC_INDEX_YCbCr_720P,
    ADAP_PQ_SRC_INDEX_YCbCr_1080I,
    ADAP_PQ_SRC_INDEX_YCbCr_1080P,

    ADAP_PQ_SRC_INDEX_HDMI_480I,
    ADAP_PQ_SRC_INDEX_HDMI_576I,
    ADAP_PQ_SRC_INDEX_HDMI_480P,
    ADAP_PQ_SRC_INDEX_HDMI_576P,
    ADAP_PQ_SRC_INDEX_HDMI_720P,
    ADAP_PQ_SRC_INDEX_HDMI_1080I,
    ADAP_PQ_SRC_INDEX_HDMI_1080P,
    ADAP_PQ_SRC_INDEX_HDMI_4K2KI,
    ADAP_PQ_SRC_INDEX_HDMI_4K2KP,
    ADAP_PQ_SRC_INDEX_HDR10_HDMI_480I,
    ADAP_PQ_SRC_INDEX_HDR10_HDMI_576I,
    ADAP_PQ_SRC_INDEX_HDR10_HDMI_480P,
    ADAP_PQ_SRC_INDEX_HDR10_HDMI_576P,
    ADAP_PQ_SRC_INDEX_HDR10_HDMI_720P,
    ADAP_PQ_SRC_INDEX_HDR10_HDMI_1080I,
    ADAP_PQ_SRC_INDEX_HDR10_HDMI_1080P,
    ADAP_PQ_SRC_INDEX_HDR10_HDMI_4K2KI,
    ADAP_PQ_SRC_INDEX_HDR10_HDMI_4K2KP,
    ADAP_PQ_SRC_INDEX_HLG_HDMI_480I,
    ADAP_PQ_SRC_INDEX_HLG_HDMI_576I,
    ADAP_PQ_SRC_INDEX_HLG_HDMI_480P,
    ADAP_PQ_SRC_INDEX_HLG_HDMI_576P,
    ADAP_PQ_SRC_INDEX_HLG_HDMI_720P,
    ADAP_PQ_SRC_INDEX_HLG_HDMI_1080I,
    ADAP_PQ_SRC_INDEX_HLG_HDMI_1080P,
    ADAP_PQ_SRC_INDEX_HLG_HDMI_4K2KI,
    ADAP_PQ_SRC_INDEX_HLG_HDMI_4K2KP,
    ADAP_PQ_SRC_INDEX_DV_HDMI_480I,
    ADAP_PQ_SRC_INDEX_DV_HDMI_576I,
    ADAP_PQ_SRC_INDEX_DV_HDMI_480P,
    ADAP_PQ_SRC_INDEX_DV_HDMI_576P,
    ADAP_PQ_SRC_INDEX_DV_HDMI_720P,
    ADAP_PQ_SRC_INDEX_DV_HDMI_1080I,
    ADAP_PQ_SRC_INDEX_DV_HDMI_1080P,
    ADAP_PQ_SRC_INDEX_DV_HDMI_4K2KI,
    ADAP_PQ_SRC_INDEX_DV_HDMI_4K2KP,
    ADAP_PQ_SRC_INDEX_HDR10P_HDMI_480I,
    ADAP_PQ_SRC_INDEX_HDR10P_HDMI_576I,
    ADAP_PQ_SRC_INDEX_HDR10P_HDMI_480P,
    ADAP_PQ_SRC_INDEX_HDR10P_HDMI_576P,
    ADAP_PQ_SRC_INDEX_HDR10P_HDMI_720P,
    ADAP_PQ_SRC_INDEX_HDR10P_HDMI_1080I,
    ADAP_PQ_SRC_INDEX_HDR10P_HDMI_1080P,
    ADAP_PQ_SRC_INDEX_HDR10P_HDMI_4K2KI,
    ADAP_PQ_SRC_INDEX_HDR10P_HDMI_4K2KP,

    ADAP_PQ_SRC_INDEX_DTV_480I,
    ADAP_PQ_SRC_INDEX_DTV_576I,
    ADAP_PQ_SRC_INDEX_DTV_480P,
    ADAP_PQ_SRC_INDEX_DTV_576P,
    ADAP_PQ_SRC_INDEX_DTV_720P,
    ADAP_PQ_SRC_INDEX_DTV_1080I,
    ADAP_PQ_SRC_INDEX_DTV_1080P,
    ADAP_PQ_SRC_INDEX_DTV_4k2kI,
    ADAP_PQ_SRC_INDEX_DTV_4k2kP,

    ADAP_PQ_SRC_INDEX_HDR10_DTV_480I,
    ADAP_PQ_SRC_INDEX_HDR10_DTV_576I,
    ADAP_PQ_SRC_INDEX_HDR10_DTV_480P,
    ADAP_PQ_SRC_INDEX_HDR10_DTV_576P,
    ADAP_PQ_SRC_INDEX_HDR10_DTV_720P,
    ADAP_PQ_SRC_INDEX_HDR10_DTV_1080I,
    ADAP_PQ_SRC_INDEX_HDR10_DTV_1080P,
    ADAP_PQ_SRC_INDEX_HDR10_DTV_4K2KI,
    ADAP_PQ_SRC_INDEX_HDR10_DTV_4K2KP,

    ADAP_PQ_SRC_INDEX_HLG_DTV_480I,
    ADAP_PQ_SRC_INDEX_HLG_DTV_576I,
    ADAP_PQ_SRC_INDEX_HLG_DTV_480P,
    ADAP_PQ_SRC_INDEX_HLG_DTV_576P,
    ADAP_PQ_SRC_INDEX_HLG_DTV_720P,
    ADAP_PQ_SRC_INDEX_HLG_DTV_1080I,
    ADAP_PQ_SRC_INDEX_HLG_DTV_1080P,
    ADAP_PQ_SRC_INDEX_HLG_DTV_4K2KI,
    ADAP_PQ_SRC_INDEX_HLG_DTV_4K2KP,

    ADAP_PQ_SRC_INDEX_HDR10P_DTV_480I,
    ADAP_PQ_SRC_INDEX_HDR10P_DTV_576I,
    ADAP_PQ_SRC_INDEX_HDR10P_DTV_480P,
    ADAP_PQ_SRC_INDEX_HDR10P_DTV_576P,
    ADAP_PQ_SRC_INDEX_HDR10P_DTV_720P,
    ADAP_PQ_SRC_INDEX_HDR10P_DTV_1080I,
    ADAP_PQ_SRC_INDEX_HDR10P_DTV_1080P,
    ADAP_PQ_SRC_INDEX_HDR10P_DTV_4K2KI,
    ADAP_PQ_SRC_INDEX_HDR10P_DTV_4K2KP,

    ADAP_PQ_SRC_INDEX_DV_DTV_480I,
    ADAP_PQ_SRC_INDEX_DV_DTV_576I,
    ADAP_PQ_SRC_INDEX_DV_DTV_480P,
    ADAP_PQ_SRC_INDEX_DV_DTV_576P,
    ADAP_PQ_SRC_INDEX_DV_DTV_720P,
    ADAP_PQ_SRC_INDEX_DV_DTV_1080I,
    ADAP_PQ_SRC_INDEX_DV_DTV_1080P,
    ADAP_PQ_SRC_INDEX_DV_DTV_4K2KI,
    ADAP_PQ_SRC_INDEX_DV_DTV_4K2KP,

    ADAP_PQ_SRC_INDEX_MPEG_480I,
    ADAP_PQ_SRC_INDEX_MPEG_576I,
    ADAP_PQ_SRC_INDEX_MPEG_480P,
    ADAP_PQ_SRC_INDEX_MPEG_576P,
    ADAP_PQ_SRC_INDEX_MPEG_720P,
    ADAP_PQ_SRC_INDEX_MPEG_1080I,
    ADAP_PQ_SRC_INDEX_MPEG_1080P,
    ADAP_PQ_SRC_INDEX_MPEG_4K2KI,
    ADAP_PQ_SRC_INDEX_MPEG_4K2KP,

    ADAP_PQ_SRC_INDEX_HDR10_MPEG_480I,
    ADAP_PQ_SRC_INDEX_HDR10_MPEG_576I,
    ADAP_PQ_SRC_INDEX_HDR10_MPEG_480P,
    ADAP_PQ_SRC_INDEX_HDR10_MPEG_576P,
    ADAP_PQ_SRC_INDEX_HDR10_MPEG_720P,
    ADAP_PQ_SRC_INDEX_HDR10_MPEG_1080I,
    ADAP_PQ_SRC_INDEX_HDR10_MPEG_1080P,
    ADAP_PQ_SRC_INDEX_HDR10_MPEG_4K2KI,
    ADAP_PQ_SRC_INDEX_HDR10_MPEG_4K2KP,

    ADAP_PQ_SRC_INDEX_HLG_MPEG_480I,
    ADAP_PQ_SRC_INDEX_HLG_MPEG_576I,
    ADAP_PQ_SRC_INDEX_HLG_MPEG_480P,
    ADAP_PQ_SRC_INDEX_HLG_MPEG_576P,
    ADAP_PQ_SRC_INDEX_HLG_MPEG_720P,
    ADAP_PQ_SRC_INDEX_HLG_MPEG_1080I,
    ADAP_PQ_SRC_INDEX_HLG_MPEG_1080P,
    ADAP_PQ_SRC_INDEX_HLG_MPEG_4K2KI,
    ADAP_PQ_SRC_INDEX_HLG_MPEG_4K2KP,

    ADAP_PQ_SRC_INDEX_HDR10P_MPEG_480I,
    ADAP_PQ_SRC_INDEX_HDR10P_MPEG_576I,
    ADAP_PQ_SRC_INDEX_HDR10P_MPEG_480P,
    ADAP_PQ_SRC_INDEX_HDR10P_MPEG_576P,
    ADAP_PQ_SRC_INDEX_HDR10P_MPEG_720P,
    ADAP_PQ_SRC_INDEX_HDR10P_MPEG_1080I,
    ADAP_PQ_SRC_INDEX_HDR10P_MPEG_1080P,
    ADAP_PQ_SRC_INDEX_HDR10P_MPEG_4K2KI,
    ADAP_PQ_SRC_INDEX_HDR10P_MPEG_4K2KP,

    ADAP_PQ_SRC_INDEX_DV_MPEG_480I,
    ADAP_PQ_SRC_INDEX_DV_MPEG_576I,
    ADAP_PQ_SRC_INDEX_DV_MPEG_480P,
    ADAP_PQ_SRC_INDEX_DV_MPEG_576P,
    ADAP_PQ_SRC_INDEX_DV_MPEG_720P,
    ADAP_PQ_SRC_INDEX_DV_MPEG_1080I,
    ADAP_PQ_SRC_INDEX_DV_MPEG_1080P,
    ADAP_PQ_SRC_INDEX_DV_MPEG_4K2KI,
    ADAP_PQ_SRC_INDEX_DV_MPEG_4K2KP,

    ADAP_PQ_SRC_INDEX_MAX,
} adap_pq_source_timing_e;

/******************************************copy from vpp driver******************************************************/
typedef struct _ADAP_MAGIC_COLORTEMP_OFFSET_DATA {
    short R_offset[11];
    short G_offset[11];
    short B_offset[11];
} ADAP_MAGIC_COLORTEMP_OFFSET_DATA;

struct vpp_white_balance_s {
    int R_val;
    int G_val;
    int B_val;
    int R_offset_val;
    int G_offset_val;
    int B_offset_val;
    ADAP_MAGIC_COLORTEMP_OFFSET_DATA magicOffset;
    unsigned char  gamma_curve_index;
};

typedef struct tcon_rgb_ogo_s {
    UINT32 en;
    SINT32 r_pre_offset;  // s11.0, range -1024~+1023, default is 0
    SINT32 g_pre_offset;  // s11.0, range -1024~+1023, default is 0
    SINT32 b_pre_offset;  // s11.0, range -1024~+1023, default is 0
    UINT32 r_gain;        // u1.10, range 0~2047, default is 1024 (1.0x)
    UINT32 g_gain;        // u1.10, range 0~2047, default is 1024 (1.0x)
    UINT32 b_gain;        // u1.10, range 0~2047, default is 1024 (1.0x)
    SINT32 r_post_offset; // s11.0, range -1024~+1023, default is 0
    SINT32 g_post_offset; // s11.0, range -1024~+1023, default is 0
    SINT32 b_post_offset; // s11.0, range -1024~+1023, default is 0
} tcon_rgb_ogo_t;

struct vpp_pre_gamma_table_s {
    UINT32 r_data[VPP_PRE_GAMMA_TABLE_LEN];
    UINT32 g_data[VPP_PRE_GAMMA_TABLE_LEN];
    UINT32 b_data[VPP_PRE_GAMMA_TABLE_LEN];
};

struct vpp_gamma_table_s {
    UINT32 r_data[VPP_GAMMA_TABLE_LEN];
    UINT32 g_data[VPP_GAMMA_TABLE_LEN];
    UINT32 b_data[VPP_GAMMA_TABLE_LEN];
};

typedef struct vpp_gamma_ch_table_s {
    UINT16 data[256];
} vpp_gamma_ch_table_t;

typedef struct vpp_gm_tbl_s {
    struct vpp_gamma_ch_table_s gm_tb[10][3];
} vpp_gm_tbl_t;

enum vpp_module_e {
    EN_MODULE_VADJ1 = 0,
    EN_MODULE_VADJ2,
    EN_MODULE_PREGAMMA,
    EN_MODULE_GAMMA,
    EN_MODULE_WB,
    EN_MODULE_DNLP,
    EN_MODULE_CCORING,
    EN_MODULE_SR0,
    EN_MODULE_SR0_DNLP,
    EN_MODULE_SR1,
    EN_MODULE_SR1_DNLP,
    EN_MODULE_LC,
    EN_MODULE_CM,
    EN_MODULE_BLE,
    EN_MODULE_BLS,
    EN_MODULE_LUT3D,
    EN_MODULE_ALL,
};

struct vpp_module_ctrl_s {
    enum vpp_module_e module_type;
    SINT32 status;
};

struct vpp_pq_ctrl_s {
    UINT8 vadj1_en;    /*control video brightness contrast saturation hue*/
    UINT8 vd1_ctrst_en;
    UINT8 vadj2_en;    /*control video+osd brightness contrast saturation hue*/
    UINT8 post_ctrst_en;
    UINT8 pregamma_en;
    UINT8 gamma_en;
    UINT8 wb_en;
    UINT8 dnlp_en;
    UINT8 lc_en;
    UINT8 black_ext_en;
    UINT8 chroma_cor_en;
    UINT8 sharpness0_en;
    UINT8 sharpness1_en;
    UINT8 cm_en;
    UINT8 reserved;
};

struct vpp_pq_state_s {
    SINT32 pq_en;
    struct vpp_pq_ctrl_s pq_cfg;
};

enum vpp_pc_mode_e {
    EN_PC_MODE_OFF = 0,
    EN_PC_MODE_ON,
};

enum vpp_dnlp_param_e {
    EN_DNLP_SMHIST_CK = 0,
    EN_DNLP_MVREFLSH,
    EN_DNLP_CUVBLD_MIN,
    EN_DNLP_CUVBLD_MAX,
    EN_DNLP_BBD_RATIO_LOW,
    EN_DNLP_BBD_RATIO_HIG,
    EN_DNLP_BLK_CCTR,
    EN_DNLP_BRGT_CTRL,
    EN_DNLP_BRGT_RANGE,
    EN_DNLP_BRGHT_ADD,
    EN_DNLP_BRGHT_MAX,
    EN_DNLP_AUTO_RNG,
    EN_DNLP_LOWRANGE,
    EN_DNLP_HGHRANGE,
    EN_DNLP_SATUR_RAT,
    EN_DNLP_SATUR_MAX,
    EN_DNLP_SBGNBND,
    EN_DNLP_SENDBND,
    EN_DNLP_CLASHBGN,
    EN_DNLP_CLASHEND,
    EN_DNLP_CLAHE_GAIN_NEG,
    EN_DNLP_CLAHE_GAIN_POS,
    EN_DNLP_MTDBLD_RATE,
    EN_DNLP_ADPMTD_LBND,
    EN_DNLP_ADPMTD_HBND,
    EN_DNLP_BLKEXT_OFST,
    EN_DNLP_WHTEXT_OFST,
    EN_DNLP_BLKEXT_RATE,
    EN_DNLP_WHTEXT_RATE,
    EN_DNLP_BWEXT_DIV4X_MIN,
    EN_DNLP_IRGNBGN,
    EN_DNLP_IRGNEND,
    EN_DNLP_FINAL_GAIN,
    EN_DNLP_CLIPRATE_V3,
    EN_DNLP_CLIPRATE_MIN,
    EN_DNLP_ADPCRAT_LBND,
    EN_DNLP_ADPCRAT_HBND,
    EN_DNLP_SCURV_LOW_TH,
    EN_DNLP_SCURV_MID1_TH,
    EN_DNLP_SCURV_MID2_TH,
    EN_DNLP_SCURV_HGH1_TH,
    EN_DNLP_SCURV_HGH2_TH,
    EN_DNLP_MTDRATE_ADP_EN,
    EN_DNLP_CLAHE_METHOD,
    EN_DNLP_BLE_EN,
    EN_DNLP_NORM,
    EN_DNLP_SCN_CHG_TH,
    EN_DNLP_IIR_STEP_MUX,
    EN_DNLP_SINGLE_BIN_BW,
    EN_DNLP_SINGLE_BIN_METHOD,
    EN_DNLP_REG_MAX_SLOP_1ST,
    EN_DNLP_REG_MAX_SLOP_MID,
    EN_DNLP_REG_MAX_SLOP_FIN,
    EN_DNLP_REG_MIN_SLOP_1ST,
    EN_DNLP_REG_MIN_SLOP_MID,
    EN_DNLP_REG_MIN_SLOP_FIN,
    EN_DNLP_REG_TREND_WHT_EXPAND_MODE,
    EN_DNLP_REG_TREND_BLK_EXPAND_MODE,
    EN_DNLP_HIST_CUR_GAIN,
    EN_DNLP_HIST_CUR_GAIN_PRECISE,
    EN_DNLP_REG_MONO_BINRANG_ST,
    EN_DNLP_REG_MONO_BINRANG_ED,
    EN_DNLP_C_HIST_GAIN_BASE,
    EN_DNLP_S_HIST_GAIN_BASE,
    EN_DNLP_MVREFLSH_OFFSET,
    EN_DNLP_LUMA_AVG_TH,
    EN_DNLP_PARAM_MAX = 100,
};

struct vpp_dnlp_curve_param_s {
    UINT32 dnlp_scurv_low[VPP_DNLP_SCURV_LEN];
    UINT32 dnlp_scurv_mid1[VPP_DNLP_SCURV_LEN];
    UINT32 dnlp_scurv_mid2[VPP_DNLP_SCURV_LEN];
    UINT32 dnlp_scurv_hgh1[VPP_DNLP_SCURV_LEN];
    UINT32 dnlp_scurv_hgh2[VPP_DNLP_SCURV_LEN];
    UINT32 gain_var_lut49[VPP_DNLP_GAIN_VAR_LUT_LEN];
    UINT32 wext_gain[VPP_DNLP_WEXT_GAIN_LEN];
    UINT32 adp_thrd[VPP_DNLP_ADP_THRD_LEN];
    UINT32 reg_blk_boost_12[VPP_DNLP_REG_BLK_BOOST_LEN];
    UINT32 reg_adp_ofset_20[VPP_DNLP_REG_ADP_OFSET_LEN];
    UINT32 reg_mono_protect[VPP_DNLP_REG_MONO_PROT_LEN];
    UINT32 reg_trend_wht_expand_lut8[VPP_DNLP_TREND_WHT_EXP_LUT_LEN];
    UINT32 c_hist_gain[VPP_DNLP_HIST_GAIN_LEN];
    UINT32 s_hist_gain[VPP_DNLP_HIST_GAIN_LEN];
    UINT32 param[EN_DNLP_PARAM_MAX];
};

struct vpp_lc_curve_s {
    UINT32 lc_saturation[63];
    UINT32 lc_yminval_lmt[16];
    UINT32 lc_ypkbv_ymaxval_lmt[16];
    UINT32 lc_ymaxval_lmt[16];
    UINT32 lc_ypkbv_lmt[16];
    UINT32 lc_ypkbv_ratio[4];
    UINT32 param[100];
};

// ***************************************************************************
// *** struct definitions *********************************************
// ***************************************************************************
typedef enum vpp_dnlp_rt_e {
    VE_DNLP_RT_0S = 0,
    VE_DNLP_RT_1S = 6,
    VE_DNLP_RT_2S,
    VE_DNLP_RT_4S,
    VE_DNLP_RT_8S,
    VE_DNLP_RT_16S,
    VE_DNLP_RT_32S,
    VE_DNLP_RT_64S,
    VE_DNLP_RT_FREEZE,
} vpp_dnlp_rt_t;

typedef enum vpp_dnlp_rl_e {
    VE_DNLP_RL_01 = 1, // max_contrast = 1.0625x
    VE_DNLP_RL_02,     // max_contrast = 1.1250x
    VE_DNLP_RL_03,     // max_contrast = 1.1875x
    VE_DNLP_RL_04,     // max_contrast = 1.2500x
    VE_DNLP_RL_05,     // max_contrast = 1.3125x
    VE_DNLP_RL_06,     // max_contrast = 1.3750x
    VE_DNLP_RL_07,     // max_contrast = 1.4375x
    VE_DNLP_RL_08,     // max_contrast = 1.5000x
    VE_DNLP_RL_09,     // max_contrast = 1.5625x
    VE_DNLP_RL_10,     // max_contrast = 1.6250x
    VE_DNLP_RL_11,     // max_contrast = 1.6875x
    VE_DNLP_RL_12,     // max_contrast = 1.7500x
    VE_DNLP_RL_13,     // max_contrast = 1.8125x
    VE_DNLP_RL_14,     // max_contrast = 1.8750x
    VE_DNLP_RL_15,     // max_contrast = 1.9375x
    VE_DNLP_RL_16,     // max_contrast = 2.0000x
} vpp_dnlp_rl_t;

typedef enum vpp_dnlp_ext_e {
    VE_DNLP_EXT_00 = 0, // weak
    VE_DNLP_EXT_01,
    VE_DNLP_EXT_02,
    VE_DNLP_EXT_03,
    VE_DNLP_EXT_04,
    VE_DNLP_EXT_05,
    VE_DNLP_EXT_06,
    VE_DNLP_EXT_07,
    VE_DNLP_EXT_08,
    VE_DNLP_EXT_09,
    VE_DNLP_EXT_10,
    VE_DNLP_EXT_11,
    VE_DNLP_EXT_12,
    VE_DNLP_EXT_13,
    VE_DNLP_EXT_14,
    VE_DNLP_EXT_15,
    VE_DNLP_EXT_16,     // strong
} vpp_dnlp_ext_t;

typedef struct vpp_bext_s {
    UINT8 en;
    UINT8 start;
    UINT8 slope1;
    UINT8 midpt;
    UINT8 slope2;
} vpp_bext_t;

typedef struct vpp_dnlp_s {
    UINT32      en;
    enum vpp_dnlp_rt_e rt;
    enum vpp_dnlp_rl_e rl;
    enum vpp_dnlp_ext_e black;
    enum vpp_dnlp_ext_e white;
} vpp_dnlp_t;

typedef enum vpp_dnlp_state_e {
    VE_DNLP_STATE_OFF = 0,
    VE_DNLP_STATE_ON,
} vpp_dnlp_state_t;

enum vpp_csc_type_e {
    EN_CSC_MATRIX_NULL                = 0,
    EN_CSC_MATRIX_RGB_YUV601          = 0x1,
    EN_CSC_MATRIX_RGB_YUV601F         = 0x2,
    EN_CSC_MATRIX_RGB_YUV709          = 0x3,
    EN_CSC_MATRIX_RGB_YUV709F         = 0x4,
    EN_CSC_MATRIX_YUV601_RGB          = 0x10,
    EN_CSC_MATRIX_YUV601_YUV601F      = 0x11,
    EN_CSC_MATRIX_YUV601_YUV709       = 0x12,
    EN_CSC_MATRIX_YUV601_YUV709F      = 0x13,
    EN_CSC_MATRIX_YUV601F_RGB         = 0x14,
    EN_CSC_MATRIX_YUV601F_YUV601      = 0x15,
    EN_CSC_MATRIX_YUV601F_YUV709      = 0x16,
    EN_CSC_MATRIX_YUV601F_YUV709F     = 0x17,
    EN_CSC_MATRIX_YUV709_RGB          = 0x20,
    EN_CSC_MATRIX_YUV709_YUV601       = 0x21,
    EN_CSC_MATRIX_YUV709_YUV601F      = 0x22,
    EN_CSC_MATRIX_YUV709_YUV709F      = 0x23,
    EN_CSC_MATRIX_YUV709F_RGB         = 0x24,
    EN_CSC_MATRIX_YUV709F_YUV601      = 0x25,
    EN_CSC_MATRIX_YUV709F_YUV709      = 0x26,
    EN_CSC_MATRIX_YUV601L_YUV709L     = 0x27,
    EN_CSC_MATRIX_YUV709L_YUV601L     = 0x28,
    EN_CSC_MATRIX_YUV709F_YUV601F     = 0x29,
    EN_CSC_MATRIX_BT2020YUV_BT2020RGB = 0x40,
    EN_CSC_MATRIX_BT2020RGB_709RGB,
    EN_CSC_MATRIX_BT2020RGB_CUSRGB,
    EN_CSC_MATRIX_DEFAULT_CSCTYPE     = 0xffff,
};

enum vpp_hdr_type_e {
    EN_TYPE_NONE = 0,
    EN_TYPE_SDR,
    EN_TYPE_HDR10,
    EN_TYPE_HLG,
    EN_TYPE_HDR10PLUS,
    EN_TYPE_DOBVI,
    EN_TYPE_MVC,
    EN_TYPE_CUVA_HDR,
    EN_TYPE_CUVA_HLG,
};

enum vpp_color_primary_e {
    EN_COLOR_PRI_NULL = 0,
    EN_COLOR_PRI_BT601,
    EN_COLOR_PRI_BT709,
    EN_COLOR_PRI_BT2020,
    EN_COLOR_PRI_MAX,
};

/*master_display_info for display device*/
struct vpp_hdr_metadata_s {
    UINT32 primaries[3][2]; /*normalized 50000 in G,B,R order*/
    UINT32 white_point[2];  /*normalized 50000*/
    UINT32 luminance[2];    /*max/min luminance, normalized 10000*/
};

struct vpp_histgm_ave_s {
    UINT32 sum;
    SINT32 width;
    SINT32 height;
    SINT32 ave;
};

struct vpp_histgm_param_s {
    UINT32 hist_pow;
    UINT32 luma_sum;
    UINT32 pixel_sum;
    UINT32 histgm[VPP_HIST_BIN_COUNT];
    UINT32 hue_histgm[VPP_COLOR_HIST_BIN_COUNT];
    UINT32 sat_histgm[VPP_COLOR_HIST_BIN_COUNT];
};

enum vpp_mtrx_type_e {
    EN_MTRX_VD1 = 0,
    EN_MTRX_POST,
    EN_MTRX_POST2,
    EN_MTRX_MAX,
};

struct vpp_mtrx_param_s {
    UINT32 pre_offset[VPP_MTRX_OFFSET_LEN];
    UINT32 matrix_coef[VPP_MTRX_COEF_LEN];
    UINT32 post_offset[VPP_MTRX_OFFSET_LEN];
    UINT32 right_shift;
};

struct vpp_mtrx_info_s {
    enum vpp_mtrx_type_e mtrx_sel;
    struct vpp_mtrx_param_s mtrx_param;
};

enum vpp_lc_param_e {
    EN_LC_CURVE_NODES_VLPF = 0,
    EN_LC_CURVE_NODES_HLPF,
    EN_LC_LMT_RAT_VALID,
    EN_LC_LMT_RAT_MIN_MAX,
    EN_LC_CONTRAST_GAIN_HIGH,
    EN_LC_CONTRAST_GAIN_LOW,   /*5*/
    EN_LC_CONTRAST_LMT_HIGH_1,
    EN_LC_CONTRAST_LMT_LOW_1,
    EN_LC_CONTRAST_LMT_HIGH_0,
    EN_LC_CONTRAST_LMT_LOW_0,
    EN_LC_CONTRAST_SCALE_HIGH, /*10*/
    EN_LC_CONTRAST_SCALE_LOW,
    EN_LC_CONTRAST_BVN_HIGH,
    EN_LC_CONTRAST_BVN_LOW,
    EN_LC_SLOPE_MAX_FACE,
    EN_LC_NUM_M_CORING,        /*15*/
    EN_LC_YPKBV_SLOPE_MAX,
    EN_LC_YPKBV_SLOPE_MIN,
    EN_LC_PARAM_MAX,
};

struct vpp_lc_param_s {
    UINT32 param[EN_LC_PARAM_MAX];
};

typedef struct am_pic_mode_s {
    SINT32 flag;
    SINT32 brightness;
    SINT32 brightness2;
    SINT32 saturation_hue;
    SINT32 saturation_hue_post;
    SINT32 contrast;
    SINT32 contrast2;
    SINT32 vadj1_en;  /*vadj1 enable: 1 enable  0 disable*/
    SINT32 vadj2_en;
}am_pic_mode_t;

/* Register table structure */
typedef struct am_reg_s {
    unsigned int type; //32-bits; 0: CBUS; 1: APB BUS...
    unsigned int addr; //32-bits; Register address
    unsigned int mask; //32-bits; Valid bits
    unsigned int  val; //32-bits; Register Value
} am_reg_t;

typedef struct vpp_regs_s {
    unsigned int    length; // Length of total am_reg
    struct am_reg_s am_reg[ADAP_REGS_MAX_NUMBER];
} vpp_regs_t;

typedef enum vpp_pq_table_name_e {
    TABLE_NAME_SHARPNESS0 = 0x1,/*in vpp*/
    TABLE_NAME_SHARPNESS1 = 0x2,/*in vpp*/
    TABLE_NAME_DNLP = 0x4,      /*in vpp*/
    TABLE_NAME_CM = 0x8,        /*in vpp*/
    TABLE_NAME_BLK_BLUE_EXT = 0x10,/*in vpp*/
    TABLE_NAME_BRIGHTNESS = 0x20,/*in vpp*/
    TABLE_NAME_CONTRAST = 0x40, /*in vpp*/
    TABLE_NAME_SATURATION_HUE = 0x80,/*in vpp*/
    TABLE_NAME_CVD2 = 0x100,        /*in tvafe*/
    TABLE_NAME_DI = 0x200,      /*in di*/
    TABLE_NAME_NR = 0x400,      /*in di*/
    TABLE_NAME_MCDI = 0x800,    /*in di*/
    TABLE_NAME_DEBLOCK = 0x1000,    /*in di*/
    TABLE_NAME_DEMOSQUITO = 0x2000,/*in di*/
    TABLE_NAME_WB = 0X4000,     /*in vpp*/
    TABLE_NAME_GAMMA = 0X8000,  /*in vpp*/
    TABLE_NAME_XVYCC = 0x10000, /*in vpp*/
    TABLE_NAME_HDR = 0x20000,   /*in vpp*/
    TABLE_NAME_DOLBY_VISION = 0x40000,/*in vpp*/
    TABLE_NAME_OVERSCAN = 0x80000,
    TABLE_NAME_SMOOTHPLUS = 0x100000, /*in di*/
    TABLE_NAME_RESERVED2 = 0x200000,
    TABLE_NAME_RESERVED3 = 0x400000,
    TABLE_NAME_RESERVED4 = 0x800000,
    TABLE_NAME_MAX,
} vpp_pq_table_name_t;

typedef struct vpp_pq_load_s {
    vpp_pq_table_name_e param_id;
    UINT32 length;
    union {
        void *param_ptr;
        SINT64 param_ptr_len;
    };
    union {
        void *reserved;
        SINT64 reserved_len;
    };
} vpp_pq_load_t;

typedef enum vpp_lut_type_e {
    LUT_TYPE_HLG = 1,
    LUT_TYPE_HDR = 2,
    LUT_TYPE_MAX
} vpp_lut_type_t;

/*tone mapping struct*/
typedef struct vpp_hdr_tone_mapping_s {
    vpp_lut_type_t lut_type;
    UINT32 lutlength;
    union {
        void *tm_lut;
        SINT64 tm_lut_len;
    };
} vpp_hdr_tone_mapping_t;

typedef struct cms_data_s {
    int color;
    int value;
} cms_data_t;

typedef struct ve_pq_ctrl_s {
    UINT32 length;
    union {
        void *ptr;
        SINT64 ptr_len;
    };
} ve_pq_ctrl_t;

typedef enum meson_cpu_ver_e {
    MESON_CPU_VERSION_NULL = 0,
    MESON_CPU_VERSION_A,
    MESON_CPU_VERSION_B,
    MESON_CPU_VERSION_C,
    MESON_CPU_VERSION_MAX,
} meson_cpu_ver_t;


/*adjust for user*/
typedef struct hdr_tmo_sw_s {
    SINT32 tmo_en;              // 0 1
    SINT32 reg_highlight;       //u10: control overexposure level
    SINT32 reg_hist_th;         //u7
    SINT32 reg_light_th;
    SINT32 reg_highlight_th1;
    SINT32 reg_highlight_th2;
    SINT32 reg_display_e;       //u10
    SINT32 reg_middle_a;        //u7
    SINT32 reg_middle_a_adj;    //u10
    SINT32 reg_middle_b;        //u7
    SINT32 reg_middle_s;        //u7
    SINT32 reg_max_th1;          //u10
    SINT32 reg_middle_th;          //u10
    SINT32 reg_thold1;          //u10
    SINT32 reg_thold2;          //u10
    SINT32 reg_thold3;          //u10
    SINT32 reg_thold4;          //u10
    SINT32 reg_max_th2;          //u10
    SINT32 reg_pnum_th;          //u16
    SINT32 reg_hl0;
    SINT32 reg_hl1;             //u7
    SINT32 reg_hl2;             //u7
    SINT32 reg_hl3;             //u7
    SINT32 reg_display_adj;     //u7
    SINT32 reg_avg_th;
    SINT32 reg_avg_adj;
    SINT32 reg_low_adj;         //u7
    SINT32 reg_high_en;         //u3
    SINT32 reg_high_adj1;       //u7
    SINT32 reg_high_adj2;       //u7
    SINT32 reg_high_maxdiff;    //u7
    SINT32 reg_high_mindiff;    //u7
    UINT32 alpha;
}hdr_tmo_sw_t;


typedef struct ai_pic_table_s {
    unsigned int height;
    unsigned int width;
    union {
        void *table_ptr;
        long long table_len;
    };
} ai_pic_table_t;

struct db_cabc_aad_param_s {
    unsigned int length;
    union {
        void *cabc_aad_param_ptr;
        long long cabc_aad_param_ptr_len;
    };
};

typedef struct db_cabc_param_s {
    int cabc_param_cabc_en;
    int cabc_param_hist_mode;
    int cabc_param_tf_en;
    int cabc_param_sc_flag;
    int cabc_param_bl_map_mode;
    int cabc_param_bl_map_en;
    int cabc_param_temp_proc;
    int cabc_param_max95_ratio;
    int cabc_param_hist_blend_alpha;
    int cabc_param_init_bl_min;
    int cabc_param_init_bl_max;
    int cabc_param_tf_alpha;
    int cabc_param_sc_hist_diff_thd;
    int cabc_param_sc_apl_diff_thd;
    int cabc_param_patch_bl_th;
    int cabc_param_patch_on_alpha;
    int cabc_param_patch_bl_off_th;
    int cabc_param_patch_off_alpha;
    struct db_cabc_aad_param_s db_o_bl_cv;
    struct db_cabc_aad_param_s db_maxbin_bl_cv;
}db_cabc_param_t;

#define CABC_O_BL_CV_MAX       15
#define CABC_MAXBIN_BL_CV_MAX  15

typedef struct cabc_param_s {
    int cabc_param_cabc_en;
    int cabc_param_hist_mode;
    int cabc_param_tf_en;
    int cabc_param_sc_flag;
    int cabc_param_bl_map_mode;
    int cabc_param_bl_map_en;
    int cabc_param_temp_proc;
    int cabc_param_max95_ratio;
    int cabc_param_hist_blend_alpha;
    int cabc_param_init_bl_min;
    int cabc_param_init_bl_max;
    int cabc_param_tf_alpha;
    int cabc_param_sc_hist_diff_thd;
    int cabc_param_sc_apl_diff_thd;
    int cabc_param_patch_bl_th;
    int cabc_param_patch_on_alpha;
    int cabc_param_patch_bl_off_th;
    int cabc_param_patch_off_alpha;
    int cabc_param_o_bl_cv_len;
    int cabc_param_o_bl_cv[CABC_O_BL_CV_MAX];
    int cabc_param_maxbin_bl_cv_len;
    int cabc_param_maxbin_bl_cv[CABC_MAXBIN_BL_CV_MAX];
}cabc_param_t;

typedef enum aad_curve_e {
    LUT_Y_gain = 400,
    LUT_RG_gain,
    LUT_BG_gain,
    gain_lut,
    xy_lut,
    sensor_input,
} add_curve_t;

typedef struct db_aad_param_s {
    int aad_param_cabc_aad_en;
    int aad_param_aad_en;
    int aad_param_tf_en;
    int aad_param_force_gain_en;
    int aad_param_sensor_mode;
    int aad_param_mode;
    int aad_param_dist_mode;
    int aad_param_tf_alpha;
    int aad_param_sensor_input[3];
    struct db_cabc_aad_param_s db_LUT_Y_gain;
    struct db_cabc_aad_param_s db_LUT_RG_gain;
    struct db_cabc_aad_param_s db_LUT_BG_gain;
    struct db_cabc_aad_param_s db_gain_lut;
    struct db_cabc_aad_param_s db_xy_lut;
}db_aad_param_t;

#define CAAD_SENSOR_INPUT_MAX  3
#define CAAD_LUT_Y_GAIN_MAX    20
#define CAAD_LUT_RG_GAIN_MAX   20
#define CAAD_LUT_BG_GAIN_MAX   20
#define CAAD_GAIN_LUT_MAX      50
#define CAAD_XY_LUT_MAX        40


typedef struct aad_param_s {
    int aad_param_cabc_aad_en;
    int aad_param_aad_en;
    int aad_param_tf_en;
    int aad_param_force_gain_en;
    int aad_param_sensor_mode;
    int aad_param_mode;
    int aad_param_dist_mode;
    int aad_param_tf_alpha;
    int aad_param_sensor_input_len;
    int aad_param_sensor_input[CAAD_SENSOR_INPUT_MAX];
    int aad_param_LUT_Y_gain_len;
    int aad_param_LUT_Y_gain[CAAD_LUT_Y_GAIN_MAX];
    int aad_param_LUT_RG_gain_len;
    int aad_param_LUT_RG_gain[CAAD_LUT_RG_GAIN_MAX];
    int aad_param_LUT_BG_gain_len;
    int aad_param_LUT_BG_gain[CAAD_LUT_BG_GAIN_MAX];
    int aad_param_gain_lut_len;
    int aad_param_gain_lut[CAAD_GAIN_LUT_MAX];
    int aad_param_xy_lut_len;
    int aad_param_xy_lut[CAAD_XY_LUT_MAX];
}aad_param_t;

/**
*** function
**/
ADAP_STATUS_T ADAP_PQ_INIT(void);
ADAP_STATUS_T ADAP_PQ_UNINIT(void);
ADAP_STATUS_T ADAP_PQ_DevIoCtl(int request, ...);
ADAP_STATUS_T ADAP_PQ_SetBrightness(SINT32 value);
ADAP_STATUS_T ADAP_PQ_SetBrightness_OSD(SINT32 value);
ADAP_STATUS_T ADAP_PQ_GetBrightness(SINT32 *pValue);
ADAP_STATUS_T ADAP_PQ_GetBrightness_OSD(SINT32 *pValue);
ADAP_STATUS_T ADAP_PQ_SetContrast(SINT32 value);
ADAP_STATUS_T ADAP_PQ_SetContrast_OSD(SINT32 value);
ADAP_STATUS_T ADAP_PQ_GetContrast(SINT32 *pValue);
ADAP_STATUS_T ADAP_PQ_GetContrast_OSD(SINT32 *pValue);
ADAP_STATUS_T ADAP_PQ_SetSaturation(SINT32 value);
ADAP_STATUS_T ADAP_PQ_GetSaturation(SINT32 *pValue);
ADAP_STATUS_T ADAP_PQ_SetHue(SINT32 value);
ADAP_STATUS_T ADAP_PQ_GetHue(SINT32 *pValue);
ADAP_STATUS_T ADAP_PQ_SetSaturationHue(SINT32 sat, SINT32 hue);
ADAP_STATUS_T ADAP_PQ_SetSaturationHue_OSD(SINT32 sat, SINT32 hue);
ADAP_STATUS_T ADAP_PQ_GetSaturationHue(SINT32 *value);
ADAP_STATUS_T ADAP_PQ_GetSaturationHue_OSD(SINT32 *value);

ADAP_STATUS_T ADAP_PQ_SetSharpness(SINT32 value);
ADAP_STATUS_T ADAP_PQ_GetSharpness(SINT32 *pValue);
ADAP_STATUS_T ADAP_PQ_SetBacklight(SINT32 value);
ADAP_STATUS_T ADAP_PQ_GetBacklight(SINT32 *pValue);
ADAP_STATUS_T ADAP_PQ_SetColorTemp(struct vpp_white_balance_s *ptAdapPqWb);
ADAP_STATUS_T ADAP_PQ_GetColorTemp(struct vpp_white_balance_s *ptAdapPqWb);
ADAP_STATUS_T ADAP_PQ_SetPreGamma(struct vpp_pre_gamma_table_s *pPreGamma);
ADAP_STATUS_T ADAP_PQ_GetPreGamma(SINT32 *pValue);
ADAP_STATUS_T ADAP_PQ_SetGammaCurve(adap_pq_gamma_curve_e eGammaCurve);
ADAP_STATUS_T ADAP_PQ_GetPreGamma(SINT32 *pValue);
ADAP_STATUS_T ADAP_PQ_SetGammaChannel_R(vpp_gamma_ch_table_s *pData);
ADAP_STATUS_T ADAP_PQ_SetGammaChannel_G(vpp_gamma_ch_table_s *pData);
ADAP_STATUS_T ADAP_PQ_SetGammaChannel_B(vpp_gamma_ch_table_s *pData);

ADAP_STATUS_T ADAP_PQ_SetModuleCtrl(struct vpp_module_ctrl_s *pModuleCtrl);
ADAP_STATUS_T ADAP_PQ_GetModuleCtrl(SINT32 *pValue);
ADAP_STATUS_T ADAP_PQ_SetMatrixParam(struct vpp_mtrx_info_s *pMatrixInfo);
ADAP_STATUS_T ADAP_PQ_SetPqState(struct vpp_pq_state_s *pPqState);
ADAP_STATUS_T ADAP_PQ_GetPqState(struct vpp_pq_state_s *pPqState);
ADAP_STATUS_T ADAP_PQ_SetPcMode(enum vpp_pc_mode_e ePcMode);
ADAP_STATUS_T ADAP_PQ_GetPcMode(enum vpp_pc_mode_e *pPcMode);
ADAP_STATUS_T ADAP_PQ_SetDnlpMode(adap_pq_dnlp_mode_e eDnlpMode);
ADAP_STATUS_T ADAP_PQ_SetLcMode(adap_pq_lc_mode_e eLcMode);
ADAP_STATUS_T ADAP_PQ_SetLcParam(struct vpp_lc_param_s *pLcParam);
ADAP_STATUS_T ADAP_PQ_SetCscType(enum vpp_csc_type_e eCscType);
ADAP_STATUS_T ADAP_PQ_GetCscType(enum vpp_csc_type_e *pCscType);
ADAP_STATUS_T ADAP_PQ_Set3DLutData(SINT32 value);
ADAP_STATUS_T ADAP_PQ_Get3DLutData(SINT32 *pValue);
ADAP_STATUS_T ADAP_PQ_GetHdrType(enum vpp_hdr_type_e *pHdrType);
ADAP_STATUS_T ADAP_PQ_GetColorPrim(enum vpp_color_primary_e *pColorPrim);
ADAP_STATUS_T ADAP_PQ_GetHdrMetadata(struct vpp_hdr_metadata_s *pHdrMetadata);
ADAP_STATUS_T ADAP_PQ_GetHistAvg(struct vpp_histgm_ave_s *pHistAvg);
ADAP_STATUS_T ADAP_PQ_GetHistParam(struct vpp_histgm_param_s *pHistParam);

ADAP_STATUS_T ADAP_PQ_SetInputSrcTiming(adap_pq_source_timing_e eSrcTiming);
ADAP_STATUS_T video_set_saturation_hue(signed char saturation, signed char hue, signed long *mab);

#endif
