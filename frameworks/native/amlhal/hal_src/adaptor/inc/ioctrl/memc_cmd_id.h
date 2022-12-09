#ifndef _MEMC_CMD_ID_H_
#define _MEMC_CMD_ID_H_

#define MEMC_DEVICE_NAME                          "frc"
#define MEMC_IOC_MAGIC                            'F'


enum ioc_memc_cmd
{
    IOC_MEMC_CMD_GET_FRC_EN                               = 0x00,
    IOC_MEMC_CMD_GET_FRC_STS                              = 0x01,
    IOC_MEMC_CMD_SET_FRC_CANDENCE                         = 0x02,
    IOC_MEMC_CMD_GET_VIDEO_LATENCY                        = 0x03,
    IOC_MEMC_CMD_GET_IS_ON                                = 0x04,
    IOC_MEMC_CMD_SET_INPUT_VS_RATE                        = 0x05,
    IOC_MEMC_CMD_SET_MEMC_ON_OFF                          = 0x06,
    IOC_MEMC_CMD_SET_MEMC_LEVEL                           = 0x07,
    IOC_MEMC_CMD_SET_MEMC_DEMO_MODE                       = 0x08,
    IOC_MEMC_CMD_SET_FPP_MEMC_LEVEL                       = 0x09,
    IOC_MEMC_CMD_SET_MEMC_VENDOR                          = 0x0A,
    IOC_MEMC_CMD_SET_MEMC_FB                              = 0x0B,
    IOC_MEMC_CMD_SET_MEMC_FILM                            = 0x0C,
    IOC_MEMC_CMD_SET_LGE_MEMC_LEVEL                       = 0x0D,
    IOC_MEMC_CMD_GET_LGE_MEMC_LEVEL                       = 0x0E,
    IOC_MEMC_CMD_GET_MEMC_VERSION                         = 0x0F,
    IOC_MEMC_CMD_SET_LGE_MEMC_INIT                        = 0x10,
};


#define FRC_IOC_GET_FRC_EN                                _IOR(MEMC_IOC_MAGIC, IOC_MEMC_CMD_GET_FRC_EN, unsigned int)
#define FRC_IOC_GET_FRC_STS                               _IOR(MEMC_IOC_MAGIC, IOC_MEMC_CMD_GET_FRC_STS, unsigned int)
#define FRC_IOC_SET_FRC_CANDENCE                          _IOW(MEMC_IOC_MAGIC, IOC_MEMC_CMD_SET_FRC_CANDENCE, unsigned int)
#define FRC_IOC_GET_VIDEO_LATENCY                         _IOR(MEMC_IOC_MAGIC, IOC_MEMC_CMD_GET_VIDEO_LATENCY, unsigned int)
#define FRC_IOC_GET_IS_ON                                 _IOR(MEMC_IOC_MAGIC, IOC_MEMC_CMD_GET_IS_ON, unsigned int)
#define FRC_IOC_SET_INPUT_VS_RATE                         _IOW(MEMC_IOC_MAGIC, IOC_MEMC_CMD_SET_INPUT_VS_RATE, unsigned int)
#define FRC_IOC_SET_MEMC_ON_OFF                           _IOW(MEMC_IOC_MAGIC, IOC_MEMC_CMD_SET_MEMC_ON_OFF, unsigned int)
#define FRC_IOC_SET_MEMC_LEVEL                            _IOW(MEMC_IOC_MAGIC, IOC_MEMC_CMD_SET_MEMC_LEVEL, unsigned int)
#define FRC_IOC_SET_MEMC_DMEO_MODE                        _IOW(MEMC_IOC_MAGIC, IOC_MEMC_CMD_SET_MEMC_DEMO_MODE, unsigned int)
#define FRC_IOC_SET_FPP_MEMC_LEVEL                        _IOW(MEMC_IOC_MAGIC, IOC_MEMC_CMD_SET_FPP_MEMC_LEVEL, enum frc_fpp_state_e)
#define FRC_IOC_SET_MEMC_VENDOR                           _IOW(MEMC_IOC_MAGIC, IOC_MEMC_CMD_SET_MEMC_VENDOR, unsigned int)
#define FRC_IOC_SET_MEMC_FB                               _IOW(MEMC_IOC_MAGIC, IOC_MEMC_CMD_SET_MEMC_FB, unsigned int)
#define FRC_IOC_SET_MEMC_FILM                             _IOW(MEMC_IOC_MAGIC, IOC_MEMC_CMD_SET_MEMC_FILM, unsigned int)
#define FRC_IOC_SET_LGE_MEMC_LEVEL                        _IOW(MEMC_IOC_MAGIC, IOC_MEMC_CMD_SET_LGE_MEMC_LEVEL, struct v4l2_ext_memc_motion_comp_info)
#define FRC_IOC_GET_LGE_MEMC_LEVEL                        _IOR(MEMC_IOC_MAGIC, IOC_MEMC_CMD_GET_LGE_MEMC_LEVEL, struct v4l2_ext_memc_motion_comp_info)
#define FRC_IOC_GET_MEMC_VERSION                          _IOR(MEMC_IOC_MAGIC, IOC_MEMC_CMD_GET_MEMC_VERSION, unsigned char[32])
#define FRC_IOC_SET_LGE_MEMC_INIT                         _IOW(MEMC_IOC_MAGIC, IOC_MEMC_CMD_SET_LGE_MEMC_INIT, unsigned int)


#endif
