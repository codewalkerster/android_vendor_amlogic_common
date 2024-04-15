/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "SystemControl"
#define LOG_TV_TAG "CSqlite"

#include "CSqlite.h"
#include <log/log.h>
#include "CPQLog.h"

CSqlite::CSqlite()
{
    mHandle = NULL;
}

CSqlite::~CSqlite()
{
    AutoMutex _l( mLock );
    if (mHandle != NULL) {
        sqlite3_close(mHandle);
        mHandle = NULL;
    }
}

int CSqlite::sqlite3_exec_callback(void *data __unused, int nColumn, char **colValues __unused, char **colNames __unused)
{
    SYS_LOGD("sqlite3_exec_callback, nums = %d", nColumn);
    return 0;
}

int CSqlite::openDb(const char *path)
{
    AutoMutex _l( mLock );
    if (sqlite3_open(path, &mHandle) != SQLITE_OK) {
        SYS_LOGD("open db(%s) error", path);
        mHandle = NULL;
        return -1;
    }
    return 0;
}

int CSqlite::closeDb()
{
    AutoMutex _l( mLock );
    int rval = 0;
    if (mHandle != NULL) {
        rval = sqlite3_close(mHandle);
        mHandle = NULL;
    }
    return rval;
}

void CSqlite::setHandle(sqlite3 *h)
{
    mHandle = h;
}

sqlite3 *CSqlite::getHandle()
{
    return mHandle;
}

int CSqlite::select(const char *sql, CSqlite::Cursor &c)
{
    AutoMutex _l( mLock );
    int col, row;
    char **pResult = NULL;
    char *errmsg;
    assert(mHandle && sql);

    if (strncmp(sql, "select", 6))
        return -1;
    if (sqlite3_get_table(mHandle, sql, &pResult, &row, &col, &errmsg) != SQLITE_OK) {
        SYS_LOGE("errmsg=%s", errmsg);
        if (pResult != NULL)
            sqlite3_free_table(pResult);
        return -1;
    }

    c.Init(pResult, row, col);
    return 0;
}

void CSqlite::insert()
{
}

bool CSqlite::exeSql(const char *sql)
{
    AutoMutex _l( mLock );
    char *errmsg;
    if (sql == NULL) return false;
    if (sqlite3_exec(mHandle, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        SYS_LOGE("exeSql=: %s error=%s", sql, errmsg ? errmsg : "Unknown");
        if (errmsg)
            sqlite3_free(errmsg);
        return false;
    }
    return true;
}

bool CSqlite::beginTransaction()
{
    return exeSql("begin;");
}

bool CSqlite::commitTransaction()
{
    return exeSql("commit;");
}

bool CSqlite::rollbackTransaction()
{
    return exeSql("rollback;");
}

void CSqlite::del()
{
}

void CSqlite::update()
{
}

void CSqlite::xxtable()
{
}

bool CSqlite::Sync(void)
{
    AutoMutex _l( mSyncLock );

    if (!mHandle) return false;

    char *errmsg;
    char sqlSync[] = "COMMIT TRANSACTION";

    sqlite3_reset(m_pStmtSelect);
    sqlite3_reset(m_pStmtUpdate);

    if (sqlite3_exec(mHandle, sqlSync, NULL, NULL, &errmsg) != SQLITE_OK) {
        //SYS_LOGE("exeSql=: %s error = %s", sqlSync, errmsg ? errmsg : "NULL");
        if (errmsg)
            sqlite3_free(errmsg);
        return false;
    }

    return true;
}

bool CSqlite::PrepareSqlSelect(const char *msql)
{
    AutoMutex _l( mLock );

    int ret = sqlite3_prepare_v2(mHandle, msql, -1, &m_pStmtSelect, 0);

    if (ret != SQLITE_OK || !m_pStmtSelect) {
        SYS_LOGE("ret: %d, msql =%s, errorCode=%d\n", ret, msql, sqlite3_errcode(mHandle));
        sqlite3_finalize(m_pStmtSelect);
        m_pStmtSelect = 0;
        assert(0);
    }

    return true;
}

bool CSqlite::PrepareSqlUpdate(const char *msql)
{
    AutoMutex _l( mLock );

    int ret = sqlite3_prepare_v2(mHandle, msql, -1, &m_pStmtUpdate, 0);

    if (ret != SQLITE_OK || !m_pStmtUpdate) {
        SYS_LOGE("ret: %d, msql =%s, errorCode=%d\n", ret, msql, sqlite3_errcode(mHandle));
        sqlite3_finalize(m_pStmtUpdate);
        m_pStmtUpdate = 0;
        assert(0);
    }

    return true;
}

bool CSqlite::SetIntegerValueToTable(const char *Key, int Data)
{
    AutoMutex _l( mLock );

    if (Key == NULL) {
        SYS_LOGE("INVALID VALUES");
        return false;
    }

    sqlite3_reset(m_pStmtUpdate);
    sqlite3_bind_text(m_pStmtUpdate, 1, Key, sizeof(char) * strlen(Key), SQLITE_TRANSIENT);
    sqlite3_bind_int(m_pStmtUpdate, 2, Data);

    int sql_ret = sqlite3_step(m_pStmtUpdate);

    if (sql_ret == SQLITE_DONE || sql_ret == SQLITE_ROW) {
        Sync();
        return true;
    } else {
        SYS_LOGE("sqlite3_step Key Table :%s m_pStmtUpdate fail", Key);
        return false;
    }

    return true;
}

bool CSqlite::GetIntegerValueFromTable(const char *Key, int *pData)
{
    AutoMutex _l( mLock );

    if (Key == NULL || pData == NULL) {
        SYS_LOGE("INVALID VALUES");
        return false;
    }

    sqlite3_reset(m_pStmtSelect);
    sqlite3_bind_text(m_pStmtSelect, 1, Key, sizeof(char) * strlen(Key), SQLITE_TRANSIENT);

    int sql_ret = sqlite3_step(m_pStmtSelect);

    if (sql_ret != SQLITE_ROW && sql_ret != SQLITE_DONE) {
        //SYS_LOGE("sqlite3_step m_pStmtSelect : %s fail\n", Key);
        return false;
    }

    int valSize = sqlite3_column_bytes(m_pStmtSelect, 0);
    if (valSize == 0) {
        //SYS_LOGE("sqlite3_column_bytes have no data");
        return false;
    }

    *pData = sqlite3_column_int(m_pStmtSelect, 0);

    return true;
}

bool CSqlite::SetRealValueToTable(const char *Key, double Data)
{
    AutoMutex _l( mLock );

    if (Key == NULL) {
        SYS_LOGE("INVALID VALUES");
        return false;
    }

    sqlite3_reset(m_pStmtUpdate);
    sqlite3_bind_text(m_pStmtUpdate, 1, Key, sizeof(char) * strlen(Key), SQLITE_TRANSIENT);
    sqlite3_bind_double(m_pStmtUpdate, 2, Data);

    int sql_ret = sqlite3_step(m_pStmtUpdate);

    if (sql_ret == SQLITE_DONE || sql_ret == SQLITE_ROW) {
        Sync();
        return true;
    } else {
        SYS_LOGE("sqlite3_step Key Table :%s m_pStmtUpdate fail", Key);
        return false;
    }

    return true;
}

bool CSqlite::GetRealValueFromTable(const char *Key, double *pData)
{
    AutoMutex _l( mLock );

    if (Key == NULL || pData == NULL) {
        SYS_LOGE("INVALID VALUES");
        return false;
    }

    sqlite3_reset(m_pStmtSelect);
    sqlite3_bind_text(m_pStmtSelect, 1, Key, sizeof(char) * strlen(Key), SQLITE_TRANSIENT);

    int sql_ret = sqlite3_step(m_pStmtSelect);

    if (sql_ret != SQLITE_ROW && sql_ret != SQLITE_DONE) {
        //SYS_LOGE("sqlite3_step m_pStmtSelect : %s fail\n", Key);
        return false;
    }

    int valSize = sqlite3_column_bytes(m_pStmtSelect, 0);
    if (valSize == 0) {
        //SYS_LOGE("sqlite3_column_bytes have no data");
        return false;
    }

    *pData = sqlite3_column_double(m_pStmtSelect, 0);

    return true;
}

bool CSqlite::SetTextValueToTable(const char *Key, char *pData)
{
    AutoMutex _l( mLock );

    if (Key == NULL || pData == NULL) {
        SYS_LOGE("INVALID VALUES");
        return false;
    }

    sqlite3_reset(m_pStmtUpdate);
    sqlite3_bind_text(m_pStmtUpdate, 1, Key, sizeof(char) * strlen(Key), SQLITE_TRANSIENT);
    sqlite3_bind_text(m_pStmtUpdate, 2, pData, sizeof(char) * strlen(pData), SQLITE_TRANSIENT);

    int sql_ret = sqlite3_step(m_pStmtUpdate);

    if (sql_ret == SQLITE_DONE || sql_ret == SQLITE_ROW) {
        Sync();
        return true;
    } else {
        SYS_LOGE("sqlite3_step Key Table :%s m_pStmtUpdate fail", Key);
        return false;
    }

    return true;
}

bool CSqlite::GetTextValueFromTable(const char *Key, char *pData)
{
    AutoMutex _l( mLock );

    if (Key == NULL || pData == NULL) {
        SYS_LOGE("INVALID VALUES");
        return false;
    }

    sqlite3_reset(m_pStmtSelect);
    sqlite3_bind_text(m_pStmtSelect, 1, Key, sizeof(char) * strlen(Key), SQLITE_TRANSIENT);

    int sql_ret = sqlite3_step(m_pStmtSelect);

    if (sql_ret != SQLITE_ROW && sql_ret != SQLITE_DONE) {
        SYS_LOGE("sqlite3_step m_pStmtSelect : %s fail\n", Key);
        return false;
    }

    int valSize = sqlite3_column_bytes(m_pStmtSelect, 0);
    if (valSize == 0) {
        //SYS_LOGE("sqlite3_column_bytes have no data");
        return false;
    }

    void *str = NULL;
    str = (char *)sqlite3_column_text(m_pStmtSelect, 0);

    memcpy(pData, str, valSize);

    return true;
}

bool CSqlite::SetBlobValueToTable(const char *Key, void *pData, int iSize)
{
    AutoMutex _l( mLock );

    if (Key == NULL || pData == NULL) {
        SYS_LOGE("INVALID VALUES");
        return false;
    }

    sqlite3_reset(m_pStmtUpdate);
    sqlite3_bind_text(m_pStmtUpdate, 1, Key, sizeof(char) * strlen(Key), SQLITE_TRANSIENT);
    sqlite3_bind_blob(m_pStmtUpdate, 2, pData, iSize, SQLITE_TRANSIENT);

    int sql_ret = sqlite3_step(m_pStmtUpdate);

    if (sql_ret == SQLITE_DONE || sql_ret == SQLITE_ROW) {
        Sync();
        return true;
    } else {
        SYS_LOGE("sqlite3_step Key Table :%s m_pStmtUpdate fail", Key);
        return false;
    }

    return true;
}

bool CSqlite::GetBlobValueFromTable(const char *Key, void *pData, int iSize)
{
    AutoMutex _l( mLock );

    if (Key == NULL || pData == NULL) {
        SYS_LOGE("INVALID VALUES");
        return false;
    }

    sqlite3_reset(m_pStmtSelect);
    sqlite3_bind_text(m_pStmtSelect, 1, Key, sizeof(char) * strlen(Key), SQLITE_TRANSIENT);

    int sql_ret = sqlite3_step(m_pStmtSelect);

    if (sql_ret != SQLITE_ROW && sql_ret != SQLITE_DONE) {
        //SYS_LOGE("sqlite3_step Key Table :%s fail\n", Key);
        return false;
    }

    int valSize = sqlite3_column_bytes(m_pStmtSelect, 0);
    if (valSize == 0) {
        //SYS_LOGE("sqlite3_column_bytes have no data");
        return false;
    }

    void *str = NULL;
    str = (void *)sqlite3_column_blob(m_pStmtSelect, 0);

    if (valSize != iSize) {
        SYS_LOGE("valSize[%d] != iSize[%d], data is not math", valSize, iSize);
        return false;
    }

    memcpy(pData, str, iSize);

    return true;
}

