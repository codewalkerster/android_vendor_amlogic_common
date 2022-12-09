/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "PlatMisc"

#include <stdio.h>
#include <string.h>
#include <utils/Log.h>
#include <errno.h>

#include "tcondef.h"
#include "PlatMisc.h"

#define LCD_DISP_MODE_4K1K120HZ "3840x1080p120hz"

#define LCD_PMU_PATH_NORMAL_ENV         "TCON_EXT_B0_BIN_PATH"        // normal case
#define LCD_PMU_PATH_MULTI_4K2K60Hz     "TCON_EXT_B0_0_BIN_PATH"      // multi case
#define LCD_PMU_PATH_MULTI_4K1K120Hz    "TCON_EXT_B0_1_BIN_PATH"      // multi case

#define LCD_PGAMMA_PATH_NORMAL_ENV      "TCON_EXT_B0_SPI_BIN_PATH"    // normal case
#define LCD_PGAMMA_PATH_MULTI_4K2K60Hz  "TCON_EXT_B0_0_SPI_BIN_PATH"  // multi case
#define LCD_PGAMMA_PATH_MULTI_4K1K120Hz "TCON_EXT_B0_1_SPI_BIN_PATH"  // multi case

PlatMisc *PlatMisc::mInstance = NULL;
PlatMisc *PlatMisc::GetInstance()
{
    if (NULL == mInstance)
        mInstance = new PlatMisc();
    return mInstance;
}

PlatMisc::PlatMisc()
{
    FILE *cusctrl_fp = NULL, *disp_fp = NULL;
    bool result = false;
    bool dlg_plat = false;
    ssize_t cnt = -1;
    size_t len = 0;
    char *line = NULL;

    isTconlessPlat = false;
    isDlgPlat = false;
    env = NULL;
    memset(dispMode, 0, sizeof(dispMode));

    // check is tconless platform
    if (access(LCD_TCON_NODE_K54, F_OK) &&
          access(LCD_TCON_NODE_K49, F_OK)) {
        isTconlessPlat = false;
    } else {
        isTconlessPlat = true;
    }

    // check is dlg platform
    cusctrl_fp = fopen(LCD_TCON_CUSCTRL_NODE_K54, "r");
    if (!cusctrl_fp)
        cusctrl_fp  = fopen(LCD_TCON_CUSCTRL_NODE_K49, "r");

    if (cusctrl_fp) {
        while (-1 != (cnt = getline((char **)&line, &len, cusctrl_fp))) {
            if (strstr(line, "dlg_flag")) {
                int dlg_flag = 0;
                sscanf(line, "dlg_flag: %d", &dlg_flag);
                isDlgPlat = dlg_flag > 0;
                break;
            }
        }
        if (line)
            free(line);
        line = NULL;
    }

    //
    disp_fp = fopen(LCD_DISP_MODE_NODE, "r");
    if (disp_fp) {
        if (getline(&line, &len, cusctrl_fp) < 0) {
            ALOGD("get display mode error:%s\n", strerror(errno));
        } else {
            strncpy(dispMode, line, len);
        }
    }

    if (line)
        free(line);

    env = new UbootEnv();

__platmisc_exit:
    if (disp_fp)
        fclose(disp_fp);

    if (cusctrl_fp)
        fclose(cusctrl_fp);
}

PlatMisc::~PlatMisc()
{
    if (env)
        delete env;
    env = NULL;
}

bool PlatMisc::IsTconlesssPlatform()
{
    return isTconlessPlat;
}

bool PlatMisc::IsDlgPlatform()
{
    return isTconlessPlat && isDlgPlat;
}

bool PlatMisc::getPmuPath(char path[], int len)
{
    int result = false;
    char *key = NULL;

    if (!env)
        return false;
    if (IsDlgPlatform()) {
        if (strstr(dispMode, LCD_DISP_MODE_4K1K120HZ))
            key = LCD_PMU_PATH_MULTI_4K1K120Hz;
        else
            key = LCD_PMU_PATH_MULTI_4K2K60Hz;
    } else {
        key = LCD_PMU_PATH_NORMAL_ENV;
    }

    if (key)
        result = env->read(key, path);

    return result;
}

bool PlatMisc::getPGammaPath(char path[], int len)
{
    int result = false;
    char *key = NULL;

    if (!env)
        return false;
    if (IsDlgPlatform()) {
        if (strstr(dispMode, LCD_DISP_MODE_4K1K120HZ))
            key = LCD_PGAMMA_PATH_MULTI_4K1K120Hz;
        else
            key = LCD_PGAMMA_PATH_MULTI_4K2K60Hz;
    } else {
        key = LCD_PGAMMA_PATH_NORMAL_ENV;
    }

    if (key)
        result = env->read(key, path);

    return result;
}

