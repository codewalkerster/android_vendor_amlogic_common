/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "SystemControl"
#define LOG_TV_TAG "SSMAction"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cutils/properties.h>
#include "SSMAction.h"
#include "CPQLog.h"


pthread_mutex_t ssm_r_w_op_mutex = PTHREAD_MUTEX_INITIALIZER;

SSMAction *SSMAction::mInstance;
SSMHandler *SSMAction::mSSMHandler = NULL;
SSMAction *SSMAction::getInstance()
{
    if (NULL == mInstance)
        mInstance = new SSMAction();
    return mInstance;
}

SSMAction::SSMAction()
{
    m_dev_fd = -1;
    mpObserver = NULL;
}

SSMAction::~SSMAction()
{
    if (m_dev_fd > 0) {
        close(m_dev_fd);
        m_dev_fd = -1;
    }

    if (mSSMHandler != NULL) {
        delete mSSMHandler;
        mSSMHandler = NULL;
    }
}

void SSMAction::init(const char *SsmDataPath, const char *SsmDataHandlerPath, const char *WhiteBalanceFilePath)
{
    //copy wb file path
    if (strlen(WhiteBalanceFilePath) < sizeof(mWhiteBalanceFilePath)/sizeof(char)) {
        strcpy(mWhiteBalanceFilePath, WhiteBalanceFilePath);
    }

    bool FileExist;
    //check ssm file exist!
    FileExist = isFileExist(SsmDataPath);

    //open ssm file!
    if (!FileExist) {
        SYS_LOGD("%s, %s don't exist,create and open!\n", __FUNCTION__, SsmDataPath);
        m_dev_fd = open(SsmDataPath, O_RDWR | O_SYNC | O_CREAT, S_IRUSR | S_IWUSR);
    } else {
        SYS_LOGD("open %s file!\n",SsmDataPath);
        m_dev_fd = open(SsmDataPath, O_RDWR | O_SYNC | O_CREAT, S_IRUSR | S_IWUSR);
    }

    if (m_dev_fd < 0) {
        SYS_LOGE("%s, Open %s failed! error: %s.\n", __FUNCTION__, SsmDataPath, strerror(errno));
    } else {
        SYS_LOGD("%s, Open %s success!", __FUNCTION__, SsmDataPath);
    }

    //open SSM handler!
    mSSMHandler = SSMHandler::GetSingletonInstance(SsmDataHandlerPath);

    //ssm check
    if (mSSMHandler != NULL) {
        SSM_status_t SSM_status = (SSM_status_t)GetSSMStatus();
        SYS_LOGD ("%s, Verify SSMHeader, status= %d\n", __FUNCTION__, SSM_status);
        if (DeviceMarkCheck() < 0 || SSM_status == SSM_HEADER_INVALID) {
            if (mpObserver != NULL) {
                mpObserver->resetAllUserSettingParam();
                mpObserver->resetSSMData();
                if (mSSMHandler->SSMRecreateHeader())
                    SYS_LOGD ("%s, SSMRecreateHeader success\n", __FUNCTION__);
                RestoreDeviceMarkValues();
            } else {
                SYS_LOGE ("%s: SSMActionObserver is NULL!\n", __FUNCTION__);
            }
        } else if (SSM_status == SSM_HEADER_STRUCT_CHANGE) {
            if (mpObserver != NULL) {
                mpObserver->resetAllUserSettingParam();
                mpObserver->resetSSMData();
                RestoreDeviceMarkValues();
            } else {
                SYS_LOGE ("%s: SSMActionObserver is NULL!\n", __FUNCTION__);
            }
        }
    }
}

bool SSMAction::isFileExist(const char *file_name)
{
    struct stat tmp_st;
    int ret = -1;

    ret = stat(file_name, &tmp_st);
    if (ret != 0 ) {
       SYS_LOGE("%s, %s don't exist!\n", __FUNCTION__, file_name);
       return false;
    } else {
       return true;
    }
}

//Mark r/w values
#define CC_DEF_CHARACTER_CHAR_VAL                   (0x8A)
static const int SSM_MARK_01_VALUE = 0x90;
static const int SSM_MARK_02_VALUE = 0xCE;
static const int SSM_MARK_03_VALUE = 0xDF;

int SSMAction::SaveBurnWriteCharacterChar(int rw_val)
{
    int value = rw_val;
    return SSMWriteNTypes(SSM_RSV_W_CHARACTER_CHAR_START, 1, &value, 0);
}

int SSMAction::ReadBurnWriteCharacterChar()
{
    int tmp_val = 0;

    if (SSMReadNTypes(SSM_RSV_W_CHARACTER_CHAR_START, 1, &tmp_val, 0) < 0) {
        return -1;
    }

    return tmp_val;
}

int SSMAction::DeviceMarkCheck()
{
    int i = 0, failed_count = 0;
    int mark_offset[3] = { 0, 0, 0 };
    unsigned char mark_values[3] = { 0, 0, 0 };
    int tmp_ch = 0;

    //read temp one byte
    SSMReadNTypes(0, 1, &tmp_ch, 0);

    mark_offset[0] = SSM_MARK_01_START;
    mark_offset[1] = SSM_MARK_02_START;
    mark_offset[2] = SSM_MARK_03_START;

    mark_values[0] = SSM_MARK_01_VALUE;
    mark_values[1] = SSM_MARK_02_VALUE;
    mark_values[2] = SSM_MARK_03_VALUE;

    if (ReadBurnWriteCharacterChar() != CC_DEF_CHARACTER_CHAR_VAL) {
        SaveBurnWriteCharacterChar(CC_DEF_CHARACTER_CHAR_VAL);
    }

    failed_count = 0;
    for (i = 0; i < 3; i++) {
        tmp_ch = 0;
        if (SSMReadNTypes(mark_offset[i], 1, &tmp_ch, 0) < 0) {
            SYS_LOGE("%s, DeviceMarkCheck Read Mark failed!!!\n", __FUNCTION__);
            break;
        }

        if ((unsigned char)tmp_ch != mark_values[i]) {
            failed_count += 1;
            SYS_LOGE(
                "%s, DeviceMarkCheck Mark[%d]'s offset = %d, Mark[%d]'s Value = %d, read value = %d.\n",
                __FUNCTION__, i, mark_offset[i], i, mark_values[i], tmp_ch);
        }
    }

    if (failed_count >= 3) {
        return -1;
    }

    return 0;
}

int SSMAction::RestoreDeviceMarkValues()
{
    int i;
    int mark_offset[3] = {
        (int) SSM_MARK_01_START,
        (int) SSM_MARK_02_START,
        (int) SSM_MARK_03_START,
    };

    int mark_values[3] = {SSM_MARK_01_VALUE, SSM_MARK_02_VALUE, SSM_MARK_03_VALUE};

    for (i = 0; i < 3; i++) {
        if (SSMWriteNTypes(mark_offset[i], 1, &mark_values[i], 0) < 0) {
            SYS_LOGD("SSMRestoreDeviceMarkValues Write Mark failed.\n");
            break;
        }
    }

    if (i < 3) {
        return -1;
    }

    return 0;
}

int SSMAction::WriteBytes(int offset, int size, int *buf)
{
    if (m_dev_fd < 0) {
        SYS_LOGE("%s m_dev_fd is negative.\n", __FUNCTION__);
        return -1;
    }

    if (lseek(m_dev_fd, offset, SEEK_SET) == -1) {
        SYS_LOGE("%s lseek failed\n", __FUNCTION__);
        return -1;
    }

    if (write(m_dev_fd, buf, size) == -1) {
        SYS_LOGE("%s write failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}
int SSMAction::ReadBytes(int offset, int size, int *buf)
{
    if (m_dev_fd < 0) {
        SYS_LOGE("%s m_dev_fd is negative.\n", __FUNCTION__);
        return -1;
    }

    if (lseek(m_dev_fd, offset, SEEK_SET) == -1) {
        SYS_LOGE("%s lseek failed\n", __FUNCTION__);
        return -1;
    }

    if (read(m_dev_fd, buf, size) == -1) {
        SYS_LOGE("%s read failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}
int SSMAction::EraseAllData()
{
    if (m_dev_fd < 0) {
        SYS_LOGE("%s m_dev_fd is negative.\n", __FUNCTION__);
        return -1;
    }

    if (ftruncate(m_dev_fd, 0) < 0)
        SYS_LOGE("%s ftruncate failed\n", __FUNCTION__);
    lseek(m_dev_fd, 0, SEEK_SET);

    return 0;
}

int SSMAction::GetSSMActualAddr(int id)
{
    return mSSMHandler->SSMGetActualAddr(id);
}

int SSMAction::GetSSMActualSize(int id)
{
    return mSSMHandler->SSMGetActualSize(id);
}

int SSMAction::GetSSMStatus(void)
{
    return (int)mSSMHandler->SSMVerify();
}

int SSMAction::SSMWriteNTypes(int id, int data_len, int *data_buf, int offset)
{
    //SYS_LOGD("%s: id = %d, len = %d, offset = %d\n", __FUNCTION__, id, data_len, offset);
    pthread_mutex_lock(&ssm_r_w_op_mutex);
    if (data_buf == NULL) {
        SYS_LOGE("data_buf is NULL.\n");
        pthread_mutex_unlock(&ssm_r_w_op_mutex);
        return -1;
    }

    if (0 == data_len) {
        pthread_mutex_unlock(&ssm_r_w_op_mutex);
        return -1;
    }

    unsigned int actualAddr = mSSMHandler->SSMGetActualAddr(id) + offset;
    //SYS_LOGD("%s: actualAddr = %u, data = %d.\n", __FUNCTION__, actualAddr, *data_buf);
    if (WriteBytes(actualAddr, data_len, data_buf) < 0) {
        SYS_LOGE("device WriteNBytes error.\n");
        pthread_mutex_unlock(&ssm_r_w_op_mutex);
        return -1;
    }
    pthread_mutex_unlock(&ssm_r_w_op_mutex);
    return 0;
}

int SSMAction::SSMReadNTypes(int id, int data_len, int *data_buf, int offset)
{
    //SYS_LOGD("%s: id = %d, len = %d, offset = %d\n", __FUNCTION__, id, data_len, offset);
    pthread_mutex_lock(&ssm_r_w_op_mutex);
    if (data_buf == NULL) {
        SYS_LOGE("data_buf is NULL.\n");
        pthread_mutex_unlock(&ssm_r_w_op_mutex);
        return -1;
    }

    if (0 == data_len) {
        pthread_mutex_unlock(&ssm_r_w_op_mutex);
        return -1;
    }

    unsigned int actualAddr = mSSMHandler->SSMGetActualAddr(id) + offset;
    if (ReadBytes(actualAddr, data_len, data_buf) < 0) {
        SYS_LOGE("device ReadNBytes error.\n");
        pthread_mutex_unlock(&ssm_r_w_op_mutex);
        return -1;
    }
    pthread_mutex_unlock(&ssm_r_w_op_mutex);

    //SYS_LOGD("%s: actualAddr = %u, data = %d.\n", __FUNCTION__, actualAddr, *data_buf);
    return 0;
}

bool SSMAction::SSMRecovery(void)
{
    bool ret = true;
    SYS_LOGD("%s start\n", __FUNCTION__);

    EraseAllData();

    if (mpObserver != NULL) {
        mpObserver->resetAllUserSettingParam();
        mpObserver->resetSSMData();
        RestoreDeviceMarkValues();
    } else {
        SYS_LOGE ("%s: SSMActionObserver is NULL!\n", __FUNCTION__);
    }

    ret = mSSMHandler->SSMRecreateHeader();
    SYS_LOGD("%s end\n", __FUNCTION__);

    return ret;
}

int SSMAction::SSMSaveColorDemoMode(unsigned char rw_val)
{
    int tmp_val = rw_val;
    return SSMWriteNTypes(VPP_DATA_POS_COLOR_DEMO_MODE_START, 1, &tmp_val);
}

int SSMAction::SSMReadColorDemoMode(unsigned char *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_COLOR_DEMO_MODE_START, 1, &tmp_val);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSavePQModuleDemoState(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_PQMODULE_DEMO_STATE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadPQModuleDemoState(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_PQMODULE_DEMO_STATE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveDDRSSC(unsigned char rw_val)
{
    int tmp_val = rw_val;
    return SSMWriteNTypes(VPP_DATA_POS_DDR_SSC_START, 1, &tmp_val);
}

int SSMAction::SSMReadDDRSSC(unsigned char *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_DDR_SSC_START, 1, &tmp_val);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveLVDSSSC(int *rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_LVDS_SSC_START, 3, rw_val);
}

int SSMAction::SSMReadLVDSSSC(int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_LVDS_SSC_START, 3, &tmp_val);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveAutoAspect(int offset, int rw_val)
{
    return SSMWriteNTypes(CUSTOMER_DATA_POS_AUTO_ASPECT, 1, &rw_val, offset);
}

int SSMAction::SSMReadAutoAspect(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(CUSTOMER_DATA_POS_AUTO_ASPECT, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSave43Stretch(int offset, int rw_val)
{
    return SSMWriteNTypes(CUSTOMER_DATA_POS_43_STRETCH, 1, &rw_val, offset);
}

int SSMAction::SSMRead43Stretch(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(CUSTOMER_DATA_POS_43_STRETCH, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMEdidRestoreDefault(int rw_val)
{
    int ret = 0;
    ret |= SSMWriteNTypes(CUSTOMER_DATA_POS_HDMI1_EDID_START, 1, &rw_val);
    ret |= SSMWriteNTypes(CUSTOMER_DATA_POS_HDMI2_EDID_START, 1, &rw_val);
    ret |= SSMWriteNTypes(CUSTOMER_DATA_POS_HDMI3_EDID_START, 1, &rw_val);
    ret |= SSMWriteNTypes(CUSTOMER_DATA_POS_HDMI4_EDID_START, 1, &rw_val);
    return ret;
}

int SSMAction::SSMHdcpSwitcherRestoreDefault(int rw_val)
{
    return SSMWriteNTypes(CUSTOMER_DATA_POS_HDMI_HDCP_SWITCHER_START, 1, &rw_val);
}

int SSMAction::SSMSColorRangeModeRestoreDefault(int rw_val)
{
    return SSMWriteNTypes(CUSTOMER_DATA_POS_HDMI_COLOR_RANGE_START, 1, &rw_val);
}

int SSMAction::SSMReadDLGEnable(int *rw_val)
{
    int tmp_ret = 0;
    int ret = 0;

    ret = SSMReadNTypes(VPP_DATA_POS_DLG_ENABLE_START, 1, &tmp_ret);
    *rw_val = tmp_ret;

    return ret;
}

int SSMAction::SSMSaveDLGEnable(int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_DLG_ENABLE_START, 1, &rw_val);
}

int SSMAction::ReadDataFromFile(const char *file_name, int offset, int nsize, unsigned char data_buf[])
{
    int device_fd = -1;

    if (data_buf == NULL) {
        SYS_LOGE("data_buf is NULL!!!\n");
        return -1;
    }

    if (file_name == NULL) {
        SYS_LOGE("%s is NULL!\n",file_name);
        return -1;
    }

    //device_fd = open(file_name, O_RDONLY);
    device_fd = open(file_name, O_RDWR | O_SYNC | O_CREAT, S_IRUSR | S_IWUSR);
    if (device_fd < 0) {
        SYS_LOGE("open file \"%s\" error(%s).\n", file_name, strerror(errno));
        return -1;
    }

    if (lseek(device_fd, offset, SEEK_SET) == -1) {
        SYS_LOGE("%s lseek failed\n", __FUNCTION__);
        close(device_fd);
        return -1;
    }

    if (read(device_fd, data_buf, nsize) == -1) {
        SYS_LOGE("%s read failed\n", __FUNCTION__);
        close(device_fd);
        return -1;
    }

    close(device_fd);
    device_fd = -1;

    return 0;
}

int SSMAction::SaveDataToFile(const char *file_name, int offset, int nsize, unsigned char data_buf[])
{
    int device_fd = -1;

    if (data_buf == NULL) {
        SYS_LOGE("%s, data_buf is NULL!!!\n", __FUNCTION__);
        return -1;
    }

    if (file_name == NULL) {
        SYS_LOGE("%s IS NULL!\n",file_name);
        return -1;
    }

    device_fd = open(file_name, O_RDWR | O_SYNC);
    if (device_fd < 0) {
        SYS_LOGE("open file \"%s\" error(%s).\n", file_name, strerror(errno));
        return -1;
    }

    if (lseek(device_fd, offset, SEEK_SET) == -1) {
        SYS_LOGE("%s lseek failed\n", __FUNCTION__);
        close(device_fd);
        return -1;
    }

    if (write(device_fd, data_buf, nsize) == -1) {
        SYS_LOGE("%s write failed\n", __FUNCTION__);
        close(device_fd);
        return -1;
    }
    fsync(device_fd);

    close(device_fd);
    device_fd = -1;

    return 0;
}
