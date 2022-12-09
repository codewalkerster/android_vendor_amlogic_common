#ifndef _ADAP_MEMC_H_
#define _ADAP_MEMC_H_

#include <adap_common.h>



/**
*** definition
**/
/******************************************copy from frc driver******************************************************/
enum frc_fpp_state_e {
    FPP_MEMC_OFF = 0,    // MEMC OFF
    FPP_MEMC_LOW,        // MEMC LOW, default level9
    FPP_MEMC_MID,        // MEMC MID, default level10
    FPP_MEMC_HIGH,       // MEMC HIGH, default level10 and fullback
    FPP_MEMC_CUSTOME,    // Retain customization
    FPP_MEMC_24PFILM,    // 24P Film mode, 32 Pulldown out 48fps
    FPP_MEMC_MAX,
};

// MEMC Type Definition
enum v4l2_ext_memc_type {
    V4L2_EXT_MEMC_OFF = 0,
    V4L2_EXT_MEMC_NATURAL,
    V4L2_EXT_MEMC_SMOOTH,
    V4L2_EXT_MEMC_USER,
    V4L2_EXT_MEMC_PULLDOWN_55,
    V4L2_EXT_MEMC_CINEMA_CLEAR,
};

struct v4l2_ext_memc_motion_comp_info {
    unsigned char blur_level;
    unsigned char judder_level;
    enum v4l2_ext_memc_type memc_type;
};



/**
*** function
**/
ADAP_STATUS_T ADAP_MEMC_INIT(void);
ADAP_STATUS_T ADAP_MEMC_UNINIT(void);
ADAP_STATUS_T ADAP_MEMC_DevIoCtl(int request, ...);
ADAP_STATUS_T ADAP_MEMC_SetMemcEnable(SINT32 enable);
ADAP_STATUS_T ADAP_MEMC_SetMemcDeJudderLevel(SINT32 level);
ADAP_STATUS_T ADAP_MEMC_SetMemcDeBlurLevel(SINT32 level);

//LGE
ADAP_STATUS_T ADAP_MEMC_SetLgeMemcInit(SINT32 iLgeMemcInit);
ADAP_STATUS_T ADAP_MEMC_SetLgeMemcLevel(struct v4l2_ext_memc_motion_comp_info *pLgeMemcInfo);
ADAP_STATUS_T ADAP_MEMC_GetLgeMemcLevel(struct v4l2_ext_memc_motion_comp_info *pLgeMemcInfo);

#endif
