#ifndef _LCD_CMD_ID_H_
#define _LCD_CMD_ID_H_

#define LCD_DEVICE_NAME                          "lcd0"
#define LCD_IOC_MAGIC                            'C'


enum ioc_lcd_cmd
{
    IOC_LCD_CMD_NR_GET_HDR_INFO                          = 0x0,
    IOC_LCD_CMD_SET_HDR_INFO                             = 0x1,
    IOC_LCD_CMD_GET_TCON_BIN_MAX_CNT_INFO                = 0x2,
    IOC_LCD_CMD_SET_TCON_DATA_INDEX_INFO                 = 0x3,
    IOC_LCD_CMD_GET_TCON_BIN_PATH_INFO                   = 0x4,
    IOC_LCD_CMD_SET_SET_TCON_BIN_DATA_INFO               = 0x5,
    IOC_LCD_CMD_SET_POWER_CTRL                           = 0x6,
    IOC_LCD_CMD_SET_MUTE_CTRL                            = 0x7,

    IOC_LCD_CMD_SET_FRAME_RATE                           = 0x9,
    IOC_LCD_CMD_SET_PHY_PARAM                            = 0xa,
    IOC_LCD_CMD_GET_PHY_PARAM                            = 0xb,
    IOC_LCD_CMD_SET_SS                                   = 0xc,
    IOC_LCD_CMD_GET_SS                                   = 0xd,
};

#define LCD_IOC_CMD_NR_GET_HDR_INFO                      _IOR(LCD_IOC_MAGIC, IOC_LCD_CMD_NR_GET_HDR_INFO, struct lcd_optical_info_s)
#define LCD_IOC_CMD_NR_SET_HDR_INFO                      _IOW(LCD_IOC_MAGIC, IOC_LCD_CMD_SET_HDR_INFO, struct lcd_optical_info_s)
#define LCD_IOC_CMD_GET_TCON_BIN_MAX_CNT_INFO            _IOR(LCD_IOC_MAGIC, IOC_LCD_CMD_GET_TCON_BIN_MAX_CNT_INFO, unsigned int)
#define LCD_IOC_CMD_SET_TCON_DATA_INDEX_INFO             _IOW(LCD_IOC_MAGIC, IOC_LCD_CMD_SET_TCON_DATA_INDEX_INFO, unsigned int)
#define LCD_IOC_CMD_GET_TCON_BIN_PATH_INFO               _IOR(LCD_IOC_MAGIC, IOC_LCD_CMD_GET_TCON_BIN_PATH_INFO, struct aml_path_s)
#define LCD_IOC_CMD_SET_TCON_BIN_DATA_INFO               _IOW(LCD_IOC_MAGIC, IOC_LCD_CMD_SET_SET_TCON_BIN_DATA_INFO, struct am_pq_bin_param_s)
#define LCD_IOC_CMD_POWER_CTRL                           _IOW(LCD_IOC_MAGIC, IOC_LCD_CMD_SET_POWER_CTRL, unsigned int)
#define LCD_IOC_CMD_MUTE_CTRL                            _IOW(LCD_IOC_MAGIC, IOC_LCD_CMD_SET_MUTE_CTRL, unsigned int)

#define LCD_IOC_CMD_SET_PHY_PARAM                        _IOW(LCD_IOC_MAGIC, IOC_LCD_CMD_SET_PHY_PARAM, struct phy_config_s)
#define LCD_IOC_CMD_GET_PHY_PARAM                        _IOR(LCD_IOC_MAGIC, IOC_LCD_CMD_GET_PHY_PARAM, struct phy_config_s)
#define LCD_IOC_CMD_SET_SS                               _IOW(LCD_IOC_MAGIC, IOC_LCD_CMD_SET_SS, struct aml_lcd_ss_ctl_s)
#define LCD_IOC_CMD_GET_SS                               _IOR(LCD_IOC_MAGIC, IOC_LCD_CMD_GET_SS, struct aml_lcd_ss_ctl_s)

#endif
