/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef __SSM_ACTION_H__
#define __SSM_ACTION_H__

#include "SSMHandler.h"
#include "PQType.h"

#define DEFAULT_BACKLIGHT_BRIGHTNESS                (10)


class SSMAction {
public:
    SSMAction();
    ~SSMAction();
    void init(const char *SsmDataPath, const char *SsmDataHandlerPath, const char *WhiteBalanceFilePath);
    int SaveBurnWriteCharacterChar(int rw_val);
    int ReadBurnWriteCharacterChar();
    int DeviceMarkCheck();
    int RestoreDeviceMarkValues();
    static SSMAction *getInstance();
    bool isFileExist(const char *file_name);
    int WriteBytes(int offset, int size, int *buf);
    int ReadBytes(int offset, int size, int *buf);
    int EraseAllData(void);
    int GetSSMActualAddr(int id);
    int GetSSMActualSize(int id);
    int GetSSMStatus(void);
    int SSMReadNTypes(int id, int data_len, int *data_buf, int offset = 0);
    int SSMWriteNTypes(int id, int data_len, int *data_buf, int offset = 0);

    bool SSMRecovery();

    int SSMSaveColorDemoMode(unsigned char rw_val);
    int SSMReadColorDemoMode(unsigned char *rw_val);

    int SSMSavePQModuleDemoState(int offset, int rw_val);
    int SSMReadPQModuleDemoState(int offset, int *rw_val);

    int SSMSaveDDRSSC(unsigned char rw_val);
    int SSMReadDDRSSC(unsigned char *rw_val);
    int SSMSaveLVDSSSC(int *rw_val);
    int SSMReadLVDSSSC(int *rw_val);

    int SSMSaveAutoAspect(int offset, int rw_val);
    int SSMReadAutoAspect(int offset, int *rw_val);
    int SSMSave43Stretch(int offset, int rw_val);
    int SSMRead43Stretch(int offset, int *rw_val);
    int SSMEdidRestoreDefault(int rw_val);
    int SSMHdcpSwitcherRestoreDefault(int rw_val);
    int SSMSColorRangeModeRestoreDefault(int rw_val);
    int SSMReadDLGEnable(int *rw_val);
    int SSMSaveDLGEnable(int rw_val);

    int ReadDataFromFile(const char *file_name, int offset, int nsize, unsigned char data_buf[]);
    int SaveDataToFile(const char *file_name, int offset, int nsize, unsigned char data_buf[]);

    int m_dev_fd;
    static SSMAction *mInstance;
    static SSMHandler *mSSMHandler;
    class ISSMActionObserver {
        public:
            ISSMActionObserver() {};
            virtual ~ISSMActionObserver() {};
            virtual void resetAllUserSettingParam() {};
            virtual void resetSSMData() {};
    };

    void setObserver (ISSMActionObserver *pOb)
    {
        mpObserver = pOb;
    };
private:
    ISSMActionObserver *mpObserver;
    char mWhiteBalanceFilePath[128];
};
#endif
