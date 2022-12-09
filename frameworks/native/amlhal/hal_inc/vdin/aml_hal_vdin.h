#ifndef _AML_HAL_VDIN_H_
#define _AML_HAL_VDIN_H_

#include <halcommon.h>

#ifdef  __cplusplus
extern "C"
{
#endif


HAL_STATUS_T AML_HAL_VDIN_INIT(void);
HAL_STATUS_T AML_HAL_VDIN_Uninit(void);
HAL_STATUS_T AML_HAL_VDIN_Open(void);
HAL_STATUS_T AML_HAL_VDIN_Close(void);

#ifdef  __cplusplus
}
#endif
#endif
