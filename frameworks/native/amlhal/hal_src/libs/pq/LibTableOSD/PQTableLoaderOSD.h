#ifndef __PQ_TABLE_LOADEROSD_H__
#define __PQ_TABLE_LOADEROSD_H__

#include <stdio.h>
#include "PQTableLoaderTypesOSD.h"
#include "PQTableTypeOSD.h"

bool PQTableGenerate_Osd(char* pPanelFile);
bool PQOSD_TableLoader_GetTable(FILE *pPanelFile, PQ_OSD_TABLE_TYPE type, PQ_OSD_TABLE_STRUCT* pTable);
unsigned short int get_crc16(unsigned const char *ptr, unsigned int len);
void IsLoadFromBinFile(bool enable);

int GetNonlinearOsdRemapVal(noline_params_type_t type, int Value);
bool xLoadTableDataToMem(PQ_OSD_TABLE_STRUCT *m_PQOsdTable);
void* GetPQOSDTableData(PQ_OSD_TABLE_TYPE type, unsigned int *indexTabLen);

//single picture mode table
bool PQTable_SetPictureModeData(PICTURE_MODE_DATA *pData);
PICTURE_MODE_DATA* PQTable_GetPictureModeData(void);

//single Color Temp table
bool PQTable_SetColorTempData(COLORTEMP_DATA *pData);
COLORTEMP_DATA* PQTable_GetColorTempData(void);

//single Nonlinear table
bool PQTable_SetNonlinearModeType(NonlinearModeType *pData);
NonlinearModeType* PQTable_GetNonlinearModeType(void);

int GetNonlinearMappingTableSize(void);
int GetColorTempTableSize(void);
int GetPictureModeTableSize(void);

#endif
