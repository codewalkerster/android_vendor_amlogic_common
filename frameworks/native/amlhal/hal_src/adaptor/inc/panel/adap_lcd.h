#ifndef _ADAP_LCD_H_
#define _ADAP_LCD_H_

#include <adap_common.h>

#define CH_LANE_MAX 32

typedef struct lcd_optical_info_s {
    UINT32 hdr_support;
    UINT32 features;
    UINT32 primaries_r_x;
    UINT32 primaries_r_y;
    UINT32 primaries_g_x;
    UINT32 primaries_g_y;
    UINT32 primaries_b_x;
    UINT32 primaries_b_y;
    UINT32 white_point_x;
    UINT32 white_point_y;
    UINT32 luma_max;
    UINT32 luma_min;
    UINT32 luma_avg;
} lcd_optical_info_t;

typedef struct aml_path_s {
    CHAR string[256];
} aml_path_t;

typedef struct am_pq_bin_param_s {
    UINT32 table_index;
    UINT32 table_len;
    union {
        void *table_ptr;
        SINT64 l_table;
    };
}am_pq_bin_param_t;

typedef struct aml_lcd_ss_ctl_s {
    UINT32 level;
    UINT32 freq;
    UINT32 mode;
}aml_lcd_ss_ctl_t;

typedef struct phy_lane_s {
    UINT32 preem;
    UINT32 amp;
}phy_lane_t;

typedef struct phy_config_s {
    UINT32 flag;
    UINT32 vswing;
    UINT32 vcm;
    UINT32 odt;
    UINT32 ref_bias;
    UINT32 mode;
    UINT32 weakly_pull_down;
    UINT32 lane_num;
    UINT32 ext_pullup;
    UINT32 vswing_level;
    UINT32 preem_level;
    SINT32 ioctl_mode; //for ioctl control mode
    struct phy_lane_s lane[CH_LANE_MAX];
}phy_config_t;

//function
ADAP_STATUS_T ADAP_LCD_INIT(void);
ADAP_STATUS_T ADAP_LCD_Uninit(void);
ADAP_STATUS_T ADAP_LCD_Open(void);
ADAP_STATUS_T ADAP_LCD_Close(void);

ADAP_STATUS_T ADAP_LCD_GetNrHdrInfo(lcd_optical_info_s *param);
ADAP_STATUS_T ADAP_LCD_SetNrHdrInfo(lcd_optical_info_s *param);
ADAP_STATUS_T ADAP_LCD_GetTconBinMaxCnt(UINT32 *cnt);
ADAP_STATUS_T ADAP_LCD_SetTconDataIndex(UINT32 index);
ADAP_STATUS_T ADAP_LCD_GetTconBinPath(aml_path_s *str);
ADAP_STATUS_T ADAP_LCD_SetTconBinData(am_pq_bin_param_s *param);

ADAP_STATUS_T ADAP_LCD_SetPowerCtrl(UINT32 state);
ADAP_STATUS_T ADAP_LCD_SetMuteCtrl(UINT32 state);

ADAP_STATUS_T ADAP_LCD_SetPhyParam(phy_config_s *param);
ADAP_STATUS_T ADAP_LCD_GetPhyParam(phy_config_s *param);

ADAP_STATUS_T ADAP_LCD_SetSS(aml_lcd_ss_ctl_s *param);
ADAP_STATUS_T ADAP_LCD_GetSS(aml_lcd_ss_ctl_s *param);

#endif
