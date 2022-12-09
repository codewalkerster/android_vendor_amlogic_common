#ifndef _ADAP_LD_H_
#define _ADAP_LD_H_

#include <adap_common.h>



/**
*** definition
**/
/******************************************halpq itself define******************************************************/
typedef enum _adap_ld_level_e {
      ADAP_LD_LV_OFF,
      ADAP_LD_LV_LOW,
      ADAP_LD_LV_MID,
      ADAP_LD_LV_HIGH,
      ADAP_LD_LV_MAX,
} adap_ld_level_e;


/******************************************copy from ldm driver******************************************************/
typedef struct aml_ldim_pq_s {
    unsigned int func_en;
    unsigned int remapping_en;

    /* switch fw, use for custom fw. 0=aml_hw_fw, 1=aml_sw_fw */
    unsigned int fw_sel;

    /* fw parameters */
    unsigned int ldc_hist_mode;
    unsigned int ldc_hist_blend_mode;
    unsigned int ldc_hist_blend_alpha;
    unsigned int ldc_hist_adap_blend_max_gain;
    unsigned int ldc_hist_adap_blend_diff_th1;
    unsigned int ldc_hist_adap_blend_diff_th2;
    unsigned int ldc_hist_adap_blend_th0;
    unsigned int ldc_hist_adap_blend_thn;
    unsigned int ldc_hist_adap_blend_gain_0;
    unsigned int ldc_hist_adap_blend_gain_1;
    unsigned int ldc_init_bl_min;
    unsigned int ldc_init_bl_max;

    unsigned int ldc_sf_mode;
    unsigned int ldc_sf_gain_up;
    unsigned int ldc_sf_gain_dn;
    unsigned int ldc_sf_tsf_3x3;
    unsigned int ldc_sf_tsf_5x5;

    unsigned int ldc_bs_bl_mode;
    //unsigned int ldc_glb_apl; //read only
    unsigned int ldc_bs_glb_apl_gain;
    unsigned int ldc_bs_dark_scene_bl_th;
    unsigned int ldc_bs_gain;
    unsigned int ldc_bs_limit_gain;
    unsigned int ldc_bs_loc_apl_gain;
    unsigned int ldc_bs_loc_max_min_gain;
    unsigned int ldc_bs_loc_dark_scene_bl_th;

    unsigned int ldc_tf_en;
    //unsigned int ldc_tf_sc_flag; //read only
    unsigned int ldc_tf_low_alpha;
    unsigned int ldc_tf_high_alpha;
    unsigned int ldc_tf_low_alpha_sc;
    unsigned int ldc_tf_high_alpha_sc;

    unsigned int ldc_dimming_curve_en;
    unsigned int ldc_sc_hist_diff_th;
    unsigned int ldc_sc_apl_diff_th;
    unsigned int bl_remap_curve[17];

    /* comp parameters */
    unsigned int ldc_bl_buf_diff;
    unsigned int ldc_glb_gain;
    unsigned int ldc_dth_en;
    unsigned int ldc_dth_bw;
    unsigned int ldc_gain_lut[16][64];
    unsigned int ldc_min_gain_lut[64];
    //unsigned int ldc_dither_lut[32][16];
} aml_ldim_pq_t;



/**
*** function
**/
ADAP_STATUS_T ADAP_LD_INIT(void);
ADAP_STATUS_T ADAP_LD_UNINIT(void);
ADAP_STATUS_T ADAP_LD_DevIoCtl(int request, ...);
ADAP_STATUS_T ADAP_LD_GetPqInitStatus(void);
ADAP_STATUS_T ADAP_LD_SetPqInit(void);
ADAP_STATUS_T ADAP_LD_GetLevelIdx(int *pLevelIdx);
ADAP_STATUS_T ADAP_LD_SetLevelIdx(int iLevelIdx);
ADAP_STATUS_T ADAP_LD_GetFuncEn(int *pFuncEn);
ADAP_STATUS_T ADAP_LD_SetFuncEn(int iFuncEn);
ADAP_STATUS_T ADAP_LD_GetRemapEn(int *pRemapEn);
ADAP_STATUS_T ADAP_LD_SetRemapEn(int iRemapEn);
ADAP_STATUS_T ADAP_LD_GetBlMatrix(int *pBlMatrix);
ADAP_STATUS_T ADAP_LD_SetBlMatrix(int iMatrix);
ADAP_STATUS_T ADAP_LD_GetDemoMode(int *pDemoMode);
ADAP_STATUS_T ADAP_LD_SetDemoMode(int iDemoMode);

ADAP_STATUS_T ADAP_LD_GetLdmInfo(aml_ldim_pq_t *pLdmInfo);
ADAP_STATUS_T ADAP_LD_SetLdmInfo(adap_ld_level_e level);

#endif
