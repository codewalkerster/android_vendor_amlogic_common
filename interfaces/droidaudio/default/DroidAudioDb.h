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

#pragma once
#include <vector>
#include <thread>
#include <unordered_map>

#include <sqlite3.h>
#include "inicpp/inicpp.hpp"

using namespace std;


class DroidAudioDbDescriptor;

class DroidAudioDb {
public:
    static DroidAudioDb* instance();
    int32_t init();
    int32_t sqliteExecSql(const string& sql);
    int32_t addDbIdStr(string moduleId, DroidAudioDbDescriptor* pModule);

    sqlite3* getSql3Manager() {
        return mpDbHandle;
    }
    inicpp::IniManager& getIniManager() {
        return mIniFile;
    }
    bool firstBoot() {
        return mbFirstBoot;
    }

private:
    DroidAudioDb();
    virtual ~DroidAudioDb();

    sqlite3*                                            mpDbHandle;
    map<string, DroidAudioDbDescriptor*>                mMapDbInfo;
    inicpp::IniManager                                  mIniFile;
    bool                                                mbFirstBoot = false;
};

inline DroidAudioDb* DroidAudioDb::instance() {
    static DroidAudioDb instance;
    return &instance;
}

class DroidAudioDbDescriptor {
friend class DroidAudioDb;

public:
    // define module id
    inline static const std::string DROIDLOGIC_DB_MODULE_ID_AM_AUDIO_COMMON                = "AM_AUDIO_COMMON";
    inline static const std::string DROIDLOGIC_DB_MODULE_ID_AM_AUDIO_MANAGER               = "AM_AUDIO_MANAGER";
    inline static const std::string DROIDLOGIC_DB_MODULE_ID_AM_AUDIO_EFFECT                = "AM_AUDIO_EFFECT";

    DroidAudioDbDescriptor(const string& id, const vector<const char*>& vecStr) :
        mVecDbStr(vecStr), mstrModuleId(id), mpDb(DroidAudioDb::instance()), mpDbHandle(mpDb->getSql3Manager()) {
        mpDb->addDbIdStr(mstrModuleId, this);
    }
    virtual ~DroidAudioDbDescriptor() {}

    /* The subclass must implement this function,
     * determine the data type(int, string, double...) according to the database key,
     * and return the correct data size
     */
    virtual vector<uint8_t> getDefaultValue(const string& key) = 0;

    int32_t putToDb(const string& key, int32_t value);
    int32_t putToDb(const string& key, const double value);
//    template <typename T>
//    int32_t putToDb(const std::string& key, const T& value);
    int32_t putToDb(const string& key, const string& value);
    int32_t getIntFromDb(const string& key);;
    double getDoubleFromDb(const string& key);;
    string getStrFromDb(const string& key);
    void resetDefaultValue();
    bool firstBoot() {
        return mpDb->firstBoot();
    }
    int32_t getIntFromIni(const string& key);
    double getDoubleFromIni(const string& key);
    string getStrFromIni(const string& key);

protected:
    const vector<const char*>                                 mVecDbStr;

private:
    int32_t sqliteInit();
    int32_t sqliteInsert(const string &key, const vector<uint8_t>& data);
    int32_t sqliteInserDefaultValueToDb(const string& key, vector<uint8_t>& retDefault);
    int32_t sqliteStmtPrepare(const string &action, sqlite3_stmt **pStmt);

    int32_t sqlitePutBlobtoDb(const string& key, const char* pData, uint32_t size);
    int32_t sqliteGetBlobFromDb(const string& key, vector<uint8_t>& vecData, uint32_t size);

    string                                              mstrModuleId;
    DroidAudioDb*                                       mpDb;
    sqlite3*                                            mpDbHandle;
    sqlite3_stmt*                                       mpStmtDbUpdate = nullptr;
    sqlite3_stmt*                                       mpStmtDbSelect = nullptr;
};

