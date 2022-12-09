#ifndef _DV_CMD_ID_H_
#define _DV_CMD_ID_H_

#define AMDOLBY_DEVICE_NAME                          "amdolby_vision"
#define AMDOLBY_IOC_MAGIC                            'D'


enum ioc_amdolby_cmd
{
    IOC_AMDOLBY_CMD_INIT                                     = 0,
    IOC_AMDOLBY_CMD_UNINIT                                   = 1,
    IOC_AMDOLBY_CMD_OPEN                                     = 2,
    IOC_AMDOLBY_CMD_CLOSE                                    = 3,
};

#define AMDOLBY_IOC_INIT                                     _IO(AMDOLBY_IOC_MAGIC, IOC_AMDOLBY_CMD_INIT)
#define AMDOLBY_IOC_UNINIT                                   _IO(AMDOLBY_IOC_MAGIC, IOC_AMDOLBY_CMD_UNINIT)
#define AMDOLBY_IOC_OPEN                                     _IO(AMDOLBY_IOC_MAGIC, IOC_AMDOLBY_CMD_OPEN)
#define AMDOLBY_IOC_CLOSE                                    _IO(AMDOLBY_IOC_MAGIC, IOC_AMDOLBY_CMD_CLOSE)

#endif
