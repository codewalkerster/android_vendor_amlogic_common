/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "TconRegHandler"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <utils/Log.h>

#include "TconRegHandler.h"

#define LCD_TCON_NODE_0 "/sys/class/lcd/tcon"
#define LCD_TCON_NODE_1 "/sys/class/aml_lcd/lcd0/tcon"

TconRegHandler::TconRegHandler():
    dumpfd(-1),
    debug(false)
{
    memset(tconRegPath, 0, sizeof(tconRegPath));
}

TconRegHandler::~TconRegHandler() {}

bool TconRegHandler::init(const char *tconDir)
{
    bool success = false;

    if (access(LCD_TCON_NODE_0, F_OK) &&
          access(LCD_TCON_NODE_1, F_OK)) {
        logPrint("It's not tconless panel...\n");
        goto __init_exit;
    }

    if (access(tconDir, F_OK)) {
        logPrint("%s is not exist, please check!\n", tconDir);
        goto __init_exit;
    }

    sprintf(tconRegPath, "%s/tcon_reg", tconDir);
    if (access(tconRegPath, F_OK)) {
        memset(tconRegPath, 0, sizeof(tconRegPath));
        logPrint("%s is not exist, please check!\n", tconRegPath);
        goto __init_exit;
    }

    logPrint("Use path: %s\n", tconDir);

    success = true;

__init_exit:
    return success;
}

void TconRegHandler::uninit(void)
{
}

void TconRegHandler::setDumpTo(int fd, bool debug)
{
    if (fd >= 0)
        this->dumpfd = fd;

    this->debug = debug;
}

int TconRegHandler::getReg(unsigned int reg)
{
    std::vector<unsigned int> vec;
    if (getRegs(reg, 1, vec))
        return vec[0];
    return -1;
}

bool TconRegHandler::getRegs(unsigned int startReg, int num, std::vector<unsigned int> &vec)
{
    int fd = -1;
    bool result = false;
    char cmd[256] = { 0 };
    int ret = 0;
    char *result_str = NULL;
    int str_len = 0;
    char *ptr = NULL;
    int numstep = 0;

    if (num <= 256)  str_len = sizeof(char) * 14 * num + 20;
    else    str_len = sizeof(char) * 14 * 256 + 20;
    result_str = (char *)malloc(str_len);
    if (!result_str)
        goto __getTconRegs_exit;
    memset(result_str, 0, str_len);
    do {
        // 256 as a read step
        if (num > 256) numstep = 256;
        else numstep = num;

        memset(result_str, 0, str_len);

        // send command to get registers' value
        ret = sprintf(cmd, "rn 32 %#x %d", startReg, numstep);
        if (debug)
            logPrint("run \"%s > %s\"\n", cmd, tconRegPath);
        fd = open(tconRegPath, O_RDWR);
        if (fd < 0) {
            logPrint("Open %s error: %s\n", tconRegPath, strerror(errno));
            goto __getTconRegs_exit;
        }
        if (write(fd, cmd, ret) < 0) {
            logPrint("Write %s error: %s\n", tconRegPath, strerror(errno));
            goto __getTconRegs_exit;
        }
        lseek(fd, 0, SEEK_SET);
        if (read(fd, result_str, str_len) < 0) {
            logPrint("Read %s error: %s\n", tconRegPath, strerror(errno));
            goto __getTconRegs_exit;
        }

        if (strstr(result_str, "ERROR")) {  // error occur
            logPrint("Get result error, please check!\n");
            goto __getTconRegs_exit;
        }
        if (debug)
            logPrint("--> %s\n", result_str);

        // handle readed back registers' value
        ptr = strtok(result_str, ":");
        while (ptr != NULL) {
            int reg = 0;
            unsigned int value = 0;

            ptr = strtok(NULL, " ");  // split reg and value
            if (sscanf(ptr, "%04x=%08x", &reg, &value) == 2) {
                vec.push_back(value);
            } else {
                if (ptr[0] == '\n')  // the end
                    break;
                logPrint("Error read reg %#x\n", reg);
                goto __getTconRegs_exit;
            }
        }
        startReg += numstep;
        num -= numstep;
    } while (num > 0);

    result = true;

__getTconRegs_exit:
    if (fd >= 0)
        close(fd);
    if (result_str)
        free(result_str);
    return result;
}

bool TconRegHandler::setReg(unsigned int reg, unsigned int val)
{
    return setReg(reg, 0xffffffff, val);
}

bool TconRegHandler::setReg(unsigned int reg, unsigned int mask, unsigned int val)
{
    char cmd[256] = { 0 };
    char result_str[256] = { 0 };
    int ret = -1;
    int fd = -1;
    unsigned int value = 0;
    bool success = false;

    ret = sprintf(cmd, "wm 32 %#x %#x %#x", reg, mask, val);
    if (debug)
        logPrint("run \"%s > %s\"\n", cmd, tconRegPath);
    fd = open(tconRegPath, O_RDWR);
    if (fd < 0) {
        logPrint("Open %s error: %s\n", tconRegPath, strerror(errno));
        goto __setTconReg_exit;
    }
    if (write(fd, cmd, ret) < 0) {
        logPrint("Write %s error: %s\n", tconRegPath, strerror(errno));
        goto __setTconReg_exit;
    }
    lseek(fd, 0, SEEK_SET);
    if (read(fd, result_str, sizeof(result_str)) < 0) {
        logPrint("Write %s error: %s\n", tconRegPath, strerror(errno));
        goto __setTconReg_exit;
    }

    if (strstr(result_str, "ERROR")) {  // error occur
        logPrint("Get result error, please check!\n");
        goto __setTconReg_exit;
    }
    if (sscanf(result_str, "for_tool:%04x=%08x", &reg, &value) == 2) {
        if (debug)
            logPrint("reg [%#x]=%#08x\n", reg, value);
    } else {
        logPrint("Error parse reg\n");
        goto __setTconReg_exit;
    }
    success = true;

__setTconReg_exit:
    if (fd >= 0)
        close(fd);
    return success;
}

void TconRegHandler::logPrint(const char *format, ...)
{
    char buf[4096];

    int ret = -1;
    va_list list;
    va_start(list, format);
    ret = vsnprintf(buf, sizeof(buf), format, list);
    va_end(list);

    if (dumpfd < 0) ALOGD("%s", buf);
    else write(dumpfd, buf, ret);
}

