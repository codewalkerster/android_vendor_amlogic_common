/*
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "SystemControl"
#define LOG_TV_TAG "CPQExtdb"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <utils/String8.h>

#include "CPQExtdb.h"
#include "CPQLog.h"

#define ID_FIELD            "TableID"
#define LEVEL_NAME          "Level"
#define CVBS_NAME_ID        "TVOUT_CVBS"
#define TABLE_NAME_ID       "TableName"

CPQExtdb::CPQExtdb()
{

}

CPQExtdb::~CPQExtdb()
{

}

int CPQExtdb::openPqExtDB(const char *db_path)
{
    SYS_LOGD("openPqExtDB path = %s", db_path);
    int rval;

    if (access(db_path, 0) < 0) {
        SYS_LOGE("pq_ext.db don't exist!\n");
        return -1;
    }

    closeDb();
    rval = openDb(db_path);

    String8 attributeVal;
    if (rval == 0) {
        extdb_attribute_t databaseAttribute;
        bool ret = PQ_GetDataBaseAttribute(&databaseAttribute);
        if (ret) {
            attributeVal = databaseAttribute.ToolVersion + " " +
                           databaseAttribute.ProjectVersion + " " +
                           databaseAttribute.dbversion + " " +
                           databaseAttribute.GenerateTime + " " +
                           databaseAttribute.ChipVersion;
        } else {
            attributeVal = "Get pq_ext.db Version failure!!!";
        }
        SYS_LOGD("%s = %s\n", "pq_ext.db.version", attributeVal.c_str());
    }

    return rval;
}

int CPQExtdb::closePqExtDB(void)
{
    return closeDb();
}

bool CPQExtdb::PQ_GetDataBaseAttribute(extdb_attribute_t *DbAttribute)
{
    bool ret = false;
    if (DbAttribute == NULL) {
        SYS_LOGE("%s: DbAttribute is NULL!\n", __FUNCTION__);
    } else {
        CSqlite::Cursor c;
        char sqlmaster[256] = {0};
        bool chipVersionExist = false;
        if (CheckIdExistInDb("ChipVersion", "PQ_VersionTable")) {
            chipVersionExist = true;
            getSqlParams(__FUNCTION__, sqlmaster,
                         "select ToolVersion,ProjectVersion,ChipVersion,dbversion,GenerateTime from PQ_VersionTable;");
        } else {
            chipVersionExist = false;
            getSqlParams(__FUNCTION__, sqlmaster,
                         "select ToolVersion,ProjectVersion,dbversion,GenerateTime from PQ_VersionTable;");
        }

        int rval = this->select(sqlmaster, c);

        if (!rval && c.getCount() > 0) {
            DbAttribute->ToolVersion = c.getString(0);
            DbAttribute->ProjectVersion = c.getString(1);
            if (chipVersionExist) {
                DbAttribute->ChipVersion = c.getString(2);
                DbAttribute->dbversion = c.getString(3);
                DbAttribute->GenerateTime = c.getString(4);
            } else {
                DbAttribute->ChipVersion = String8("");
                DbAttribute->dbversion = c.getString(2);
                DbAttribute->GenerateTime = c.getString(3);
            }

            SYS_LOGD("%s DbAttribute->ToolVersion %s\n", __FUNCTION__, DbAttribute->ToolVersion.c_str());
            SYS_LOGD("%s DbAttribute->ProjectVersion %s\n", __FUNCTION__, DbAttribute->ProjectVersion.c_str());
            SYS_LOGD("%s DbAttribute->dbversion %s\n", __FUNCTION__, DbAttribute->dbversion.c_str());
            SYS_LOGD("%s DbAttribute->ChipVersion %s\n", __FUNCTION__, DbAttribute->ChipVersion.c_str());
            SYS_LOGD("%s DbAttribute->GenerateTime %s\n", __FUNCTION__, DbAttribute->GenerateTime.c_str());

            ret = true;
        } else {
            SYS_LOGE("%s: select action failed!\n", __FUNCTION__);
            ret = false;
        }
    }

    return ret;
}

bool CPQExtdb::CheckIdExistInDb(const char *Id, const char *TableName)
{
    bool ret = false;
    char sqlmaster[256] = {0};
    CSqlite::Cursor tempCursor;

    getSqlParams(__FUNCTION__, sqlmaster,
                 "select sql from sqlite_master where type = 'table' and tbl_name = '%s';", TableName);

    int retVal = this->select(sqlmaster, tempCursor);
    if ((retVal == 0) && (tempCursor.moveToFirst())) {
        if (strstr(tempCursor.getString(0).c_str(), Id) != NULL) {
            ret = true;
        } else {
            ret = false;
        }
    } else {
        SYS_LOGE("%s: error!\n", __FUNCTION__);
        ret = false;
    }

    return ret;
}

String8 CPQExtdb::GetTableName(const char *GeneralTableName, source_input_param_t source_input_param)
{
    CSqlite::Cursor c;
    char sqlmaster[256] = {0};
    int ret = -1;

    getSqlParams(__FUNCTION__, sqlmaster, "select TableName from %s where "
                 "TVIN_PORT = %d and "
                 "TVIN_SIG_FMT = %d and "
                 "TVIN_TRANS_FMT = %d and "
                 "TVOUT_CVBS = %d ;", GeneralTableName, source_input_param.source_input,
                 source_input_param.sig_fmt, source_input_param.trans_fmt, OUTPUT_TYPE_LVDS);

    ret = this->select(sqlmaster, c);
    if (ret == 0) {
        if (c.moveToFirst()) {
            SYS_LOGD("%s table name is %s!\n", __FUNCTION__, c.getString(0).c_str());
            return c.getString(0);
        } else {
            SYS_LOGE("%s %s don't have this table!\n", __FUNCTION__, GeneralTableName);
            return String8("");
        }
    } else {
        SYS_LOGE("%s: select action error!\n", __FUNCTION__);
        return String8("");
    }
}

int CPQExtdb::getRegValuesByValue(const char *name, const char *f_name, const char *f2_name,
                                 const int val, const int val2, am_regs_t *regs)
{
    CSqlite::Cursor c_reg_list;
    char sqlmaster[256] = {0};
    int rval = -1;

    if ((strlen(f2_name) == 0) && (val2 == 0)) {
        getSqlParams(__FUNCTION__, sqlmaster,
                     "select RegType, RegAddr, RegMask, RegValue from %s where %s = %d;", name, f_name,
                     val);
    } else {
        getSqlParams(__FUNCTION__, sqlmaster,
                     "select RegType, RegAddr, RegMask, RegValue from %s where %s = %d and %s = %d;",
                     name, f_name, val, f2_name, val2);
    }

    rval = this->select(sqlmaster, c_reg_list);
    int count = c_reg_list.getCount();
    if (count < 0 || rval < 0) {
        SYS_LOGE("%s, Select value error!\n", __FUNCTION__);
        regs->length = 0;
        return -1;
    } else if (count > REGS_MAX_NUMBER) {
        SYS_LOGE("%s, regs is too more, in pq.db count = %d", __FUNCTION__, count);
        regs->length = 0;
        return -1;
    }

    int index_am_reg = 0;
    if (c_reg_list.moveToFirst()) { //reg list for each table
        int index_type = 0;
        int index_addr = 1;
        int index_mask = 2;
        int index_val = 3;
        do {
            regs->am_reg[index_am_reg].type = c_reg_list.getUInt(index_type);
            regs->am_reg[index_am_reg].addr = c_reg_list.getUInt(index_addr);
            regs->am_reg[index_am_reg].mask = c_reg_list.getUInt(index_mask);
            regs->am_reg[index_am_reg].val = c_reg_list.getUInt(index_val);
            index_am_reg++;
        } while (c_reg_list.moveToNext());
        regs->length = index_am_reg;
    } else {
        regs->length = 0;
        rval = -1;
    }

    SYS_LOGI("%s, length = %d", __FUNCTION__, regs->length);
    return rval;
}

int CPQExtdb::PQ_GetOsdSharpnessParams(int level, source_input_param_t source_input_param, am_regs_t *regs)
{
    int rval = -1;

    String8 TableName = GetTableName("GeneralOSDSharpnessTable", source_input_param);
    if ((TableName.c_str() != NULL) && (TableName.length() != 0) ) {
        rval = getRegValuesByValue(TableName.c_str(), LEVEL_NAME, "", level, 0, regs);
    } else {
        SYS_LOGE("%s GeneralOSDSharpnessTable don't have table!!\n", __FUNCTION__);
    }

    return rval;
}
