#ifndef _AML_HAL_DV_H_
#define _AML_HAL_DV_H_

#include <halcommon.h>

#ifdef  __cplusplus
extern "C"
{
#endif

HAL_STATUS_T AML_HAL_AMDOLBY_INIT(void);
HAL_STATUS_T AML_HAL_AMDOLBY_Uninit(void);
HAL_STATUS_T AML_HAL_AMDOLBY_Open(void);
HAL_STATUS_T AML_HAL_AMDOLBY_Close(void);


#ifdef  __cplusplus
}
#endif
#endif
