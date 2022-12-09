#ifndef _AML_HAL_DISPLAY_H_
#define _AML_HAL_DISPLAY_H_

#include <halcommon.h>

#ifdef  __cplusplus
extern "C"
{
#endif

HAL_STATUS_T AML_HAL_DISP_INIT(void);
HAL_STATUS_T AML_HAL_DISP_Uninit(void);
HAL_STATUS_T AML_HAL_DISP_Open(void);
HAL_STATUS_T AML_HAL_DISP_Close(void);


#ifdef  __cplusplus
}
#endif
#endif
