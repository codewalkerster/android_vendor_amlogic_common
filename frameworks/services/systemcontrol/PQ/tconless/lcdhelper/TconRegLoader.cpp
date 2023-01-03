/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "TconLoad"
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <errno.h>
#include <utils/Log.h>
#include <unistd.h>
#include "TconRegLoader.h"

TconRegLoader::TconRegLoader():
    dumpfd(-1),
    debug(false)
{
}

TconRegLoader::~TconRegLoader() { }

bool TconRegLoader::load(std::vector<TconRegPair> &vec, const char *path)
{
    bool success = false;
    TconRegPair pair = {0, 0};
    ssize_t nread = 0;
    size_t len = 0;
    char *line = NULL;
    int nline = 0;

    FILE *fp = fopen(path, "r");
    if (!fp) {
        logPrint("%s open fail: %s\n", path, strerror(errno));
        goto __load_exit;
    }

    do {
        nread = getline(&line, &len, fp);
        if (nread > 0) {
            nline++;
//            if (debug) logPrint("[line %4d] %s", nline, line);

            // parse line
            if (2 != sscanf(line, "0x%x, 0x%x,\n", &pair.reg, &pair.value))
                continue;

            // save pair
            vec.push_back(pair);
        }
    } while (nread != -1);

    if (line) free(line);
    if (fp) fclose(fp);
    success = true;

__load_exit:
    return success;
}

void TconRegLoader::setDumpTo(int fd, bool debug)
{
    if (fd >= 0)
        this->dumpfd = fd;

    this->debug = debug;
}

void TconRegLoader::logPrint(const char *format, ...)
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

