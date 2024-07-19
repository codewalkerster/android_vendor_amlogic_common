/*
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef CPQEXTDB_H_
#define CPQEXTDB_H_

#include <cutils/properties.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "CSqlite.h"
#include "PQType.h"

using namespace android;

typedef struct extdb_attribute_s {
    String8 ToolVersion;
    String8 ProjectVersion; //project bringup date
    String8 GenerateTime;
    String8 ChipVersion; //chip information
    String8 dbversion; //each time db version
    String8 reserved;
} extdb_attribute_t;


#ifdef getSqlParams
#undef getSqlParams
#endif
#define getSqlParams(func, buffer, args...) \
    do{\
        sprintf(buffer, ##args);\
        SYS_LOGV("getSqlParams for %s\n", func);\
        SYS_LOGV("%s = %s\n",#buffer, buffer);\
    }while(0)


class CPQExtdb: public CSqlite {
public:
    CPQExtdb();
    ~CPQExtdb();

    int openPqExtDB(const char *db_path);
    int closePqExtDB(void);

    int PQ_GetOsdSharpnessParams(int level, source_input_param_t source_input_param, am_regs_t *regs);

private:
    bool PQ_GetDataBaseAttribute(extdb_attribute_t *DbAttribute);
    bool CheckIdExistInDb(const char *Id, const char *TableName);
    int getRegValuesByValue(const char *name, const char *f_name, const char *f2_name, const int val, const int val2, am_regs_t *regs);
    String8 GetTableName(const char *GeneralTableName, source_input_param_t source_input_param);
};
#endif
