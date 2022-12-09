#ifndef _AML_HAL_LCD_H_
#define _AML_HAL_LCD_H_

#include <halcommon.h>

#ifdef  __cplusplus
extern "C"
{
#endif

#define HAL_CH_LANE_MAX 32


typedef struct HAL_lcd_optical_info_s {
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
}HAL_lcd_optical_info_t;

typedef struct HAL_aml_path_s {
    CHAR string[256];
} HAL_aml_path_t;

typedef struct HAL_am_pq_bin_param_s {
    UINT32 table_index;
    UINT32 table_len;
    union {
        void *table_ptr;
        SINT64 l_table;
    };
} HAL_am_pq_bin_param_t;

typedef struct HAL_lcd_ss_ctl_s {
    UINT32 level;
    UINT32 freq;
    UINT32 mode;
}HAL_lcd_ss_ctl_t;

typedef struct HAL_phy_lane_s {
    UINT32 preem;
    UINT32 amp;
}HAL_phy_lane_t;

typedef struct HAL_phy_config_s {
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
    struct HAL_phy_lane_s lane[HAL_CH_LANE_MAX];
}HAL_phy_config_t;

HAL_STATUS_T AML_HAL_LCD_INIT(void);
HAL_STATUS_T AML_HAL_LCD_Uninit(void);
HAL_STATUS_T AML_HAL_LCD_Open(void);
HAL_STATUS_T AML_HAL_LCD_Close(void);

HAL_STATUS_T AML_HAL_LCD_GetNrHdrInfo(HAL_lcd_optical_info_s *param);
HAL_STATUS_T AML_HAL_LCD_SetNrHdrInfo(HAL_lcd_optical_info_s *param);
HAL_STATUS_T AML_HAL_LCD_GetTconBinMaxCnt(UINT32 *cnt);
HAL_STATUS_T AML_HAL_LCD_SetTconDataIndex(UINT32 index);
HAL_STATUS_T AML_HAL_LCD_GetTconBinPath(HAL_aml_path_s *str);
HAL_STATUS_T AML_HAL_LCD_SetTconBinData(HAL_am_pq_bin_param_s *param);

HAL_STATUS_T AML_HAL_LCD_SetPowerCtrl(UINT32 state);
HAL_STATUS_T AML_HAL_LCD_SetMuteCtrl(UINT32 state);

HAL_STATUS_T AML_HAL_LCD_SetPhyParam(HAL_phy_config_t *param);
HAL_STATUS_T AML_HAL_LCD_GetPhyParam(HAL_phy_config_t *param);

HAL_STATUS_T AML_HAL_LCD_SetSS(HAL_lcd_ss_ctl_t *param);
HAL_STATUS_T AML_HAL_LCD_GetSS(HAL_lcd_ss_ctl_t *param);

#ifdef  __cplusplus
}
#endif
#endif
