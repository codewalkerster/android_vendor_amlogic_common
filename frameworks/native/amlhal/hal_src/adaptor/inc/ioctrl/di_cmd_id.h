#ifndef _DI_CMD_ID_H_
#define _DI_CMD_ID_H_

#define DI_DEVICE_NAME                          "di0"
#define DI_IOC_MAGIC                            'D'


enum ioc_di_cmd
{
    IOC_DI_CMD_INIT                                     = 0,
    IOC_DI_CMD_UNINIT                                   = 1,
    IOC_DI_CMD_OPEN                                     = 2,
    IOC_DI_CMD_CLOSE                                    = 3,

    IOC_DI_CMD_SET_PQ_PARM                              = 0x51,
};


#define DI_IOC_INIT                                     _IO(DI_IOC_MAGIC, IOC_DI_CMD_INIT)
#define DI_IOC_UNINIT                                   _IO(DI_IOC_MAGIC, IOC_DI_CMD_UNINIT)
#define DI_IOC_OPEN                                     _IO(DI_IOC_MAGIC, IOC_DI_CMD_OPEN)
#define DI_IOC_CLOSE                                    _IO(DI_IOC_MAGIC, IOC_DI_CMD_CLOSE)

#define AMDI_IOC_SET_PQ_PARM                            _IOW(DI_IOC_MAGIC, IOC_DI_CMD_SET_PQ_PARM, struct am_pq_param_s)

#endif
