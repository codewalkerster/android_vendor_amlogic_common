/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */
#ifndef _TCONREGHANDLER_H_
#define _TCONREGHANDLER_H_
#include <vector>

class TconRegHandler {
public:
    TconRegHandler();
    ~TconRegHandler();
    bool init(const char *tconDir);
    void uninit(void);
    void setDumpTo(int fd, bool debug=false);
    int getReg(unsigned int reg);
    bool getRegs(unsigned int startReg, int num, std::vector<unsigned int> &vec);
    bool setReg(unsigned int reg, unsigned int val);
    bool setReg(unsigned int reg, unsigned int mask, unsigned int val);
private:
    void logPrint(const char *format, ...);
    int dumpfd;
    bool debug;
    char tconRegPath[128];
};

#endif //_TCONREGHANDLER_H_

