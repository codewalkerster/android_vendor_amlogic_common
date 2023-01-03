/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef __TCONREGLOADER_H_
#define __TCONREGLOADER_H_
#include <vector>

typedef struct tcon_regpair_s {
    unsigned int reg;
    unsigned int value;
} TconRegPair;

class TconRegLoader {
public:
    TconRegLoader();
    ~TconRegLoader();
    bool load(std::vector<TconRegPair> &vec, const char *path);
    void setDumpTo(int fd, bool debug=false);

private:
    void logPrint(const char *format, ...);
    int dumpfd;
    bool debug;
};

#endif //__TCONREGLOADER_H_

