/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef __LCDTYPE_H_
#define __LCDTYPE_H_

struct phy_lane_s {
    unsigned int preem;
    unsigned int amp;
};

#define CH_LANE_MAX 32
struct phy_config_s {
    unsigned int flag;
    unsigned int vswing;
    unsigned int vcm;
    unsigned int odt;
    unsigned int ref_bias;
    unsigned int mode;
    unsigned int weakly_pull_down;
    unsigned int lane_num;
    unsigned int ext_pullup;
    unsigned int vswing_level;
    unsigned int preem_level;
    int ioctl_mode; //for ioctl control mode
    struct phy_lane_s lane[CH_LANE_MAX];
};

struct aml_lcd_ss_ctl_s {
    unsigned int level;
    unsigned int freq;
    unsigned int mode;
};

#define LCD_IOC_TYPE              'C'
#define LCD_IOC_POWER_CTRL        0x6
#define LCD_IOC_MUTE_CTRL         0x7
#define LCD_IOC_GET_FRAME_RATE    0x9
#define LCD_IOC_SET_PHY_PARAM     0xa
#define LCD_IOC_GET_PHY_PARAM     0xb
#define LCD_IOC_SET_SS            0xc
#define LCD_IOC_GET_SS            0xd

#define LCD_IOC_CMD_POWER_CTRL \
    _IOW(LCD_IOC_TYPE, LCD_IOC_POWER_CTRL, unsigned int)
#define LCD_IOC_CMD_MUTE_CTRL \
    _IOW(LCD_IOC_TYPE, LCD_IOC_MUTE_CTRL, unsigned int)
#define LCD_IOC_CMD_SET_PHY_PARAM \
    _IOW(LCD_IOC_TYPE, LCD_IOC_SET_PHY_PARAM, struct phy_config_s)
#define LCD_IOC_CMD_GET_PHY_PARAM \
    _IOR(LCD_IOC_TYPE, LCD_IOC_GET_PHY_PARAM, struct phy_config_s)
#define LCD_IOC_CMD_SET_SS \
    _IOW(LCD_IOC_TYPE, LCD_IOC_SET_SS, struct aml_lcd_ss_ctl_s)
#define LCD_IOC_CMD_GET_SS \
    _IOR(LCD_IOC_TYPE, LCD_IOC_GET_SS, struct aml_lcd_ss_ctl_s)

#define LCD_DEF_NODE1 "/dev/lcd"
#define LCD_DEF_NODE2 "/dev/lcd0"

#endif // __LCDTYPE_H_

