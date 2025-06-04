/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "DroidAudioDb"
//#define LOG_NDEBUG 0

#include <shared_mutex>

#include <log/log.h>

#include "inicpp/inicpp.hpp"

#include "DroidAudioClientUtils.h"
#include "DroidAudioCommon.h"
#include "DroidAudioDb.h"


#define AML_DROIDLOGIC_DB_KEY_NAME                          "db_key"
#define AML_DROIDLOGIC_DB_VALUE_NAME                        "db_value"

#define AML_DROIDLOGIC_DB_FILE_NAME                         "/data/droidaudio/droidaudio.db"
#define AML_DROIDLOGIC_INI_FILE_NAME                        "/vendor/etc/audio_config/droidaudio.ini"

using namespace std;

static shared_mutex    g_sqliteLock;

DroidAudioDb::DroidAudioDb() : mIniFile(AML_DROIDLOGIC_INI_FILE_NAME){
    AM_LOGI("");
    const char *pDbFilePath = AML_DROIDLOGIC_DB_FILE_NAME;
    mbFirstBoot = (access(pDbFilePath, R_OK) != 0);
    if (sqlite3_open_v2(pDbFilePath, &mpDbHandle,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL) != 0) {
        AM_LOGE("open db(%s) error", pDbFilePath);
        mpDbHandle = nullptr;
        return;
    }
    sqliteExecSql("pragma synchronous = full;"); // full, normal, off
//    sqliteExecSql("pragma journal_mode = wal;");
    if (mbFirstBoot) {
        AM_LOGI("First boot, %s created successfully", pDbFilePath);
    } else {
        AM_LOGI("Normal boot %s", pDbFilePath);
    }
}

DroidAudioDb::~DroidAudioDb() {
    if (mpDbHandle) {
        sqlite3_close(mpDbHandle);
        mpDbHandle = nullptr;
    }
}

int32_t DroidAudioDb::sqliteExecSql(const string& sql) {
    char *errmsg;
    if (sqlite3_exec(mpDbHandle, sql.c_str(), NULL, NULL, &errmsg) != SQLITE_OK) {
        AM_LOGE("exeSql:%s error:[%s]", sql.c_str(), errmsg ? errmsg : "Unknown");
        if (errmsg) {
            sqlite3_free(errmsg);
        }
        return -1;
    }
    return 0;
}

int32_t DroidAudioDb::addDbIdStr(string moduleId, DroidAudioDbDescriptor* pModule) {
    R_CHECK_POINTER_LEGAL(-1, pModule,)
    auto it = mMapDbInfo.find(moduleId);
    if (it != mMapDbInfo.end()) {
        AM_LOGE("moduleId:%s already exists. add fail.", moduleId.c_str());
        return -1;
    }
    if (pModule->mVecDbStr.size() <= 0) {
        AM_LOGE("moduleId:%s mVecDbStr is null.", moduleId.c_str());
        return -1;
    }
    mMapDbInfo[moduleId] = pModule;
    return 0;
}

int32_t DroidAudioDb::init() {
    AM_LOGI("init start");
    int32_t ret = 0;
    for (const auto& pair : mMapDbInfo) {
        DroidAudioDbDescriptor* pModule = pair.second;
        ret |= pModule->sqliteInit();
    }
    if (ret == 0) {
        AM_LOGI("init db:%s success", AML_DROIDLOGIC_DB_FILE_NAME);
    }
    return ret;
}

int32_t DroidAudioDbDescriptor::sqliteInit() {
    unique_lock<shared_mutex> l(g_sqliteLock);
    if (firstBoot()) {
        char *errmsg;
        char sql[256];
        sprintf(sql, "create table %s(%s text primary key, %s blob);",
            mstrModuleId.c_str(), AML_DROIDLOGIC_DB_KEY_NAME, AML_DROIDLOGIC_DB_VALUE_NAME);
        if (sqlite3_exec(mpDbHandle, sql, 0, 0, &errmsg) != SQLITE_OK) {
            AM_LOGE("DB create %s table error:[%s]", mstrModuleId.c_str(), errmsg);
            sqlite3_free(errmsg);
            return -1;
        }
        for (const string& dbIdStr : mVecDbStr) {
            AM_LOGV("moduleId:%s, key: %s", mstrModuleId.c_str(), dbIdStr.c_str());
            sqliteInsert(dbIdStr, getDefaultValue(dbIdStr));
        }
    }
    sqliteStmtPrepare("select", &mpStmtDbSelect);
    sqliteStmtPrepare("replace", &mpStmtDbUpdate);
    AM_LOGI("module mpStmtDbUpdate:%p module:%s", mpStmtDbUpdate, mstrModuleId.c_str());
    return 0;
}

int32_t DroidAudioDbDescriptor::sqliteInserDefaultValueToDb(const string& key, vector<uint8_t>& retDefault) {
    auto it = std::find(mVecDbStr.begin(), mVecDbStr.end(), key);
    if (it == mVecDbStr.end()) {
        AM_LOGE("module:%s There is no key:%s in the db code, return 0.", mstrModuleId.c_str(), key.c_str());
        return -1;
    } else {
        if (mpDb->sqliteExecSql("begin transaction;") == 0) {
            vector<uint8_t> defaultValue = getDefaultValue(key);
            sqliteInsert(key, defaultValue);
            mpDb->sqliteExecSql("commit;");
            AM_LOGI("module:%s key:%s, return default and insert to Db.", mstrModuleId.c_str(), key.c_str());
            retDefault.clear();
            retDefault.resize(defaultValue.size());
            std::copy(defaultValue.begin(), defaultValue.end(), retDefault.begin());
            return 0;
        } else {
            AM_LOGE(" module:%s key:%s ret:%d, insert default fail.", mstrModuleId.c_str(), key.c_str());
            return -1;
        }
    }
}

int32_t DroidAudioDbDescriptor::sqliteInsert(const string& key, const vector<uint8_t>& data) {
    if (data.size() == 0) {
        AM_LOGE("module:%s, id:%s data size is 0.", mstrModuleId.c_str(), key.c_str());
        return -1;
    }

    sqlite3_stmt *pInsertStmt = nullptr;
    int32_t ret;

    AM_LOGV("sqliteInsert moduleId:%s key:%s", mstrModuleId.c_str(), key.c_str());
    ret = sqliteStmtPrepare("insert", &pInsertStmt);
    R_CHECK_RET(ret,);
    sqlite3_bind_text(pInsertStmt, 1, key.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_blob(pInsertStmt, 2, data.data(), data.size(), SQLITE_TRANSIENT);

    ret = sqlite3_step(pInsertStmt);
    if (ret != SQLITE_DONE) {
        AM_LOGE("sqlite3_step error:[%s], for id:%s", sqlite3_errmsg(mpDbHandle), key.c_str());
    }
    sqlite3_finalize(pInsertStmt);
    pInsertStmt = nullptr;
    return 0;
}

int32_t DroidAudioDbDescriptor::sqliteStmtPrepare(const string &action, sqlite3_stmt **pStmt) {
    R_CHECK_POINTER_LEGAL(-1, pStmt, "action:%s", action.c_str());
    char sql[256];
    if (action == "replace") {
        sprintf(sql, "replace into %s(%s, %s) values(?, ?);",
            mstrModuleId.c_str(), AML_DROIDLOGIC_DB_KEY_NAME, AML_DROIDLOGIC_DB_VALUE_NAME);
    } else if (action == "select") {
        sprintf(sql, "select %s from %s where %s=?;", AML_DROIDLOGIC_DB_VALUE_NAME, mstrModuleId.c_str(), AML_DROIDLOGIC_DB_KEY_NAME);
    } else if (action == "insert") {
        sprintf(sql, "insert into %s (%s, %s) values (?, ?);",
            mstrModuleId.c_str(), AML_DROIDLOGIC_DB_KEY_NAME, AML_DROIDLOGIC_DB_VALUE_NAME);
    } else {
        AM_LOGE("not supported action:%s", action.c_str());
        return -1;
    }

    int32_t ret = sqlite3_prepare_v2(mpDbHandle, sql, -1, pStmt, 0);
    if (ret != SQLITE_OK || !(*pStmt)) {
        AM_LOGE("ret:%d sql:%s error:[%s]", ret, sql, sqlite3_errmsg(mpDbHandle));
        sqlite3_finalize(*pStmt);
        *pStmt = nullptr;
        return -1;
    }
    AM_LOGV("success: %s", action.c_str());
    return 0;
}

int32_t DroidAudioDbDescriptor::sqlitePutBlobtoDb(const string& key, const char* pData, uint32_t size) {
    if (isDriverBaseProject()) {
        AM_LOGV("module:%s, id:%s, the driver base project, doesn't need to be saved to db", mstrModuleId.c_str(), key.c_str());
        return 0;
    }
    unique_lock<shared_mutex> l(g_sqliteLock);
    if (size == 0) {
        AM_LOGE("module:%s, id:%s size is 0.", mstrModuleId.c_str(), key.c_str());
        return -1;
    }
    R_CHECK_POINTER_LEGAL(-1, mpStmtDbUpdate, "module:%s, id:%s", mstrModuleId.c_str(), key.c_str());
    if (mpDb->sqliteExecSql("begin transaction;") != 0) {
        AM_LOGE("Failed to begin transaction");
        return -1;
    }
    sqlite3_reset(mpStmtDbUpdate);
    sqlite3_bind_text(mpStmtDbUpdate, 1, key.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_blob(mpStmtDbUpdate, 2, pData, size, SQLITE_TRANSIENT);
    int32_t ret = sqlite3_step(mpStmtDbUpdate);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW) {
        ret = mpDb->sqliteExecSql("commit;");
    } else {
        const char *errMsg = sqlite3_errmsg(mpDb->getSql3Manager());
        AM_LOGW("set Db fail. module:%s key:%s error:[%s]", mstrModuleId.c_str(), key.c_str(), errMsg);
        mpDb->sqliteExecSql("rollback;");
        return -1;
    }
    AM_LOGV("module:%s key:%s end", mstrModuleId.c_str(), key.c_str());
    return 0;
}

int32_t DroidAudioDbDescriptor::sqliteGetBlobFromDb(const string& key, vector<uint8_t>& vecData, uint32_t size) {
    unique_lock<shared_mutex> l(g_sqliteLock);
    R_CHECK_POINTER_LEGAL(-1, mpStmtDbSelect, "module:%s, key:%s", mstrModuleId.c_str(), key.c_str());
    sqlite3_reset(mpStmtDbSelect);
    sqlite3_bind_text(mpStmtDbSelect, 1, key.c_str(), -1, SQLITE_STATIC);
    int32_t ret = sqlite3_step(mpStmtDbSelect);
    if (ret != SQLITE_DONE && ret != SQLITE_ROW) {
        const char *errMsg = sqlite3_errmsg(mpDb->getSql3Manager());
        AM_LOGW("get Db fail. module:%s key:%s error:[%s]", mstrModuleId.c_str(), key.c_str(), errMsg);
        mpDb->sqliteExecSql("rollback;");
        return -1;
    }

    uint32_t valSize = sqlite3_column_bytes(mpStmtDbSelect, 0);
    if (valSize == 0) {
        AM_LOGW("get Db fail. db size is 0, need size:%d. module:%s, key:%s.", size, mstrModuleId.c_str(), key.c_str());
        return sqliteInserDefaultValueToDb(key, vecData);
    } else {
        if (valSize != size) {
            AM_LOGW("get Db fail. module:%s, key:%s. db size:%d != need size:%d",
                mstrModuleId.c_str(), key.c_str(), valSize, size);
            return sqliteInserDefaultValueToDb(key, vecData);
        }
    }
    void *pValue = (void *)sqlite3_column_blob(mpStmtDbSelect, 0);
    R_CHECK_POINTER_LEGAL(-1, pValue, "sqlite3_column_blob fail")
    vecData.resize(valSize);
    memcpy(vecData.data(), pValue, valSize);
    AM_LOGV("module:%s, key:%s", mstrModuleId.c_str(), key.c_str());
    return 0;
}

int32_t DroidAudioDbDescriptor::putToDb(const string& key, int32_t value) {
    AM_LOGV("module:%s, key:%s = %d", mstrModuleId.c_str(), key.c_str(), value);
    return sqlitePutBlobtoDb(key, (char*)&value, sizeof(value));
}

int32_t DroidAudioDbDescriptor::putToDb(const string& key, const double value) {
    AM_LOGV("module:%s, key:%s = %f", mstrModuleId.c_str(), key.c_str(), value);
    return sqlitePutBlobtoDb(key, (char*)&value, sizeof(value));
}

int32_t DroidAudioDbDescriptor::putToDb(const string& key, const string& value) {
    AM_LOGV("module:%s, key:%s = %s", mstrModuleId.c_str(), key.c_str(), value.c_str());
    return sqlitePutBlobtoDb(key, value.c_str(), value.size());
}

int32_t DroidAudioDbDescriptor::getIntFromDb(const string& key) {
    vector<uint8_t> value;
    int32_t intValue = 0;
    int32_t ret = sqliteGetBlobFromDb(key, value, sizeof(intValue));
    if (ret != 0) {
        AM_LOGE("get BLOB fail. module:%s, key:%s", mstrModuleId.c_str(), key.c_str());
        return 0;
    }
    if (value.size() != sizeof(int32_t)) {
        AM_LOGE("get BLOB fail. db size:%zu != need size:%zu module:%s, key:%s",
            value.size(), sizeof(int32_t), mstrModuleId.c_str(), key.c_str());
        return 0;
    }
    memcpy(&intValue, value.data(), sizeof(int32_t));
    return intValue;
}

double DroidAudioDbDescriptor::getDoubleFromDb(const string& key) {
    vector<uint8_t> value;
    double doubleValue = 0;
    int32_t ret = sqliteGetBlobFromDb(key, value, sizeof(doubleValue));
    if (ret != 0) {
        AM_LOGE("get BLOB fail. module:%s, key:%s", mstrModuleId.c_str(), key.c_str());
        return 0.0;
    }
    if (value.size() != sizeof(double)) {
        AM_LOGE("get BLOB fail. db size:%zu != need size:%zu module:%s, key:%s",
            value.size(), sizeof(double), mstrModuleId.c_str(), key.c_str());
        return 0.0;
    }
    memcpy(&doubleValue, value.data(), sizeof(double));
    return doubleValue;
}

string DroidAudioDbDescriptor::getStrFromDb(const string& key) {
    vector<uint8_t> value;
    int32_t ret = sqliteGetBlobFromDb(key, value, 0/* string*/);
    if (ret != 0) {
        AM_LOGE("get BLOB fail. module:%s, key:%s", mstrModuleId.c_str(), key.c_str());
        return "";
    }
    string str(value.begin(), value.end());
    return str;
}

int32_t DroidAudioDbDescriptor::getIntFromIni(const string& key) {
    inicpp::IniManager& ini = mpDb->getIniManager();
    return ini[mstrModuleId].toInt(key);
}
double DroidAudioDbDescriptor::getDoubleFromIni(const string& key) {
    inicpp::IniManager& ini = mpDb->getIniManager();
    return ini[mstrModuleId].toDouble(key);
}
string DroidAudioDbDescriptor::getStrFromIni(const string& key) {
    inicpp::IniManager& ini = mpDb->getIniManager();
    return ini[mstrModuleId].toString(key);
}

void DroidAudioDbDescriptor::resetDefaultValue() {
    unique_lock<shared_mutex> l(g_sqliteLock);
    R_CHECK_POINTER_LEGAL(, mpStmtDbUpdate, "module:%s", mstrModuleId.c_str());
    if (mpDb->sqliteExecSql("begin transaction;") != 0) {
        AM_LOGE("Failed to begin transaction");
        return;
    }
    for (const char* dbStr : mVecDbStr) {
        vector<uint8_t> defaultValue = getDefaultValue(dbStr);
        if (defaultValue.size() == 0) {
            AM_LOGE("module:%s, id:%s size is 0.", mstrModuleId.c_str(), dbStr);
            continue;
        }
        sqlite3_reset(mpStmtDbUpdate);
        sqlite3_bind_text(mpStmtDbUpdate, 1, dbStr, -1, SQLITE_STATIC);
        sqlite3_bind_blob(mpStmtDbUpdate, 2, defaultValue.data(), defaultValue.size(), SQLITE_TRANSIENT);
        int32_t ret = sqlite3_step(mpStmtDbUpdate);
        if (ret == SQLITE_DONE || ret == SQLITE_ROW) {
        } else {
            AM_LOGE("sqlite3_step module:%s key:%s ret:%d", mstrModuleId.c_str(), dbStr, ret);
        }
    }
    mpDb->sqliteExecSql("commit;");
}


