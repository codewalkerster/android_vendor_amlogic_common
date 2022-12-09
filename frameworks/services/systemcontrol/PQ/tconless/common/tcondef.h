/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef _TCONDEF_H_
#define _TCONDEF_H_
#include <stdio.h>
#include <string.h>

#define LCD_TCON_SPI_FLASH_PATH_K54 "/dev/mtd0"
#define LCD_TCON_SPI_FLASH_PATH_K49 "/dev/mtd/mtd0"

#define LCD_BASE_DIR_K54         "/sys/class/aml_lcd/lcd0"
#define LCD_BASE_DIR_K49         "/sys/class/lcd"

#define LCD_TCON_NODE_K54        LCD_BASE_DIR_K54"/tcon"
#define LCD_TCON_NODE_K49        LCD_BASE_DIR_K49"/tcon"

#define LCD_TCON_CUSCTRL_NODE_K54 LCD_BASE_DIR_K54"/cus_ctrl"
#define LCD_TCON_CUSCTRL_NODE_K49 LCD_BASE_DIR_K49"/cus_ctrl"

#define LCD_DISP_MODE_NODE       "/sys/class/display/mode"

#define MYSTRCPY(dst, src, format, args...) \
    do { \
        sprintf(src, format, ##args); \
        strcat(dst, src); \
    } while (0)

#endif //_TCONDEF_H_

