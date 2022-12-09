#ifndef _AML_HAL_DI_H_
#define _AML_HAL_DI_H_

#include <halcommon.h>

#ifdef  __cplusplus
extern "C"
{
#endif

HAL_STATUS_T AML_HAL_DI_INIT(void);
HAL_STATUS_T AML_HAL_DI_Uninit(void);
HAL_STATUS_T AML_HAL_DI_Open(void);
HAL_STATUS_T AML_HAL_DI_Close(void);


#ifdef  __cplusplus
}
#endif
#endif
