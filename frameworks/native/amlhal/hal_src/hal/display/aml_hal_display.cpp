#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <display/aml_hal_display.h>

#ifdef __cplusplus
extern "C"
{
#endif

HAL_STATUS_T AML_HAL_DISP_INIT(void)
{
    return API_OK;
}

HAL_STATUS_T AML_HAL_DISP_Uninit(void)
{
    return API_OK;
}

HAL_STATUS_T AML_HAL_DISP_Open(void)
{
    return API_OK;
}

HAL_STATUS_T AML_HAL_DISP_Close(void)
{
    return API_OK;
}

#ifdef __cplusplus
}
#endif
