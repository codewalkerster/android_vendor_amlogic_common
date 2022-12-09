/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef _UBOOTENV_H_
#define _UBOOTENV_H_
#include "SystemControlClient.h"

class UbootEnv {
public:
    UbootEnv();
    virtual ~UbootEnv();
    virtual bool write(const char *name, char *value);
    virtual bool read(const char *name, char *value);

private:
    android::sp<android::SystemControlClient> sys_client;
};

#endif //_UBOOTENV_H_

