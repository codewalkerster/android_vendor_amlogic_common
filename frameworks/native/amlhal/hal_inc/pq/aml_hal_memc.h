#ifndef _AML_HAL_MEMC_H_
#define _AML_HAL_MEMC_H_

#include <halcommon.h>

#ifdef  __cplusplus
extern "C"
{
#endif


/**
*** definition
**/
typedef enum _aml_hal_fpp_sate_e {
    AML_HAL_FPP_MEMC_OFF = 0,    // MEMC OFF
    AML_HAL_FPP_MEMC_LOW,        // MEMC LOW, default level9
    AML_HAL_FPP_MEMC_MID,        // MEMC MID, default level10
    AML_HAL_FPP_MEMC_HIGH,       // MEMC HIGH, default level10 and fullback
    AML_HAL_FPP_MEMC_CUSTOME,    // Retain customization
    AML_HAL_FPP_MEMC_24PFILM,    // 24P Film mode, 32 Pulldown out 48fps
    AML_HAL_FPP_MEMC_MAX,
} aml_hal_fpp_sate_e;

typedef enum _aml_hal_memc_type_old { //v4l2_ext_memc_type_old
    AML_HAL_MEMC_TYPE_OFF = 0,
    AML_HAL_MEMC_TYPE_LOW,
    AML_HAL_MEMC_TYPE_HIGH,
    AML_HAL_MEMC_TYPE_USER,
    AML_HAL_MEMC_TYPE_55_PULLDOWN,
    AML_HAL_MEMC_TYPE_MEDIUM
} aml_hal_memc_type_old;

typedef enum _aml_hal_memc_type { //v4l2_ext_memc_type
    AML_HAL_MEMC_OFF          = AML_HAL_MEMC_TYPE_OFF,
    AML_HAL_MEMC_CINEMA_CLEAR = AML_HAL_MEMC_TYPE_MEDIUM,
    AML_HAL_MEMC_NATURAL      = AML_HAL_MEMC_TYPE_LOW,
    AML_HAL_MEMC_SMOOTH       = AML_HAL_MEMC_TYPE_HIGH,
    AML_HAL_MEMC_USER         = AML_HAL_MEMC_TYPE_USER,
    AML_HAL_MEMC_PULLDOWN_55  = AML_HAL_MEMC_TYPE_55_PULLDOWN
} aml_hal_memc_type;

typedef struct aml_hal_memc_motion_comp_info_s { //v4l2_ext_memc_motion_comp_info
    unsigned char blur_level;
    unsigned char judder_level;
    aml_hal_memc_type memc_type;
} aml_hal_memc_motion_comp_info_t;


/**
*** function
**/
HAL_STATUS_T AML_HAL_MEMC_INIT(void);
HAL_STATUS_T AML_HAL_MEMC_UNINIT(void);
HAL_STATUS_T AML_HAL_MEMC_SetMemcEnable(SINT32 enable);
HAL_STATUS_T AML_HAL_MEMC_SetMemcDeJudderLevel(SINT32 level);
HAL_STATUS_T AML_HAL_MEMC_SetMemcDeBlurLevel(SINT32 level);

//LGE
HAL_STATUS_T AML_HAL_MEMC_SetLgeMemcInit(SINT32 iLgeMemcInit);
HAL_STATUS_T AML_HAL_MEMC_SetLgeMemcLevel(aml_hal_memc_motion_comp_info_t *pLgeMemcInfo);
HAL_STATUS_T AML_HAL_MEMC_GetLgeMemcLevel(aml_hal_memc_motion_comp_info_t *pLgeMemcInfo);

#ifdef  __cplusplus
}
#endif
#endif
