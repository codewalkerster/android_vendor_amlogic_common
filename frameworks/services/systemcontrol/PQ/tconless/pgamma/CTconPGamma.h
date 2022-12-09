/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */
#ifndef __CTCONPGAMMA_H_
#define __CTCONPGAMMA_H_
#include <UkeyBlock.h>
#include "PlatMisc.h"
#include "PGammaDeviceBase.h"

class CTconPGamma {
public:
    CTconPGamma();
    ~CTconPGamma();
    static CTconPGamma *GetInstance();
    int Init(char *pgammaPath, char *pmuPath = NULL);
    int GenerateBin(char *binPath); // "default" means use default path, others use user path
    void PrintInfo(int fd);  // if fd < 0, print to ALOGX
    void UnInit(void);

private:
    bool GetPmuInfo(char *pmuPath);
    static CTconPGamma *mInstance;
    char pgammaFlashPath[128];
    char pmu_Path[128];
    UKeyBlockInfo_s *ukeyPGammaInfo;
    UKeyBlock *mUkeyInstance;
    PGammaDeviceBase *device;
    PlatMisc *plat;
    struct {
        unsigned char *buf;
        int len;
    } pmuInfo;
};

#endif //__CTCONPGAMMA_H_

