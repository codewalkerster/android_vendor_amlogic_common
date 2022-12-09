#ifndef _VDIN_CMD_ID_H_
#define _VDIN_CMD_ID_H_

#define VDIN_DEVICE_NAME                          "vdin0"
#define VDIN_IOC_MAGIC                            'T'


enum ioc_vdin_cmd
{
    IOC_VDIN_CMD_INIT                                     = 0,
    IOC_VDIN_CMD_UNINIT                                   = 1,
    IOC_VDIN_CMD_OPEN                                     = 2,
    IOC_VDIN_CMD_CLOSE                                    = 3,
};

#define VDIN_IOC_INIT                                     _IO(VDIN_IOC_MAGIC, IOC_VDIN_CMD_INIT)
#define VDIN_IOC_UNINIT                                   _IO(VDIN_IOC_MAGIC, IOC_VDIN_CMD_UNINIT)
#define VDIN_IOC_OPEN                                     _IO(VDIN_IOC_MAGIC, IOC_VDIN_CMD_OPEN)
#define VDIN_IOC_CLOSE                                    _IO(VDIN_IOC_MAGIC, IOC_VDIN_CMD_CLOSE)

#endif
