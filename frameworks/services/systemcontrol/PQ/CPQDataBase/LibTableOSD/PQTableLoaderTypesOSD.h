#ifndef __PQ_TABLE_LOADER_TYPES_OSD_H__
#define __PQ_TABLE_LOADER_TYPES_OSD_H__

#pragma pack(4) //4byte

typedef enum _PQ_OSD_TABLE_TYPE {
    PQ_OSD_TABLE_VERSION = 0,
    PQ_OSD_TABLE_NONLINEARMAPPING,
    PQ_OSD_TABLE_PICTUREMODE,
    PQ_OSD_TABLE_COLORTEMP,
    PQ_OSD_TABLE_COLORCUSTOMIZE,
    PQ_OSD_TABLE_PICTURE_SETTING_EXT,
    PQ_OSD_TABLE_MAX,
} PQ_OSD_TABLE_TYPE;

typedef struct _PQ_OSD_FILE_HEADER {
    int Size;
    int PqOsdVerOffset;
    int NonlinearMappingOffset;
    int PictureModeOffset;
    int ColorTempOffset;
    int ColorCustomizeOffset;
    int PictureSettingExt;
    int chip;
    unsigned int crc;
} PQ_OSD_FILE_HEADER;

typedef struct _PQ_OSD_TABLE_STRUCT_HEADER {
    unsigned int    TotalSize;
    unsigned int    TableSize;
    unsigned int    IndexTableSize;
    unsigned char   TableOffset;
    unsigned char   TableNum;
    unsigned char   IndexTableNum;
    unsigned char   Reserved;
} PQ_OSD_TABLE_STRUCT_HEADER;

typedef struct _PQ_OSD_TABLE_STRUCT {
    PQ_OSD_TABLE_STRUCT_HEADER header;
    void*                      pTableArray;
    void*                      pIndexTable;
} PQ_OSD_TABLE_STRUCT;

typedef struct _PQ_OSD_TABLE_DATA_STRUCT_SAVE {
        char source;
        char timing;
        short tableDataIdx;
        unsigned int tableDataLen;
} PQ_OSD_TABLE_DATA_STRUCT_SAVE;

#pragma pack()

#endif
