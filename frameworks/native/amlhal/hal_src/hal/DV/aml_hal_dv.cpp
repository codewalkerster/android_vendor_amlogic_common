#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "DV/aml_hal_dv.h"
#include "DV/adap_dv.h"


#ifdef __cplusplus
extern "C"
{
#endif


HAL_STATUS_T AML_HAL_AMDOLBY_INIT(void)
{
    if (ADAP_AMDOLBY_INIT() == ADAP_OK)
        return API_OK;
    else
        return API_NOT_OK;
}

HAL_STATUS_T AML_HAL_AMDOLBY_Uninit(void)
{
    if (ADAP_AMDOLBY_Uninit() == ADAP_OK)
        return API_OK;
    else
        return API_NOT_OK;
}

HAL_STATUS_T AML_HAL_AMDOLBY_Open(void)
{
    if (ADAP_AMDOLBY_Open() == ADAP_OK)
        return API_OK;
    else
        return API_NOT_OK;
}

HAL_STATUS_T AML_HAL_AMDOLBY_Close(void)
{
    if (ADAP_AMDOLBY_Close() == ADAP_OK)
        return API_OK;
    else
        return API_NOT_OK;
}

#ifdef __cplusplus
}
#endif
