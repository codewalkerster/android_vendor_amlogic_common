#ifndef __PQ_TABLE_LOADER_H__
#define __PQ_TABLE_LOADER_H__

#include <stdio.h>
#include "PQTableLoaderTypes.h"

bool PQTableGenerate(char* pPanelFile);
bool PQ_TableLoader_GetTable(FILE *pPanelFile, PQ_TABLE_TYPE type, PQ_TABLE_STRUCT* pTable);

#endif
