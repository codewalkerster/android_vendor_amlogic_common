/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */
#ifndef _PGAMMADEVICEBASE_H_
#define _PGAMMADEVICEBASE_H_

typedef struct PGammaBufferInfo {
    unsigned char *buffer;  // pgamma buffer
    unsigned int size;      // buffer size
    unsigned int crc;       // buffer data crc
    unsigned int crclen;    // crc byte length
} PGammaBufferInfo_s;

class PGammaDeviceBase {
public:
    PGammaDeviceBase() {};
    virtual ~PGammaDeviceBase() {};
    static bool detect(char *flashPath);
    virtual int Init(char *flashPath, unsigned char *pmu, int pmuLen);
    virtual void PrintInfo(int fd); // if fd < 0, print to ALOGX
    virtual PGammaBufferInfo_s * GenerateBuffer();
    virtual bool DestroyBuffer(PGammaBufferInfo_s * buf);
    virtual void UnInit();
};

#endif //_PGAMMADEVICEBASE_H_

