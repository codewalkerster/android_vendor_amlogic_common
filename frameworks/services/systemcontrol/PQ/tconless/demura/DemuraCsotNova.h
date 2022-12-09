/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef _C_DEMURACSOTNOVA_H
#define _C_DEMURACSOTNOVA_H
#include "DemuraBaseDevice.h"

struct __attribute__((packed)) CSOTNOVA_DemuraPara {
    unsigned char reserve1;
    unsigned char planeNum;
    unsigned short hLutNum;
    unsigned short vLutNum;
    unsigned char hBlockSize;
    unsigned char vBlockSize;
    unsigned char reserve2[10];
    unsigned short lowerBound;
    unsigned short upperBound;
    unsigned char reserve3[12];
    unsigned short plane0Level;
    unsigned short plane1Level;
    unsigned short plane2Level;
    unsigned short plane3Level;
    unsigned short plane4Level;
    unsigned char reserve4[14];
};

struct __attribute__((packed)) CSOTNOVA_DemuraData {
    unsigned int header;
    unsigned short paraCrc;
    struct CSOTNOVA_DemuraPara demuraPara;
    unsigned short lutCrc;
    unsigned char *demuraLut;
};

class DemuraCsotNova: public DemuraBaseDevice {
public:
    DemuraCsotNova();
    virtual ~DemuraCsotNova();
    static bool detect(char *flashPath);
    virtual int Init(char *flashPath);
    virtual DemuraBufferInfo_t * GenerateBuffer();
    virtual bool DestroyBuffer(DemuraBufferInfo_t * buf);
    virtual void PrintInfo(int fd);
    virtual void UnInit(void);
private:
    unsigned short CalculateCrc16(unsigned char *buf, unsigned int len);
    bool GetFlashData(int fd, int offset, void *buf, int bufSize);
    char *BlkSize2Str(unsigned char blkSize);
    struct CSOTNOVA_DemuraData demuraFlashData;
    unsigned int demuraFlashLutLen;   //length of demuraFlashData.demuraLut
    DemuraBufferInfo_t lutPlaneBuf;
};


#endif //_C_DEMURACSOTNOVA_H

