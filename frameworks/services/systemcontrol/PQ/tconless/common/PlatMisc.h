/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef __PLATMISC_H_
#define __PLATMISC_H_
#include "UbootEnv.h"

class PlatMisc {
public:
    PlatMisc();
    virtual ~PlatMisc();
    static PlatMisc *GetInstance();
    virtual bool IsTconlesssPlatform();
    virtual bool IsDlgPlatform();
    virtual bool getPmuPath(char path[], int len);
    virtual bool getPGammaPath(char path[], int len);

private:
    static PlatMisc *mInstance;
    bool isTconlessPlat;
    bool isDlgPlat;
    char dispMode[128];
    UbootEnv *env;
};

#endif //__PLATMISC_H_

