/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef _C_CTCONDEMURA_H
#define _C_CTCONDEMURA_H
#include "DemuraBaseDevice.h"
#include "PlatMisc.h"
#include <UkeyBlock.h>

class CTconDemura {
public:
    CTconDemura();
    ~CTconDemura();
    static CTconDemura *GetInstance();
    int Init(char *demuraPath);
    int GenerateBin(char *binPath);
    void PrintInfo(int fd);  // if fd < 0, print to ALOGX
    void UnInit(void);
private:
    unsigned int generateFormatCrc32(unsigned int crc, const unsigned char *ptr, int buf_len);
    bool generateFomatBin(char *binPath, unsigned char *lutBuf, unsigned int lutSize,
                                 unsigned char *crcBuf, unsigned int crcLen);
    static CTconDemura *mInstance;
    UKeyBlockInfo_s ukeyDemuraInfo;
    UKeyBlock *mUkeyBlockInstance;
    char demuraFlashPath[128];
    DemuraBaseDevice *device;
    PlatMisc *plat;
};

#endif //_C_CTCONDEMURA_H

