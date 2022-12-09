/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */
#ifndef _PGAMMADEVICECSOT_H_
#define _PGAMMADEVICECSOT_H_
#include "PGammaDeviceBase.h"

struct CsotPGammaData;

class PGammaDeviceCsot: public PGammaDeviceBase {
public:
    PGammaDeviceCsot();
    virtual ~PGammaDeviceCsot();
    static bool detect(char *flashPath);
    virtual int Init(char *flashPath, unsigned char *pmu, int pmuLen);
    virtual void PrintInfo(int fd); // if fd < 0, print to ALOGX
    virtual PGammaBufferInfo_s * GenerateBuffer();
    virtual bool DestroyBuffer(PGammaBufferInfo_s * buf);
    virtual void UnInit();

private:

    bool DecodePGammaData(unsigned char *flashbuf, int bufSize);
    bool GetFlashData(int fd, int offset, void *buf, int bufSize);
    struct CsotPGammaData *pgammaInfo;
    PGammaBufferInfo_s pgammaBuf;
    unsigned char *flashData;
};

#endif //_PGAMMADEVICECSOT_H_

