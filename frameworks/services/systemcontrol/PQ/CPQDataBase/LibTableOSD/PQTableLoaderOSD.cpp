#include <string.h>
#include <stdlib.h>
#include <climits>
#include "PQTableLoaderOSD.h"

extern TABLE_VER_OSD                mVerInfoOSD;
extern TABLE_DATA_STRUCT            mNonlinearMappingTable[];
extern TABLE_DATA_STRUCT            mPictureModeTable[];
extern TABLE_DATA_STRUCT            mColorTempTable[];
extern TABLE_DATA_STRUCT            mColorCustomizeTable[];

extern TABLE_PICTURE_SETTING_EXT    mPictureSettingTable;

static TABLE_VER_OSD                *m_VerInfoOSD = NULL;
static TABLE_DATA_STRUCT            *m_NonlinearMappingTable = NULL;
static TABLE_DATA_STRUCT            *m_PictureModeTable = NULL;
static TABLE_DATA_STRUCT            *m_ColorTempTable = NULL;

unsigned int                        NonlinearMappingOSDTableNum = 0;
unsigned int                        PictureModeOSDTableNum = 0;
unsigned int                        ColorTempOSDTableNum = 0;

extern int GetNonlinearMappingTableSize(void);
extern int GetColorTempTableSize(void);
extern int GetPictureModeTableSize(void);
extern int GetColorCustomizeTableSize(void);

#define MAX_TABLE_SIZE 128
#define INT_VALUE_MAX_RANGE    2147483647 //1<<31 -1

bool GetPqOsdVerBuffer(void** ppBuffer, int* piBufferLen)
{
    int iSize = sizeof(PQ_OSD_TABLE_STRUCT_HEADER) + sizeof(TABLE_VER_OSD);
    void* pBuffer = malloc(iSize);
    unsigned char* pCurPtr = (unsigned char*)pBuffer;
    if (pCurPtr == NULL) {
        return false;
    }

    PQ_OSD_TABLE_STRUCT_HEADER header;
    memset(&header, 0, sizeof(PQ_OSD_TABLE_STRUCT_HEADER));
    header.TotalSize = iSize;
    header.TableSize = sizeof(TABLE_VER_OSD);
    header.TableOffset = PQ_OSD_TABLE_VERSION;
    header.TableNum = 1;

    memcpy(pCurPtr, &header, sizeof(header));
    pCurPtr += sizeof(header);
    memcpy(pCurPtr, &mVerInfoOSD, sizeof(TABLE_VER_OSD));

    *ppBuffer = pBuffer;
    *piBufferLen = iSize;
    return true;
}

bool GetNonlinearMappingBuffer(void** ppBuffer, int* piBufferLen)
{
    void* TableArray[MAX_TABLE_SIZE];
    int iTableArrayIdx = 0;
    int iCnt1 = 0, iCnt2 = 0;
    int iIndexTableLen = GetNonlinearMappingTableSize();
    int tableMapping[MAX_TABLE_SIZE];

    for (iCnt1 = 0; iCnt1 < iIndexTableLen; iCnt1++) {
        bool bExist = false;

        for (iCnt2 = 0; iCnt2 < iTableArrayIdx; iCnt2++) {
            if (mNonlinearMappingTable[iCnt1].tableData == TableArray[iCnt2]) {
                tableMapping[iCnt1] = iCnt2;
                bExist = true;
                break;
            }
        }

        if (!bExist) {
            TableArray[iTableArrayIdx] = mNonlinearMappingTable[iCnt1].tableData;
            tableMapping[iCnt1] = iTableArrayIdx;
            iTableArrayIdx ++;
            if (iTableArrayIdx == MAX_TABLE_SIZE) {
                break;
            }
        }
    }

    int iSize = sizeof(PQ_OSD_TABLE_STRUCT_HEADER) + sizeof(NonlinearModeType) * iTableArrayIdx + sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE) * iIndexTableLen;
    void* pBuffer = malloc(iSize);
    unsigned char* pCurPtr = (unsigned char*)pBuffer;
    if (pCurPtr == NULL) {
        return false;
    }

    PQ_OSD_TABLE_STRUCT_HEADER header;
    memset(&header, 0, sizeof(PQ_OSD_TABLE_STRUCT_HEADER));

    header.TotalSize = iSize;
    header.TableSize = sizeof(NonlinearModeType) * iTableArrayIdx;
    header.IndexTableSize = sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE) * iIndexTableLen;
    header.TableOffset = PQ_OSD_TABLE_NONLINEARMAPPING;
    header.TableNum = iTableArrayIdx;
    header.IndexTableNum = iIndexTableLen;

    memcpy(pCurPtr, &header, sizeof(header));
    pCurPtr += sizeof(header);

    for (iCnt1 = 0; iCnt1 < iTableArrayIdx; iCnt1 ++) {
        memcpy(pCurPtr, TableArray[iCnt1], sizeof(NonlinearModeType));
        pCurPtr += sizeof(NonlinearModeType);
    }

    PQ_OSD_TABLE_DATA_STRUCT_SAVE SaveIndex;

    for (iCnt1 = 0; iCnt1 < GetNonlinearMappingTableSize(); iCnt1 ++) {
        SaveIndex.source = (char)mNonlinearMappingTable[iCnt1].source;
        SaveIndex.timing = (char)mNonlinearMappingTable[iCnt1].timing;
        SaveIndex.tableDataLen = (char)mNonlinearMappingTable[iCnt1].tableDataLen;
        SaveIndex.tableDataIdx = (short)tableMapping[iCnt1];
        memcpy(pCurPtr, &SaveIndex, sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE));
        pCurPtr += sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE);
    }

    *ppBuffer = pBuffer;
    *piBufferLen = iSize;
    return true;
}

bool GetPictureModeBuffer(void** ppBuffer, int* piBufferLen)
{
    void* TableArray[MAX_TABLE_SIZE];
    int iTableArrayIdx = 0;
    int iCnt1 = 0, iCnt2 = 0;
    int iIndexTableLen = GetPictureModeTableSize();
    int iIndexTableSize = sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE) * GetPictureModeTableSize();
    int iTableLen[MAX_TABLE_SIZE];
    int iTableSize = 0;
    int tableMapping[MAX_TABLE_SIZE];

    for (iCnt1 = 0; iCnt1 < iIndexTableLen; iCnt1++) {
        bool bExist = false;

        for (iCnt2 = 0; iCnt2 < iTableArrayIdx; iCnt2++) {
            if (mPictureModeTable[iCnt1].tableData == TableArray[iCnt2]) {
                tableMapping[iCnt1] = iCnt2;
                bExist = true;
                break;
            }
        }

        if (!bExist) {
            TableArray[iTableArrayIdx] = mPictureModeTable[iCnt1].tableData;
            iTableLen[iTableArrayIdx] = mPictureModeTable[iCnt1].tableDataLen;
            iTableSize += sizeof(PICTURE_MODE_DATA) * mPictureModeTable[iCnt1].tableDataLen;
            tableMapping[iCnt1] = iTableArrayIdx;
            iTableArrayIdx ++;
            if (iTableArrayIdx == MAX_TABLE_SIZE) {
                printf("[Error] Table out of range!!\n");
                break;
            }
        }
    }

    int iSize = sizeof(PQ_OSD_TABLE_STRUCT_HEADER) + iTableSize + iIndexTableSize;

    void* pBuffer = malloc(iSize);
    memset(pBuffer, 0, iSize);

    unsigned char* pCurPtr = (unsigned char*)pBuffer;
    if (pCurPtr == NULL) {
        printf("%s:%s(%d)can not alloc memory\n",__FILE__,__func__,__LINE__);
        return false;
    }

    PQ_OSD_TABLE_STRUCT_HEADER header;
    memset(&header, 0, sizeof(PQ_OSD_TABLE_STRUCT_HEADER));
    header.TotalSize = iSize;
    header.TableSize = iTableSize;
    header.IndexTableSize = iIndexTableSize;
    header.TableOffset = PQ_OSD_TABLE_PICTUREMODE;
    header.TableNum = iTableArrayIdx;
    header.IndexTableNum = iIndexTableLen;
    header.Reserved = 0;

    memcpy(pCurPtr, &header, sizeof(header));
    pCurPtr += sizeof(header);

    for (iCnt1 = 0; iCnt1 < iTableArrayIdx; iCnt1 ++) {
        memcpy(pCurPtr, TableArray[iCnt1], iTableLen[iCnt1] * sizeof(PICTURE_MODE_DATA));
        pCurPtr += iTableLen[iCnt1] * sizeof(PICTURE_MODE_DATA);
    }

    PQ_OSD_TABLE_DATA_STRUCT_SAVE SaveIndex;

    for (iCnt1 = 0; iCnt1 < iIndexTableLen; iCnt1 ++) {
        SaveIndex.source = (char)mPictureModeTable[iCnt1].source;
        SaveIndex.timing = (char)mPictureModeTable[iCnt1].timing;
        SaveIndex.tableDataLen = (unsigned int)mPictureModeTable[iCnt1].tableDataLen;
        SaveIndex.tableDataIdx = (short)tableMapping[iCnt1];
        memcpy(pCurPtr, &SaveIndex, sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE));
        pCurPtr += sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE);
    }

    *ppBuffer = pBuffer;
    *piBufferLen = iSize;

    return true;
}

bool GetColorTempBuffer(void** ppBuffer, int* piBufferLen)
{
    void* TableArray[MAX_TABLE_SIZE];
    int iTableArrayIdx = 0;
    int iCnt1 = 0, iCnt2 = 0;
    int iHeaderSize = sizeof(PQ_OSD_TABLE_STRUCT_HEADER);
    int iIndexTableLen = GetColorTempTableSize();
    int iIndexTableSize = sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE) * GetColorTempTableSize();
    int iTableLen[MAX_TABLE_SIZE];
    int iTableSize = 0;
    int tableMapping[MAX_TABLE_SIZE];


    for (iCnt1 = 0; iCnt1 < iIndexTableLen; iCnt1++) {
        bool bExist = false;

        for (iCnt2 = 0; iCnt2 < iTableArrayIdx; iCnt2++) {
            if (mColorTempTable[iCnt1].tableData == TableArray[iCnt2]) {
                tableMapping[iCnt1] = iCnt2;
                bExist = true;
                break;
            }
        }

        if (!bExist) {
            TableArray[iTableArrayIdx] = mColorTempTable[iCnt1].tableData;
            iTableLen[iTableArrayIdx] = mColorTempTable[iCnt1].tableDataLen;
            iTableSize += sizeof(COLORTEMP_DATA) * mColorTempTable[iCnt1].tableDataLen;
            tableMapping[iCnt1] = iTableArrayIdx;
            iTableArrayIdx ++;
            if (iTableArrayIdx == MAX_TABLE_SIZE) {
                break;
            }
        }
    }

    int iSize =  iHeaderSize + iTableSize + iIndexTableSize;

    void* pBuffer = malloc(iSize);
    unsigned char* pCurPtr = (unsigned char*)pBuffer;
    if (pCurPtr == NULL) {
        return false;
    }

    PQ_OSD_TABLE_STRUCT_HEADER header;
    memset(&header, 0, iHeaderSize);

    header.TotalSize = iSize;
    header.TableSize = iTableSize;
    header.IndexTableSize = iIndexTableSize;
    header.TableOffset = PQ_OSD_TABLE_COLORTEMP;
    header.TableNum = iTableArrayIdx;
    header.IndexTableNum = iIndexTableLen;
    header.Reserved = COLOR_TMP_MODE_MAX;

    memcpy(pCurPtr, &header, sizeof(header));
    pCurPtr += sizeof(header);

    for (iCnt1 = 0; iCnt1 < iTableArrayIdx; iCnt1 ++) {
        memcpy(pCurPtr, TableArray[iCnt1], sizeof(COLORTEMP_DATA) * iTableLen[iCnt1]);
        pCurPtr += sizeof(COLORTEMP_DATA) * iTableLen[iCnt1];
    }

    PQ_OSD_TABLE_DATA_STRUCT_SAVE SaveIndex;

    for (iCnt1 = 0; iCnt1 < iIndexTableLen; iCnt1 ++) {
        SaveIndex.source = (char)mColorTempTable[iCnt1].source;
        SaveIndex.timing = (char)mColorTempTable[iCnt1].timing;
        SaveIndex.tableDataLen = (unsigned int)mColorTempTable[iCnt1].tableDataLen;
        SaveIndex.tableDataIdx = (short)tableMapping[iCnt1];
        memcpy(pCurPtr, &SaveIndex, sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE));

        pCurPtr += sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE);
    }

    *ppBuffer = pBuffer;
    *piBufferLen = iSize;

    return true;
}

bool GetColorCustomizeBuffer(void** ppBuffer, int* piBufferLen)
{
    void* TableArray[MAX_TABLE_SIZE];
    int iTableArrayIdx = 0;
    int iCnt1 = 0, iCnt2 = 0;
    int iHeaderSize = sizeof(PQ_OSD_TABLE_STRUCT_HEADER);
    int iIndexTableLen = GetColorCustomizeTableSize();
    int iIndexTableSize = sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE) * GetColorCustomizeTableSize();
    int iTableLen[MAX_TABLE_SIZE];
    int iTableSize = 0;
    int tableMapping[MAX_TABLE_SIZE];


    for (iCnt1 = 0; iCnt1 < iIndexTableLen; iCnt1++) {
        bool bExist = false;

        for (iCnt2 = 0; iCnt2 < iTableArrayIdx; iCnt2++) {
            if (mColorCustomizeTable[iCnt1].tableData == TableArray[iCnt2]) {
                tableMapping[iCnt1] = iCnt2;
                bExist = true;
                break;
            }
        }

        if (!bExist) {
            TableArray[iTableArrayIdx] = mColorCustomizeTable[iCnt1].tableData;
            iTableLen[iTableArrayIdx] = mColorCustomizeTable[iCnt1].tableDataLen;
            iTableSize += sizeof(TABLE_CMS) * mColorCustomizeTable[iCnt1].tableDataLen;
            tableMapping[iCnt1] = iTableArrayIdx;
            iTableArrayIdx ++;
            if (iTableArrayIdx == MAX_TABLE_SIZE) {
            break;
            }
        }
    }

    int iSize =  iHeaderSize + iTableSize + iIndexTableSize;

    void* pBuffer = malloc(iSize);
    unsigned char* pCurPtr = (unsigned char*)pBuffer;
    if (pCurPtr == NULL) {
        return false;
    }

    PQ_OSD_TABLE_STRUCT_HEADER header;
    memset(&header, 0, iHeaderSize);

    header.TotalSize = iSize;
    header.TableSize = iTableSize;
    header.IndexTableSize = iIndexTableSize;
    header.TableOffset = PQ_OSD_TABLE_COLORTEMP;
    header.TableNum = iTableArrayIdx;
    header.IndexTableNum = iIndexTableLen;
    header.Reserved = 0;

    memcpy(pCurPtr, &header, sizeof(header));
    pCurPtr += sizeof(header);

    for (iCnt1 = 0; iCnt1 < iTableArrayIdx; iCnt1 ++) {
        memcpy(pCurPtr, TableArray[iCnt1], sizeof(TABLE_CMS) * iTableLen[iCnt1]);
        pCurPtr += sizeof(TABLE_CMS) * iTableLen[iCnt1];
    }

    PQ_OSD_TABLE_DATA_STRUCT_SAVE SaveIndex;

    for (iCnt1 = 0; iCnt1 < iIndexTableLen; iCnt1 ++) {
        SaveIndex.source = (char)mColorTempTable[iCnt1].source;
        SaveIndex.timing = (char)mColorTempTable[iCnt1].timing;
        SaveIndex.tableDataLen = (unsigned int)mColorTempTable[iCnt1].tableDataLen;
        SaveIndex.tableDataIdx = (short)tableMapping[iCnt1];
        memcpy(pCurPtr, &SaveIndex, sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE));

        pCurPtr += sizeof(PQ_OSD_TABLE_DATA_STRUCT_SAVE);
    }

    *ppBuffer = pBuffer;
    *piBufferLen = iSize;

    return true;
}

bool GetPictureSettingExtBuffer(void** ppBuffer, int* piBufferLen)
{
    int iSize = sizeof(PQ_OSD_TABLE_STRUCT_HEADER) + sizeof(TABLE_PICTURE_SETTING_EXT);
    void* pBuffer = malloc(iSize);
    unsigned char* pCurPtr = (unsigned char*)pBuffer;
    if (pCurPtr == NULL) {
        return false;
    }

    PQ_OSD_TABLE_STRUCT_HEADER header;
    memset(&header, 0, sizeof(PQ_OSD_TABLE_STRUCT_HEADER));
    header.TotalSize = iSize;
    header.TableSize = sizeof(TABLE_PICTURE_SETTING_EXT);
    header.TableOffset = PQ_OSD_TABLE_PICTURE_SETTING_EXT;
    header.TableNum = 1;

    memcpy(pCurPtr, &header, sizeof(header));
    pCurPtr += sizeof(header);
    memcpy(pCurPtr, &mPictureSettingTable, sizeof(TABLE_PICTURE_SETTING_EXT));

    *ppBuffer = pBuffer;
    *piBufferLen = iSize;
    return true;
}

bool PQTableGenerate_Osd(char* pPanelFile)
{
    if (NULL == pPanelFile)
        return false;

    FILE *pFile = fopen(pPanelFile, "wb");
    if (NULL == pFile)
        return false;

    bool ret = false;
    PQ_OSD_FILE_HEADER header;
    memset(&header, 0, sizeof(header));

    void* PqOsdVerBuf = NULL;
    int PqOsdVerLen = 0;
    ret |= !GetPqOsdVerBuffer(&PqOsdVerBuf, &PqOsdVerLen);

    void* NonlinearMappingBuff = NULL;
    int NonlinearMappingBuffLen = 0;
    ret |= !GetNonlinearMappingBuffer(&NonlinearMappingBuff, &NonlinearMappingBuffLen);

    void* PictureModeBuf = NULL;
    int PictureModeLen = 0;
    ret |= !GetPictureModeBuffer(&PictureModeBuf, &PictureModeLen);

    void* ColorTempBuf = NULL;
    int ColorTempBufLen = 0;
    ret |= !GetColorTempBuffer(&ColorTempBuf, &ColorTempBufLen);

    void* ColorCustomizeBuf = NULL;
    int ColorCustomizeBufLen = 0;
    ret |= !GetColorCustomizeBuffer(&ColorCustomizeBuf, &ColorCustomizeBufLen);

    void* PictureSettingExtBuf = NULL;
    int PictureSettingExtLen= 0;
    ret |= !GetPictureSettingExtBuffer(&PictureSettingExtBuf, &PictureSettingExtLen);

    unsigned int crcSize = PqOsdVerLen + NonlinearMappingBuffLen + PictureModeLen + ColorTempBufLen + ColorCustomizeBufLen + PictureSettingExtLen;
    printf("%s crcSize = %d, PqOsdVerLen = %d, NonlinearMappingBuffLen = %d, PictureModeLen = %d, ColorTempBufLen = %d ColorCustomizeBufLen = %d PictureSettingExtBuf = %d\n", __func__, crcSize,
            PqOsdVerLen, NonlinearMappingBuffLen, PictureModeLen, ColorTempBufLen, ColorCustomizeBufLen, PictureSettingExtLen);

    unsigned char* crcBuffer = NULL;
    crcBuffer = (unsigned char *)malloc(crcSize);
    memcpy(crcBuffer, (unsigned char *)PqOsdVerBuf, PqOsdVerLen);
    memcpy(crcBuffer+PqOsdVerLen, (unsigned char *)NonlinearMappingBuff, NonlinearMappingBuffLen);
    memcpy(crcBuffer+PqOsdVerLen+NonlinearMappingBuffLen, (unsigned char *)PictureModeBuf, PictureModeLen);
    memcpy(crcBuffer+PqOsdVerLen+NonlinearMappingBuffLen+PictureModeLen, (unsigned char *)ColorTempBuf, ColorTempBufLen);
    memcpy(crcBuffer+PqOsdVerLen+NonlinearMappingBuffLen+PictureModeLen+ColorTempBufLen, (unsigned char *)ColorCustomizeBuf, ColorCustomizeBufLen);
    memcpy(crcBuffer+PqOsdVerLen+NonlinearMappingBuffLen+PictureModeLen+ColorTempBufLen+ColorCustomizeBufLen, (unsigned char *)PictureSettingExtBuf, PictureSettingExtLen);

    unsigned short int crc = get_crc16((unsigned const char *)crcBuffer, crcSize);
    printf("%s crc = %d\n", __func__, crc);

    if (!ret) {
        header.Size = sizeof(header);
        header.PqOsdVerOffset = 0;
        header.NonlinearMappingOffset = PqOsdVerLen;
        header.PictureModeOffset =      PqOsdVerLen + NonlinearMappingBuffLen;
        header.ColorTempOffset =        PqOsdVerLen + NonlinearMappingBuffLen + PictureModeLen;
        header.ColorCustomizeOffset =   PqOsdVerLen + NonlinearMappingBuffLen + PictureModeLen + ColorTempBufLen;
        header.PictureSettingExt =      PqOsdVerLen + NonlinearMappingBuffLen + PictureModeLen  + ColorTempBufLen + ColorCustomizeBufLen;

        header.chip = 0;
        header.crc = crc;

        fwrite(&header, sizeof(header), 1, pFile);
        fwrite(PqOsdVerBuf, PqOsdVerLen, 1, pFile);
        fwrite(NonlinearMappingBuff, NonlinearMappingBuffLen, 1, pFile);
        fwrite(PictureModeBuf, PictureModeLen, 1, pFile);
        fwrite(ColorTempBuf, ColorTempBufLen, 1, pFile);
        fwrite(ColorCustomizeBuf, ColorCustomizeBufLen, 1, pFile);
        fwrite(PictureSettingExtBuf, PictureSettingExtLen, 1, pFile);
    }

    if (PqOsdVerBuf != NULL)
        free(PqOsdVerBuf);
    if (NonlinearMappingBuff != NULL)
        free(NonlinearMappingBuff);
    if (PictureModeBuf != NULL)
        free(PictureModeBuf);
    if (ColorTempBuf != NULL)
        free(ColorTempBuf);
    if (ColorCustomizeBuf != NULL)
        free(ColorCustomizeBuf);
    if (PictureSettingExtBuf != NULL)
        free(PictureSettingExtBuf);

    fclose(pFile);
    free(crcBuffer);

    return true;
}

static unsigned int totalSize = 0;
static unsigned char* curDataBuf = NULL;
static unsigned char* verBuf = NULL;
static unsigned char* mappingBuf = NULL;
static unsigned char* picBuf = NULL;
static unsigned char* ctmpBuf = NULL;

unsigned short int get_crc16(unsigned const char *ptr, unsigned int len)
{
    #define CRC_POLY1 0x8005U //多项式x^16x^15x^21(0x8005)
    unsigned short int crc16 = 0xFFFFU; //设置crc寄存器,预设值0xFFFF
    unsigned short int i, j;

    for (i=0; i<len; i++) {
        crc16 ^= (unsigned short int)ptr[i] << (16 - CHAR_BIT); //低八位异或,高八位不变,值存入crc寄存器
        for (j=0; j < CHAR_BIT; j++) {
            if (crc16 & 0x8000U) {
                crc16 = (crc16 << 1) ^ CRC_POLY1;
            } else {
                crc16 <<= 1;
            }
        }
    }

    crc16 = ~crc16 & 0xFFFFU;

    printf("%s crc16=%d\n", __func__, crc16);
    return crc16;
}

bool PQOSD_TableLoader_GetTable(FILE *pPanelFile, PQ_OSD_TABLE_TYPE type, PQ_OSD_TABLE_STRUCT* pTable)
{
    if (NULL == pPanelFile || NULL == pTable)
        return false;

    int ret_header = -1, ret_t_header = -1, ret_t_array = -1, ret_index_t = -1, ret_seek = -1;
    unsigned int iOffset = 0;

    printf("%s %d type = %d\n", __func__, __LINE__, type);

    rewind(pPanelFile);

    PQ_OSD_FILE_HEADER header;
    memset(&header, 0, sizeof(header));
    ret_header = fread(&header, sizeof(PQ_OSD_FILE_HEADER), 1, pPanelFile);
    if (ret_header <= 0)    return false;

    switch (type) {
        case PQ_OSD_TABLE_VERSION:
            iOffset = header.PqOsdVerOffset;
            break;
        case PQ_OSD_TABLE_NONLINEARMAPPING:
            iOffset = header.NonlinearMappingOffset;
            break;
        case PQ_OSD_TABLE_PICTUREMODE:
            iOffset = header.PictureModeOffset;
            break;
        case PQ_OSD_TABLE_COLORTEMP:
            iOffset = header.ColorTempOffset;
            break;
        default:
            return false;
    }
    printf("%s %d iOffset = %d\n", __func__, __LINE__, iOffset);
    if (iOffset > INT_VALUE_MAX_RANGE)    return false;

    ret_seek = fseek(pPanelFile, iOffset, SEEK_CUR);
    if (ret_seek != 0) {
        printf("%s %d fseek fail!!!\n",__func__, __LINE__);
        return false;
    }

    ret_t_header = fread(&(pTable->header), sizeof(PQ_OSD_TABLE_STRUCT_HEADER), 1, pPanelFile);
    if (ret_t_header <= 0) {
        printf("%s %d fread fail!!!\n",__func__, __LINE__);
        return false;
    }

    printf("%s %d TableSize = %d, IndexTableSize = %d, TotalSize = %d\n", __func__, __LINE__,
            pTable->header.TableSize, pTable->header.IndexTableSize, pTable->header.TotalSize);

    if (pTable->header.TableSize > 0 && pTable->header.TableSize < INT_VALUE_MAX_RANGE) {
        pTable->pTableArray = malloc(pTable->header.TableSize);
        if (pTable->pTableArray == NULL) {
            printf("%s %d malloc iTableArraySize memory fail!!!\n", __func__, __LINE__);
            return false;
        }

        ret_t_array = fread(pTable->pTableArray, pTable->header.TableSize, 1, pPanelFile);
        if (ret_t_array <= 0) {
            printf("%s %d read pTableArray fail!!!\n", __func__, __LINE__);
            return false;
        }
    } else {
        printf("%s %d header.TableSize size NG\n", __func__, __LINE__);
    }

    if (pTable->header.IndexTableSize > 0 && pTable->header.IndexTableSize < INT_VALUE_MAX_RANGE) {
        pTable->pIndexTable = malloc(pTable->header.IndexTableSize);
        if (pTable->pIndexTable == NULL) {
            printf("%s %d malloc iIndexTableSize memory fail!!!\n", __func__, __LINE__);
            return false;
        }

        ret_index_t = fread(pTable->pIndexTable, pTable->header.IndexTableSize, 1, pPanelFile);
        if (ret_index_t <= 0) {
            printf("%s %d read iIndexTableSize fail!!!\n", __func__, __LINE__);
            return false;
        }
    } else {
        printf("%s %d header.IndexTableSize size NG\n", __func__, __LINE__);
    }

    //crc check start
    if (pTable->header.TotalSize > 0 && pTable->header.TotalSize < INT_VALUE_MAX_RANGE
        && pTable->header.TableSize < INT_VALUE_MAX_RANGE
        && pTable->header.IndexTableSize < INT_VALUE_MAX_RANGE) {
        curDataBuf = (unsigned char *)malloc(pTable->header.TotalSize);
        memcpy(curDataBuf, &(pTable->header), sizeof(PQ_OSD_TABLE_STRUCT_HEADER));
        memcpy(curDataBuf + sizeof(PQ_OSD_TABLE_STRUCT_HEADER), pTable->pTableArray, pTable->header.TableSize);
        memcpy(curDataBuf + sizeof(PQ_OSD_TABLE_STRUCT_HEADER) + pTable->header.TableSize, pTable->pIndexTable, pTable->header.IndexTableSize);
    } else {
        printf("%s TotalSize/TableSize/IndexTableSize size NG!!!\n", __func__);
        return false;
    }

    totalSize = totalSize + pTable->header.TotalSize;
    printf("%s %d totalSize=%d, pTable->header.TotalSize=%d\n", __func__, __LINE__, totalSize, pTable->header.TotalSize);
    if (totalSize > INT_VALUE_MAX_RANGE) {
        free(curDataBuf);
        return false;
    }

    if (type == PQ_OSD_TABLE_VERSION) {
        verBuf = (unsigned char *)malloc(totalSize);
        memcpy(verBuf, curDataBuf, pTable->header.TotalSize);
    } else if (type == PQ_OSD_TABLE_NONLINEARMAPPING) {
        mappingBuf = (unsigned char *)malloc(totalSize);
        memcpy(mappingBuf, verBuf, iOffset);
        memcpy(mappingBuf+iOffset, curDataBuf, pTable->header.TotalSize);
        free(verBuf);
    } else if (type == PQ_OSD_TABLE_PICTUREMODE) {
        picBuf = (unsigned char *)malloc(totalSize);
        memcpy(picBuf, mappingBuf, iOffset);
        memcpy(picBuf+iOffset, curDataBuf, pTable->header.TotalSize);
        free(mappingBuf);
    } else if (type == PQ_OSD_TABLE_COLORTEMP) {
        ctmpBuf = (unsigned char *)malloc(totalSize);
        memcpy(ctmpBuf, picBuf, iOffset);
        memcpy(ctmpBuf+iOffset, curDataBuf, pTable->header.TotalSize);
        free(picBuf);
        unsigned short int crcCheck  = get_crc16((unsigned const char *)ctmpBuf, totalSize);
        if (header.crc == crcCheck) {
            printf("%s %d crc check success\n", __func__, __LINE__);
        } else {
            printf("%s %d crc check fail!!! header.crc=%d, crcCheck=%d\n", __func__, __LINE__, header.crc, crcCheck);
            return false;
        }
        free(ctmpBuf);
    }

    free(curDataBuf);
    //crc check end

    printf("%s %d load bin data end\n", __func__, __LINE__);

    return true;
}

bool xLoadTableDataToMem(PQ_OSD_TABLE_STRUCT *m_PQOsdTable)
{
    if (m_PQOsdTable == NULL) {
        return false;
    }

    //VER
    m_VerInfoOSD = (TABLE_VER_OSD*)m_PQOsdTable[PQ_OSD_TABLE_VERSION].pTableArray;

    //NonlinearMapping
    PQ_OSD_TABLE_DATA_STRUCT_SAVE* pTableIndex_Nonlinear = (PQ_OSD_TABLE_DATA_STRUCT_SAVE*)m_PQOsdTable[PQ_OSD_TABLE_NONLINEARMAPPING].pIndexTable;
    NonlinearModeType* pTableArray_Nonlinear = (NonlinearModeType*)m_PQOsdTable[PQ_OSD_TABLE_NONLINEARMAPPING].pTableArray;
    NonlinearMappingOSDTableNum = m_PQOsdTable[PQ_OSD_TABLE_NONLINEARMAPPING].header.IndexTableNum;
    if (m_NonlinearMappingTable == NULL) {
        m_NonlinearMappingTable = (TABLE_DATA_STRUCT *)malloc(m_PQOsdTable[PQ_OSD_TABLE_NONLINEARMAPPING].header.TotalSize);
        if (m_NonlinearMappingTable == NULL) {
            printf("%s %d malloc m_NonlinearMappingTable memory fail!!!\n", __func__, __LINE__);
            return false;
        }
    }
    for (unsigned int i = 0; i < NonlinearMappingOSDTableNum; i++) {
        m_NonlinearMappingTable[i].source = (pq_source_input_t)pTableIndex_Nonlinear[i].source;
        m_NonlinearMappingTable[i].timing = (pq_sig_fmt_t)pTableIndex_Nonlinear[i].timing;
        m_NonlinearMappingTable[i].tableDataLen = pTableIndex_Nonlinear[i].tableDataLen;
        unsigned int offset = 0;
        for (short j = 0; j < pTableIndex_Nonlinear[i].tableDataIdx; j++) {
            offset += pTableIndex_Nonlinear[j].tableDataLen;
        }
        m_NonlinearMappingTable[i].tableData = (void*)&pTableArray_Nonlinear[offset];
    }

    //PICTURE MODE
    PQ_OSD_TABLE_DATA_STRUCT_SAVE* pTableIndex_pic = (PQ_OSD_TABLE_DATA_STRUCT_SAVE*)m_PQOsdTable[PQ_OSD_TABLE_PICTUREMODE].pIndexTable;
    PICTURE_MODE_DATA* pTableArray_pic = (PICTURE_MODE_DATA*)m_PQOsdTable[PQ_OSD_TABLE_PICTUREMODE].pTableArray;
    PictureModeOSDTableNum = m_PQOsdTable[PQ_OSD_TABLE_PICTUREMODE].header.IndexTableNum;

    if (m_PictureModeTable == NULL) {
        m_PictureModeTable = (TABLE_DATA_STRUCT *)malloc(m_PQOsdTable[PQ_OSD_TABLE_PICTUREMODE].header.TotalSize);
        if (m_PictureModeTable == NULL) {
            printf("%s %d malloc m_PictureModeTable memory fail!!!\n", __func__, __LINE__);
            return false;
        }
    }

    for (unsigned int i = 0; i < PictureModeOSDTableNum; i++) {
        m_PictureModeTable[i].source = (pq_source_input_t)pTableIndex_pic[i].source;
        m_PictureModeTable[i].timing = (pq_sig_fmt_t)pTableIndex_pic[i].timing;
        m_PictureModeTable[i].tableDataLen = pTableIndex_pic[i].tableDataLen;
        unsigned int offset = 0;
        for (short j = 0; j < pTableIndex_pic[i].tableDataIdx; j++) {
            offset += pTableIndex_pic[j].tableDataLen;
        }
        m_PictureModeTable[i].tableData = (void*)&pTableArray_pic[offset];
    }

    //COLOR TEMP
    PQ_OSD_TABLE_DATA_STRUCT_SAVE* pTableIndex_ct = (PQ_OSD_TABLE_DATA_STRUCT_SAVE*)m_PQOsdTable[PQ_OSD_TABLE_COLORTEMP].pIndexTable;
    COLORTEMP_DATA* pTableArray_ct = (COLORTEMP_DATA*)m_PQOsdTable[PQ_OSD_TABLE_COLORTEMP].pTableArray;
    ColorTempOSDTableNum = m_PQOsdTable[PQ_OSD_TABLE_COLORTEMP].header.IndexTableNum;
    if (m_ColorTempTable == NULL) {
        m_ColorTempTable = (TABLE_DATA_STRUCT *)malloc(m_PQOsdTable[PQ_OSD_TABLE_COLORTEMP].header.TotalSize);
        if (m_ColorTempTable == NULL) {
            printf("%s %d malloc m_ColorTempTable memory fail!!!\n", __func__, __LINE__);
            return false;
        }
    }

    for (unsigned int i = 0; i < ColorTempOSDTableNum; i++) {
        m_ColorTempTable[i].source =(pq_source_input_t)pTableIndex_ct[i].source;
        m_ColorTempTable[i].timing =(pq_sig_fmt_t)pTableIndex_ct[i].timing;
        m_ColorTempTable[i].tableDataLen = pTableIndex_ct[i].tableDataLen;
        unsigned int offset = 0;
        for (short j = 0; j < pTableIndex_ct[i].tableDataIdx; j++) {
            offset += pTableIndex_ct[j].tableDataLen;
        }
        m_ColorTempTable[i].tableData = (void*)&pTableArray_ct[offset];
    }

    return true;
}

void* GetPQOSDTableData(PQ_OSD_TABLE_TYPE type, unsigned int *indexTabLen)
{
        printf("%s %d Get data from bin\n", __func__, __LINE__);

    switch (type) {
        case PQ_OSD_TABLE_VERSION:
            *indexTabLen = 1;
            return (void*)m_VerInfoOSD;
        case PQ_OSD_TABLE_NONLINEARMAPPING:
            *indexTabLen = NonlinearMappingOSDTableNum;
            return (void*)m_NonlinearMappingTable;
        case PQ_OSD_TABLE_PICTUREMODE:
            *indexTabLen = PictureModeOSDTableNum;
            return (void*)m_PictureModeTable;
        case PQ_OSD_TABLE_COLORTEMP:
            *indexTabLen = ColorTempOSDTableNum;
            return (void*)m_ColorTempTable;
        default:
            break;
    }

    return NULL;
}

