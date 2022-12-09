/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef _DEMURABASEDEVICE_H
#define _DEMURABASEDEVICE_H

typedef struct DemuraBufferInfo {
    unsigned char *buffer;  // lut buffer. packed bit storage, no left space
    unsigned int size;      // lut buffer size
    unsigned int crc;       // buffer data crc
    unsigned int crclen;    // byte length of crc
} DemuraBufferInfo_t;

class DemuraBaseDevice {
public:
    DemuraBaseDevice() {};
    virtual ~DemuraBaseDevice() {};
    static bool detect(char *flashPath);
    virtual int Init(char *flashPath);
    virtual void PrintInfo(int fd); // if fd < 0, print to ALOGX
    virtual DemuraBufferInfo_t * GenerateBuffer();
    virtual bool DestroyBuffer(DemuraBufferInfo_t * buf);
    virtual void UnInit();
};

#endif //_DEMURABASEDEVICE_H

