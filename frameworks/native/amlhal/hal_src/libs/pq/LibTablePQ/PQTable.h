
#ifndef _PQ_TABLE_H
#define _PQ_TABLE_H

#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "PQTableType.h"
#include "PQTableLoader.h"

class PQTable       {
public:
    PQTable();
    ~PQTable();
    static PQTable *GetInstance();
    void Init();
    bool Set_PQBinPath(char *path);
    void* GetPQTableData(PQ_TABLE_TYPE type, int *indexTabLen);

    bool Set_VPQ_SharpnessTable(int index);
    bool Set_VPQ_GammaTable(int index);
    bool Set_VPQ_SRTable(int index);
    bool Set_VPQ_CM2Table(int index);
    bool Set_VPQ_LocalContrastTable(int index);
    bool Set_VPQ_DNLPTable(int index);

private:
    static PQTable *mInstance;
    bool Get_PQBinName(char *name);
    bool Load_PQBin(char *name);

protected:
    PQ_TABLE_STRUCT         m_PQTable[PQ_TABLE_MAX];


};

#endif

