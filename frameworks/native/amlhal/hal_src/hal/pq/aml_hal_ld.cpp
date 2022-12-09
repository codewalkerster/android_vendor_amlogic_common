#include <stdio.h>
#include <pq/aml_hal_ld.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "pq/aml_hal_ld.h"
#include "pq/adap_ld.h"


#ifdef __cplusplus
extern "C"
{
#endif



/**
*** definition
**/



/**
*** function
**/
HAL_STATUS_T AML_HAL_LD_INIT(void)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s\n", __FUNCTION__);

    ret = ADAP_LD_INIT();
    LOGD("%s ldm init %s\n", __FUNCTION__, (ret == ADAP_OK) ? "success" : "faile");

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_GetPqInitStatus(void)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s\n", __FUNCTION__);

    ret = ADAP_LD_GetPqInitStatus();
    LOGD("%s %s\n", __FUNCTION__, (ret == ADAP_OK) ? "success" : "faile");

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_SetPqInit(void)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s\n", __FUNCTION__);

    ret = ADAP_LD_SetPqInit();
    LOGD("%s %s\n", __FUNCTION__, (ret == ADAP_OK) ? "success" : "faile");

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_GetLevelIdx(int *pLevelIdx)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s\n", __FUNCTION__);

    ret = ADAP_LD_GetLevelIdx(pLevelIdx);
    LOGD("%s %s %d\n", __FUNCTION__, (ret == ADAP_OK) ? "success" : "faile", *pLevelIdx);

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_SetLevelIdx(int iLevelIdx)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s %d\n", __FUNCTION__, iLevelIdx);

    ret = ADAP_LD_SetLevelIdx(iLevelIdx);
    LOGD("%s %s\n", __FUNCTION__, (ret == ADAP_OK) ? "success" : "faile");

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_GetFuncEn(int *pFuncEn)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s\n", __FUNCTION__);

    ret = ADAP_LD_GetFuncEn(pFuncEn);
    LOGD("%s %d %d\n", __FUNCTION__, ret, *pFuncEn);

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_SetFuncEn(SINT32 value)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s %d\n", __FUNCTION__, value);

    ret = ADAP_LD_SetFuncEn(value);
    LOGD("%s %d\n", __FUNCTION__, ret);

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_GetRemapEn(int *pRemapEn)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s\n", __FUNCTION__);

    ret = ADAP_LD_GetRemapEn(pRemapEn);
    LOGD("%s %d %d\n", __FUNCTION__, ret, *pRemapEn);

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_SeRemapEn(int iRemapEn)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s %d\n", __FUNCTION__, iRemapEn);

    ret = ADAP_LD_SetRemapEn(iRemapEn);
    LOGD("%s %d\n", __FUNCTION__, ret);

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_GetBlMatrix(int *pBlMatrix)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s\n", __FUNCTION__);

    ret = ADAP_LD_GetBlMatrix(pBlMatrix);
    LOGD("%s %d %d\n", __FUNCTION__, ret, *pBlMatrix);

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_SeBlMatrix(int iMatrix)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s %d\n", __FUNCTION__, iMatrix);

    ret = ADAP_LD_SetBlMatrix(iMatrix);
    LOGD("%s %d\n", __FUNCTION__, ret);

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_GetDemoMode(aml_hal_ldim_demo_info_t *pDemoInfo)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s\n", __FUNCTION__);

    int iDemo = 0;
    ret = ADAP_LD_GetDemoMode(&iDemo);
    LOGD("%s %d %d\n", __FUNCTION__, ret, iDemo);

    pDemoInfo->bOnOff = iDemo;
    pDemoInfo->eType = aml_hal_ldim_demo_type_linedemo; //temporary

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_SetDemoMode(aml_hal_ldim_demo_info_t *pDemoInfo)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s %d %d\n", __FUNCTION__, pDemoInfo->eType, pDemoInfo->bOnOff);

    ret = ADAP_LD_SetDemoMode(pDemoInfo->bOnOff);
    LOGD("%s %d\n", __FUNCTION__, ret);

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_GetLdmInfo(aml_hal_ld_info_t *pLdmInfo)
{
    CHECK_EXPRESSION_RET((pLdmInfo == NULL));

    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s\n", __FUNCTION__);

    aml_ldim_pq_t ldm_info_adap;
    memset(&ldm_info_adap, 0, sizeof(aml_ldim_pq_t));

    ret = ADAP_LD_GetLdmInfo(&ldm_info_adap);
    LOGD("%s %d\n", __FUNCTION__, ret);

    memcpy(pLdmInfo, &ldm_info_adap, sizeof(aml_ldim_pq_t));

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_SetLocalDimming(int iLevel)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    LOGD("%s %d\n", __FUNCTION__, iLevel);

    if (iLevel >= ADAP_LD_LV_MAX) {
        LOGE("%s input parameter out of range\n", __FUNCTION__);
        return API_NOT_OK;
    }

    adap_ld_level_e ld_level_adap = (adap_ld_level_e)iLevel;

    ret = ADAP_LD_SetLdmInfo(ld_level_adap);
    LOGD("%s %d\n", __FUNCTION__, ret);

    return (ret == ADAP_OK) ? API_OK : API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LD_SetInit(aml_hal_led_panel_info_t *pLedPanelInfo)
{
    return API_OK;
}

HAL_STATUS_T AML_HAL_LD_GetAplInfo(aml_hal_led_apl_info_t *pAplInfo)
{
    return API_OK;
}

HAL_STATUS_T AML_HAL_LD_SetDbIdx(int idx)
{
    return API_OK;
}

HAL_STATUS_T AML_HAL_LD_GetDbIdx(int *pIdx)
{
   return API_OK;
}

HAL_STATUS_T AML_HAL_LD_SetControlSpi(aml_hal_led_spi_ctrl_info_t *pLedControlSpi)
{
    return API_OK;
}

HAL_STATUS_T AML_HAL_LD_GetControlSpi(aml_hal_led_spi_ctrl_info_t *pLedControlSpi)
{
    return API_OK;
}

#ifdef __cplusplus
}
#endif
