#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "pq/aml_hal_memc.h"
#include "pq/adap_memc.h"


#ifdef __cplusplus
extern "C"
{
#endif


HAL_STATUS_T AML_HAL_MEMC_INIT(void)
{
    if (ADAP_MEMC_INIT() != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_MEMC_UNINIT(void)
{
    if (ADAP_MEMC_UNINIT() != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_MEMC_SetMemcEnable(SINT32 enable)
{
    if (ADAP_MEMC_SetMemcEnable(enable) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_MEMC_SetMemcDeJudderLevel(SINT32 level)
{
    if (ADAP_MEMC_SetMemcDeJudderLevel(level) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_MEMC_SetMemcDeBlurLevel(SINT32 level)
{
    if (ADAP_MEMC_SetMemcDeBlurLevel(level) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_MEMC_SetLgeMemcInit(SINT32 iLgeMemcInit)
{
    if (ADAP_MEMC_SetLgeMemcInit(iLgeMemcInit) != ADAP_OK) {
        LOGD("%s Fail\n", __FUNCTION__);
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_MEMC_SetLgeMemcLevel(aml_hal_memc_motion_comp_info_t *pLgeMemcInfo)
{
    if (pLgeMemcInfo == NULL) {
        return API_INVALID_PARAMS;
    }

    if (ADAP_MEMC_SetLgeMemcLevel((v4l2_ext_memc_motion_comp_info *)pLgeMemcInfo) != ADAP_OK) {
        LOGD("%s Fail\n", __FUNCTION__);
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_MEMC_GetLgeMemcLevel(aml_hal_memc_motion_comp_info_t *pLgeMemcInfo)
{
    if (pLgeMemcInfo == NULL) {
        LOGD("%s pLgeMemcInfo is INVALID PARAMS\n", __FUNCTION__);
        return API_INVALID_PARAMS;
    }

    if (ADAP_MEMC_GetLgeMemcLevel((v4l2_ext_memc_motion_comp_info *)pLgeMemcInfo) != ADAP_OK) {
        LOGD("%s Fail\n", __FUNCTION__);
        return API_NOT_OK;
    }

    return API_OK;
}

#ifdef __cplusplus
}
#endif
