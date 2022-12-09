#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "panel/aml_hal_lcd.h"
#include "panel/adap_lcd.h"


#ifdef __cplusplus
extern "C"
{
#endif


HAL_STATUS_T AML_HAL_LCD_INIT(void)
{
    if (ADAP_LCD_INIT() != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_LCD_Uninit(void)
{
    if (ADAP_LCD_Uninit() == ADAP_OK)
        return API_OK;
    else
        return API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LCD_Open(void)
{
    if (ADAP_LCD_Open() == ADAP_OK)
        return API_OK;
    else
        return API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LCD_Close(void)
{
    if (ADAP_LCD_Close() == ADAP_OK)
        return API_OK;
    else
        return API_NOT_OK;
}

HAL_STATUS_T AML_HAL_LCD_GetNrHdrInfo(HAL_lcd_optical_info_s *param)
{
    if (param == NULL) {
        return API_INVALID_PARAMS;
    }

    if (ADAP_LCD_GetNrHdrInfo((lcd_optical_info_s*)param) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_LCD_SetNrHdrInfo(HAL_lcd_optical_info_s *param)
{
    if (param == NULL) {
        return API_INVALID_PARAMS;
    }

    if (ADAP_LCD_SetNrHdrInfo((lcd_optical_info_s*)param) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_LCD_GetTconBinMaxCnt(UINT32 *cnt)
{
    if (cnt == NULL) {
        return API_INVALID_PARAMS;
    }

    if (ADAP_LCD_GetTconBinMaxCnt(cnt) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_LCD_SetTconDataIndex(UINT32 index)
{
    if (ADAP_LCD_SetTconDataIndex(index) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_LCD_GetTconBinPath(HAL_aml_path_s *str)
{
    if (str == NULL) {
        return API_INVALID_PARAMS;
    }

    if (ADAP_LCD_GetTconBinPath((aml_path_s*)str) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_LCD_SetTconBinData(HAL_am_pq_bin_param_s *param)
{
    if (param == NULL) {
        return API_INVALID_PARAMS;
    }

    if (ADAP_LCD_SetTconBinData((am_pq_bin_param_s*)param) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_LCD_SetPowerCtrl(UINT32 state)
{
    if (ADAP_LCD_SetPowerCtrl(state) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_LCD_SetMuteCtrl(UINT32 state)
{
    if (ADAP_LCD_SetMuteCtrl(state) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_LCD_SetPhyParam(HAL_phy_config_t *param)
{
    if (param == NULL) {
        return API_INVALID_PARAMS;
    }

    if (ADAP_LCD_SetPhyParam((phy_config_s*)param) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_LCD_GetPhyParam(HAL_phy_config_t *param)
{
    if (param == NULL) {
        return API_INVALID_PARAMS;
    }

    if (ADAP_LCD_GetPhyParam((phy_config_s*)param) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_LCD_SetSS(HAL_lcd_ss_ctl_t *param)
{
    if (param == NULL) {
        return API_INVALID_PARAMS;
    }

    if (ADAP_LCD_SetSS((aml_lcd_ss_ctl_s*)param) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

HAL_STATUS_T AML_HAL_LCD_GetSS(HAL_lcd_ss_ctl_t *param)
{
    if (param == NULL) {
        return API_INVALID_PARAMS;
    }

    if (ADAP_LCD_GetSS((aml_lcd_ss_ctl_s*)param) != ADAP_OK) {
        return API_NOT_OK;
    }

    return API_OK;
}

#ifdef __cplusplus
}
#endif
