#ifndef __PQ_TABLE_LOADEROSD_H__
#define __PQ_TABLE_LOADEROSD_H__

#include <stdio.h>
#include "PQTableLoaderTypesOSD.h"
#include "PQTableTypeOSD.h"

bool PQTableGenerate_Osd(char* pPanelFile);
bool PQOSD_TableLoader_GetTable(FILE *pPanelFile, PQ_OSD_TABLE_TYPE type, PQ_OSD_TABLE_STRUCT* pTable);
unsigned short int get_crc16(unsigned const char *ptr, unsigned int len);
bool xLoadTableDataToMem(PQ_OSD_TABLE_STRUCT *m_PQOsdTable);
void* GetPQOSDTableData(PQ_OSD_TABLE_TYPE type, unsigned int *indexTabLen);

int GetNonlinearMappingTableSize(void);
int GetColorTempTableSize(void);
int GetPictureModeTableSize(void);
int GetColorCustomizeTableSize(void);

#endif
