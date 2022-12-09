/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef _C_DEMURACSOTSTANDARD_H
#define _C_DEMURACSOTSTANDARD_H
#include "DemuraBaseDevice.h"

/******************************************************************/
struct __attribute__((packed)) CSOTSTD_FileHeader {
    unsigned int magicNum;        //ASCII: "CSOT"
    char name[14];                //ASCII, mechine name
    unsigned short version;       //high 8bit as bit version, low 8bit as small version
    unsigned short sectionNum;    //section number
    unsigned short reserved;
    unsigned int size;            //file size, include this header
    unsigned short fileTotalCrc;  //file crc16, not include ths header
    unsigned short fileHeaderCrc; //file header crc16
};

struct __attribute__((packed)) CSOTSTD_SectionHeader {
    unsigned int type;
    unsigned int offset;
    unsigned int size;
    unsigned short SectionCrc;
    unsigned short reserved;
};

struct __attribute__((packed)) CSOTSTD_DemuraPara {
    unsigned char mode;
    unsigned char planeNum;
    unsigned char hBlockSize;
    unsigned char vBlockSize;
    unsigned short hLutNum;
    unsigned short vLutNum;
    unsigned char intBitWidth;
    unsigned char decimalsBitWidth;
    unsigned char reserved1[2];
    unsigned short compValGainR;
    unsigned short compValGainG;
    unsigned short compValGainB;
    unsigned short reserved2;
    unsigned short compValOffsetR;
    unsigned short compValOffsetG;
    unsigned short compValOffsetB;
    unsigned short reserved3;
    unsigned short Plane1LevelR[9];
    unsigned short Plane1LevelG[9];
    unsigned short Plane1LevelB[9];
    unsigned short reserved4[9];
    unsigned short planeB1SlopeR;
    unsigned short plane12SlopeR;
    unsigned short plane23SlopeR;
    unsigned short plane45SlopeR;
    unsigned short plane56SlopeR;
    unsigned short plane67SlopeR;
    unsigned short plane78SlopeR;
    unsigned short plane89SlopeR;
    unsigned short plane9WSlopeR;
    unsigned short planeB1SlopeG;
    unsigned short plane12SlopeG;
    unsigned short plane23SlopeG;
    unsigned short plane34SlopeG;
    unsigned short plane45SlopeG;
    unsigned short plane56SlopeG;
    unsigned short plane67SlopeG;
    unsigned short plane78SlopeG;
    unsigned short plane89SlopeG;
    unsigned short plane9WSlopeG;
    unsigned short planeB1SlopeB;
    unsigned short plane12SlopeB;
    unsigned short plane23SlopeB;
    unsigned short plane34SlopeB;
    unsigned short plane45SlopeB;
    unsigned short plane56SlopeB;
    unsigned short plane67SlopeB;
    unsigned short plane78SlopeB;
    unsigned short plane89SlopeB;
    unsigned short plane9WSlopeB;
    unsigned short reserved5[16];
};

struct __attribute__((packed)) CSOTSTD_DemuraData {
    struct CSOTSTD_FileHeader fileHeader;
    struct CSOTSTD_SectionHeader sectionPara;  // section description for DemuraPara
    struct CSOTSTD_SectionHeader sectionLut;   // section description for demuraLut
    struct CSOTSTD_DemuraPara demuraPara;
    unsigned char *demuraLut;
};
/******************************************************************/

class DemuraCsotStandard: public DemuraBaseDevice {
public:
    DemuraCsotStandard();
    virtual ~DemuraCsotStandard();
    static bool detect(char *flashPath);
    virtual int Init(char *flashPath);
    virtual DemuraBufferInfo_t * GenerateBuffer();
    virtual bool DestroyBuffer(DemuraBufferInfo_t * buf);
    virtual void PrintInfo(int fd);
    virtual void UnInit(void);
private:
    bool GetFlashData(int fd, int offset, void *buf, int bufSize);
    bool FillPlaneBuffer(void);
    bool FillLutPlaneBuffer(void);
    unsigned short CalculateCrc16(unsigned char *buf, unsigned int len);
    struct CSOTSTD_DemuraData demuraFlashData;
    unsigned int **planeBuffer;  //store 12bits plane data
    DemuraBufferInfo_t lutPlaneBuf;
};


#endif //_C_DEMURACSOTSTANDARD_H

