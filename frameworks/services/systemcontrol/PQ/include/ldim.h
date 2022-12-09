/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef __LDIM_H__
#define __LDIM_H__

struct vpu_ldim_param_s {
	/* beam model */
	int rgb_base;
	int boost_gain;
	int lpf_res;
	int fw_ld_th_sf; /* spatial filter threshold */
	/* beam curve */
	int ld_vgain;
	int ld_hgain;
	int ld_litgain;
	int ld_lut_vdg_lext;
	int ld_lut_hdg_lext;
	int ld_lut_vhk_lext;
	int ld_lut_hdg[32];
	int ld_lut_vdg[32];
	int ld_lut_vhk[32];
	/* beam shape minor adjustment */
	int ld_lut_vhk_pos[32];
	int ld_lut_vhk_neg[32];
	int ld_lut_hhk[32];
	int ld_lut_vho_pos[32];
	int ld_lut_vho_neg[32];
	/* remapping */
	int lit_idx_th;
	int comp_gain;
};

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
    unsigned int ldc_dither_lut[32][16];
}aml_ldim_pq_t;

typedef enum LD_lut_e {
    LD_bl_remap_curve = 1000,
    LD_remap_LUT_0    = 1100,
    LD_remap_LUT_1,
    LD_remap_LUT_2,
    LD_remap_LUT_3,
    LD_remap_LUT_4,
    LD_remap_LUT_5,
    LD_remap_LUT_6,
    LD_remap_LUT_7,
    LD_remap_LUT_8,
    LD_remap_LUT_9,
    LD_remap_LUT_10,
    LD_remap_LUT_11,
    LD_remap_LUT_12,
    LD_remap_LUT_13,
    LD_remap_LUT_14,
    LD_remap_LUT_15,
    LD_min_gain_lut   = 1200,
    LD_dither_lut     = 1300,
} LD_lut_t;

#endif
