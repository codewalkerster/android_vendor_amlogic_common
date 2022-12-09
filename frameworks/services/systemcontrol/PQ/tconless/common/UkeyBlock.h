/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */
#ifndef _UKEYBLOCK_H_
#define _UKEYBLOCK_H_

typedef struct __attribute__((packed)) UKeyBlockInfo {
    unsigned int offset;
    unsigned int size;
    unsigned int blk_prm[6];
} UKeyBlockInfo_s;

class UKeyBlock {
public:
    UKeyBlock();
    ~UKeyBlock();
    static UKeyBlock *GetInstance();
    int Init(void);
    UKeyBlockInfo_s *getDemuraUkeyInfo();
    UKeyBlockInfo_s *getPgammaUkeyInfo();
    UKeyBlockInfo_s *getAutoaccUkeyInfo();
    UKeyBlockInfo_s *getAutoflickerUkeyInfo();
    void PrintInfo(int fd);
    void UnInit(void);

private:
    struct UKeySpiBlock {
        unsigned int crc;
        unsigned int data_size;
        unsigned int version;
        unsigned int block_cnt;
        UKeyBlockInfo_s demura_info;
        UKeyBlockInfo_s pgamma_info;
        UKeyBlockInfo_s autoacc_info;
        UKeyBlockInfo_s autoflicker_info;
    };
    void writeFs(const char *path, const char *val);
    int readFs(const char *path, char *buf, int count);
    int readUkey(const char *path, char *value, int count);

    static UKeyBlock *mInstance;
    struct UKeySpiBlock *ukeyblock;
};

#endif //_UKEYBLOCK_H_

