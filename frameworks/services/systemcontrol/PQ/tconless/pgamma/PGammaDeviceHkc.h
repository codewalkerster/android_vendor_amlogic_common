/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */
#ifndef _PGAMMADEVICEHKC_H_
#define _PGAMMADEVICEHKC_H_
#include "PGammaDeviceBase.h"

struct HkcPGammaData;

class PGammaDeviceHkc: public PGammaDeviceBase {
public:
    PGammaDeviceHkc();
    virtual ~PGammaDeviceHkc();
    static bool detect(char *flashPath);
    virtual int Init(char *flashPath, unsigned char *pmu, int pmuLen);
    virtual void PrintInfo(int fd); // if fd < 0, print to ALOGX
    virtual PGammaBufferInfo_s * GenerateBuffer();
    virtual bool DestroyBuffer(PGammaBufferInfo_s * buf);
    virtual void UnInit();

private:
    unsigned short GenerateCrc(unsigned char *buf, int buflen);
    bool GetFlashData(int fd, int offset, void *buf, int bufSize);
    struct HkcPGammaData *pgammaInfo;
    PGammaBufferInfo_s pgammaBuf;
    unsigned char *flashData;
};

#endif //_PGAMMADEVICEHKC_H_

