#ifndef _PQ_CMD_ID_H_
#define _PQ_CMD_ID_H_

#define PQ_DEVICE_NAME                          "amvecm"
#define PQ_IOC_MAGIC                            'C'

enum ioc_pq_cmd
{
    IOC_PQ_CMD_SET_BRIGHTNESS                     = 0x01,
    IOC_PQ_CMD_SET_CONTRAST                       = 0x02,
    IOC_PQ_CMD_SET_SATURATION                     = 0x03,
    IOC_PQ_CMD_SET_HUE                            = 0x04,
    IOC_PQ_CMD_SET_SHARPNESS                      = 0x05,
    IOC_PQ_CMD_SET_BRIGHTNESS_POST                = 0x06,
    IOC_PQ_CMD_SET_CONTRAST_POST                  = 0x07,
    IOC_PQ_CMD_SET_SATURATION_POST                = 0x08,
    IOC_PQ_CMD_SET_HUE_POST                       = 0x09,
    IOC_PQ_CMD_SET_WB                             = 0x0a,
    IOC_PQ_CMD_SET_PRE_GAMMA                      = 0x0b,
    IOC_PQ_CMD_SET_GAMMA_DATA                     = 0x0c,
    IOC_PQ_CMD_SET_MODULE_STATUS                  = 0x0e,
    IOC_PQ_CMD_SET_MATRIX_PARAM                   = 0x0d,
    IOC_PQ_CMD_SET_PQ_STATE                       = 0x0f,
//    IOC_PQ_CMD_SET_PC_MODE                        = 0x10,
    IOC_PQ_CMD_SET_DNLP_PARAM                     = 0x11,
//    IOC_PQ_CMD_SET_LC_CURVE                       = 0x12,
//    IOC_PQ_CMD_SET_CSC_TYPE                       = 0x13,
    IOC_PQ_CMD_SET_LC_PARAM                       = 0x14,
    IOC_PQ_CMD_SET_3DLUT_DATA                     = 0x15,

    IOC_PQ_CMD_SET_VE_DNLP                        = 0x21,
    IOC_PQ_CMD_GET_HIST_AVG                       = 0x22,
    IOC_PQ_CMD_VE_DNLP_EN                         = 0x23,
    IOC_PQ_CMD_VE_DNLP_DIS                        = 0x24,
    IOC_PQ_CMD_SET_VE_NEW_DNLP                    = 0x25,
    IOC_PQ_CMD_GET_HIST_BIN                       = 0x26,

    IOC_PQ_CMD_SET_LOAD_REG                       = 0x30,

    IOC_PQ_CMD_GAMMA_TABLE_EN                     = 0x40,
    IOC_PQ_CMD_GAMMA_TABLE_DIS                    = 0x41,
    IOC_PQ_CMD_SET_GAMMA_TABLE_R                  = 0x42,
    IOC_PQ_CMD_SET_GAMMA_TABLE_G                  = 0x43,
    IOC_PQ_CMD_SET_GAMMA_TABLE_B                  = 0x44,
    IOC_PQ_CMD_SET_RGB_OGO                        = 0x45,
    IOC_PQ_CMD_GET_RGB_OGO                        = 0x46,

    IOC_PQ_CMD_SET_GAMMA                          = 0x4a,

    IOC_PQ_CMD_SET_OVERSCAN                       = 0x52,
    IOC_PQ_CMD_GET_DNLP_STATE                     = 0x53,
    IOC_PQ_CMD_SET_DNLP_STATE                     = 0x54,
    IOC_PQ_CMD_GET_PC_MODE                        = 0x55,
    IOC_PQ_CMD_SET_PC_MODE                        = 0x56,
    IOC_PQ_CMD_GET_CSC_TYPE                       = 0x57,
    IOC_PQ_CMD_SET_CSC_TYPE                       = 0x58,
    IOC_PQ_CMD_GET_PIC_MODE                       = 0x59,
    IOC_PQ_CMD_SET_PIC_MODE                       = 0x60,
    IOC_PQ_CMD_GET_HDR_TYPE                       = 0x61,
    IOC_PQ_CMD_SET_LC_CURVE                       = 0x62,
    IOC_PQ_CMD_SET_HDR_TM                         = 0x63,
    IOC_PQ_CMD_GET_HDR_TM                         = 0x64,
    IOC_PQ_CMD_SET_CMS_LUMA                       = 0x65,
    IOC_PQ_CMD_SET_CMS_SAT                        = 0x66,
    IOC_PQ_CMD_SET_CMS_HUE                        = 0x67,
    IOC_PQ_CMD_SET_CMS_HUE_HS                     = 0x68,
    IOC_PQ_CMD_SET_PQ_CTRL                        = 0x69,
    IOC_PQ_CMD_GET_PQ_CTRL                        = 0x6a,
    IOC_PQ_CMD_SET_MESON_CPU_VER                  = 0x6b,
    IOC_PQ_CMD_SET_AIPQ_TABLE                     = 0x6c,

    IOC_PQ_CMD_SET_HDR_TMO                        = 0x74,
    IOC_PQ_CMD_GET_HDR_TMO                        = 0x75,
    IOC_PQ_CMD_SET_CABC_PARAM                     = 0x76,
    IOC_PQ_CMD_GET_AAD_PARAM                      = 0x77,

//    IOC_PQ_CMD_GET_PC_MODE                        = 0x80,
//    IOC_PQ_CMD_GET_CSC_TYPE                       = 0x81,
//    IOC_PQ_CMD_GET_HDR_TYPE                       = 0x82,
    IOC_PQ_CMD_GET_COLOR_PRIM                     = 0x83,
    IOC_PQ_CMD_GET_HDR_METADATA                   = 0x84,
//    IOC_PQ_CMD_GET_HIST_AVG                       = 0x85,
//    IOC_PQ_CMD_GET_HIST_BIN                       = 0x86,
    IOC_PQ_CMD_GET_PQ_STATE                       = 0x87,
};


#define VPP_IOC_SET_BRIGHTNESS                    _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_BRIGHTNESS, int)
#define VPP_IOC_SET_CONTRAST                      _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_CONTRAST, int)
#define VPP_IOC_SET_SATURATION                    _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_SATURATION, int)
#define VPP_IOC_SET_HUE                           _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_HUE, int)
#define VPP_IOC_SET_SHARPNESS                     _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_SHARPNESS, int)
#define VPP_IOC_SET_BRIGHTNESS_POST               _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_BRIGHTNESS_POST, int)
#define VPP_IOC_SET_CONTRAST_POST                 _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_CONTRAST_POST, int)
#define VPP_IOC_SET_SATURATION_POST               _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_SATURATION_POST, int)
#define VPP_IOC_SET_HUE_POST                      _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_HUE_POST, int)
#define VPP_IOC_SET_WB                            _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_WB, struct vpp_white_balance_s)
#define VPP_IOC_SET_PRE_GAMMA_DATA                _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_PRE_GAMMA, struct vpp_pre_gamma_table_s)
#define VPP_IOC_SET_GAMMA_DATA                    _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_GAMMA_DATA, struct vpp_gamma_table_s)
#define VPP_IOC_SET_MODULE_STATUS                 _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_MODULE_STATUS, struct vpp_module_ctrl_s)
#define VPP_IOC_SET_MATRIX_PARAM                  _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_MATRIX_PARAM, struct vpp_mtrx_info_s)
#define VPP_IOC_SET_PQ_STATE                      _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_PQ_STATE, struct vpp_pq_state_s)
//#define VPP_IOC_SET_PC_MODE                       _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_PC_MODE, enum vpp_pc_mode_e)
#define VPP_IOC_SET_DNLP_PARAM                    _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_DNLP_PARAM, struct vpp_dnlp_curve_param_s)
//#define VPP_IOC_SET_LC_CURVE                      _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_LC_CURVE, struct vpp_lc_curve_s)
#define VPP_IOC_SET_CSC_TYPE                      _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_CSC_TYPE, enum vpp_csc_type_e)
#define VPP_IOC_SET_LC_PARAM                      _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_LC_PARAM, struct vpp_lc_param_s)
#define VPP_IOC_SET_3DLUT_DATA                    _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_3DLUT_DATA, int)

#define VPP_IOC_VE_DNLP                           _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_VE_DNLP, struct vpp_dnlp_s)
#define VPP_IOC_GET_HIST_AVG                      _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_HIST_AVG, struct vpp_histgm_ave_s)
#define VPP_IOC_VE_DNLP_EN                        _IO(PQ_IOC_MAGIC, IOC_PQ_CMD_VE_DNLP_EN)
#define VPP_IOC_VE_DNLP_DIS                       _IO(PQ_IOC_MAGIC, IOC_PQ_CMD_VE_DNLP_DIS)
#define VPP_IOC_SET_VE_NEW_DNLP                   _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_VE_NEW_DNLP, struct vpp_dnlp_curve_param_s)
#define VPP_IOC_GET_HIST_BIN                      _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_HIST_BIN, struct vpp_histgm_param_s)

    // VPP.CM IOCTL command list
#define VPP_IOC_LOAD_REG                          _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_LOAD_REG, struct vpp_regs_s)

    // VPP.GAMMA IOCTL command list
#define VPP_IOC_GAMMA_TABLE_EN                    _IO(PQ_IOC_MAGIC, IOC_PQ_CMD_GAMMA_TABLE_EN)
#define VPP_IOC_GAMMA_TABLE_DIS                   _IO(PQ_IOC_MAGIC, IOC_PQ_CMD_GAMMA_TABLE_DIS)
#define VPP_IOC_SET_GAMMA_TABLE_R                 _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_GAMMA_TABLE_R, struct vpp_gamma_ch_table_s)
#define VPP_IOC_SET_GAMMA_TABLE_G                 _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_GAMMA_TABLE_G, struct vpp_gamma_ch_table_s)
#define VPP_IOC_SET_GAMMA_TABLE_B                 _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_GAMMA_TABLE_B, struct vpp_gamma_ch_table_s)

#define VPP_IOC_SET_RGB_OGO                       _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_RGB_OGO, struct tcon_rgb_ogo_s)
#define VPP_IOC_GET_RGB_OGO                       _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_RGB_OGO, struct tcon_rgb_ogo_s)

#define VPP_IOC_GAMMA_SET                         _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_GAMMA, struct vpp_gm_tbl_s)

// VPP.display mode command list
#define VPP_IOC_SET_OVERSCAN                      _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_OVERSCAN, struct vpp_pq_load_s)
//DNLP IOCTL command list
#define VPP_IOC_GET_DNLP_STATE                    _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_DNLP_STATE, enum vpp_dnlp_state_e)
#define VPP_IOC_SET_DNLP_STATE                    _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_DNLP_STATE, enum vpp_dnlp_state_e)

//PC mode IOCTL command list
#define VPP_IOC_GET_PC_MODE                       _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_PC_MODE, enum vpp_pc_mode_e)
#define VPP_IOC_SET_PC_MODE                       _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_PC_MODE, enum vpp_pc_mode_e)

//CSC IOCTL command list
#define VPP_IOC_GET_CSC_TYPE                       _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_CSC_TYPE, enum vpp_csc_type_e)
#define VPP_IOC_SET_CSC_TYPE                       _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_CSC_TYPE, enum vpp_csc_type_e)

#define VPP_IOC_GET_PIC_MODE                      _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_PIC_MODE, struct am_pic_mode_s)
#define VPP_IOC_SET_PIC_MODE                      _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_PIC_MODE, struct am_pic_mode_s)

/*HDR TYPE command list*/
#define VPP_IOC_GET_HDR_TYPE                      _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_HDR_TYPE, enum vpp_hdr_type_e)
/*Local contrast command list*/
#define VPP_IOC_SET_LC_CURVE                      _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_LC_CURVE, struct vpp_lc_curve_s)

#define VPP_IOC_SET_HDR_TM                        _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_HDR_TM, struct vpp_hdr_tone_mapping_s)
#define VPP_IOC_GET_HDR_TM                        _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_HDR_TM, struct vpp_hdr_tone_mapping_s)

/*Skin_tone_control command list*/
#define VPP_IOC_SET_CMS_LUMA                      _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_CMS_LUMA, struct cms_data_s)
#define VPP_IOC_SET_CMS_SAT                       _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_CMS_SAT, struct cms_data_s)
#define VPP_IOC_SET_CMS_HUE                       _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_CMS_HUE, struct cms_data_s)
#define VPP_IOC_SET_CMS_HUE_HS                    _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_CMS_HUE_HS, struct cms_data_s)
//module control for amvecm
#define VPP_IOC_SET_PQ_CTRL                       _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_PQ_CTRL, struct ve_pq_ctrl_s)
#define VPP_IOC_GET_PQ_CTRL                       _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_PQ_CTRL, struct ve_pq_ctrl_s)
/*cpu version ioc*/
#define VPP_IOC_SET_MESON_CPU_VER                 _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_MESON_CPU_VER, enum meson_cpu_ver_e)
/*AI PIC param load IOCTL command*/
#define VPP_IOC_SET_AIPQ_TABLE                    _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_AIPQ_TABLE, struct ai_pic_table_s)
/*hdr10_tmo ioc*/
#define VPP_IOC_SET_HDR_TMO                       _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_HDR_TMO, struct hdr_tmo_sw_s)
#define VPP_IOC_GET_HDR_TMO                       _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_HDR_TMO, struct hdr_tmo_sw_s)
/*cabc command list*/
#define VPP_IOC_SET_CABC_PARAM                    _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_SET_CABC_PARAM, struct db_cabc_param_s)
/*aad command list*/
#define VPP_IOC_SET_AAD_PARAM                     _IOW(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_AAD_PARAM, struct db_aad_param_s)

//#define VPP_IOC_GET_PC_MODE                       _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_PC_MODE, enum vpp_pc_mode_e)
//#define VPP_IOC_GET_CSC_TYPE                      _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_CSC_TYPE, enum vpp_csc_type_e)
//#define VPP_IOC_GET_HDR_TYPE                      _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_HDR_TYPE, enum vpp_hdr_type_e)
#define VPP_IOC_GET_COLOR_PRIM                    _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_COLOR_PRIM, enum vpp_color_primary_e)
#define VPP_IOC_GET_HDR_METADATA                  _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_HDR_METADATA, struct vpp_hdr_metadata_s)
//#define VPP_IOC_GET_HIST_AVG                      _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_HIST_AVG, struct vpp_histgm_ave_s)
//#define VPP_IOC_GET_HIST_BIN                      _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_HIST_BIN, struct vpp_histgm_param_s)
#define VPP_IOC_GET_PQ_STATE                      _IOR(PQ_IOC_MAGIC, IOC_PQ_CMD_GET_PQ_STATE, struct vpp_pq_state_s)

#endif
