#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "panel/adap_lcd.h"
#include "ioctrl/lcd_cmd_id.h"


static int mLcdFd = -1;
static int mLcdFdIsOpened = 0;


ADAP_STATUS_T ADAP_LCD_INIT(void)
{
    if (mLcdFdIsOpened) {
        LOGD("%s LcdFd has been opened.\n", __FUNCTION__);
        return ADAP_OK;
    }

    if (mLcdFd < 0) {
        mLcdFd = open("/dev/" LCD_DEVICE_NAME, O_RDWR);
        LOGD("%s mLcdFd = %d.\n", __FUNCTION__, mLcdFd);
    }

    mLcdFdIsOpened = 1;

    if (mLcdFd < 0) {
        LOGE("%s failed.\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    LOGD("%s mLcdFd=%d.\n", __FUNCTION__, mLcdFd);

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_Uninit(void)
{
    mLcdFd = -1;
    mLcdFdIsOpened = 0;
    printf("[%s][%d] mLcdFd = %d Fail\n",__func__,__LINE__, mLcdFd);

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_Open(void)
{
    return ADAP_NOT_SUPPORTED;
}

ADAP_STATUS_T ADAP_LCD_Close(void)
{
    return ADAP_NOT_SUPPORTED;
}

ADAP_STATUS_T ADAP_LCD_GetNrHdrInfo(lcd_optical_info_s *param)
{
    if (mLcdFd < 0) {
        LOGE("%s OPEN /dev/%s fail\n", __FUNCTION__,LCD_DEVICE_NAME);
        return ADAP_NOT_OK;
    }

    if (ioctl(mLcdFd, LCD_IOC_CMD_NR_GET_HDR_INFO, param) < 0) {
        LOGE("%s VBE ioctl LCD_IOC_CMD_NR_GET_HDR_INFO fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_SetNrHdrInfo(lcd_optical_info_s *param)
{
    if (mLcdFd < 0) {
        LOGE("%s OPEN /dev/%s fail\n", __FUNCTION__,LCD_DEVICE_NAME);
        return ADAP_NOT_OK;
    }

    if (ioctl(mLcdFd, LCD_IOC_CMD_NR_SET_HDR_INFO, param) < 0) {
        LOGE("%s VBE ioctl LCD_IOC_CMD_NR_SET_HDR_INFO fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_GetTconBinMaxCnt(UINT32 *cnt)
{
    if (mLcdFd < 0) {
        LOGE("%s OPEN /dev/%s fail\n", __FUNCTION__,LCD_DEVICE_NAME);
        return ADAP_NOT_OK;
    }

    if (ioctl(mLcdFd, LCD_IOC_CMD_GET_TCON_BIN_MAX_CNT_INFO, cnt) < 0) {
        LOGE("%s VBE ioctl LCD_IOC_CMD_NR_SET_HDR_INFO fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_SetTconDataIndex(UINT32 index)
{
    if (mLcdFd < 0) {
        LOGE("%s OPEN /dev/%s fail\n", __FUNCTION__,LCD_DEVICE_NAME);
        return ADAP_NOT_OK;
    }

    if (ioctl(mLcdFd, LCD_IOC_CMD_SET_TCON_DATA_INDEX_INFO, &index) < 0) {
        LOGE("%s VBE ioctl LCD_IOC_CMD_SET_TCON_DATA_INDEX_INFO fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_GetTconBinPath(aml_path_s *str)
{
    if (mLcdFd < 0) {
        LOGE("%s OPEN /dev/%s fail\n", __FUNCTION__,LCD_DEVICE_NAME);
        return ADAP_NOT_OK;
    }

    if (ioctl(mLcdFd, LCD_IOC_CMD_GET_TCON_BIN_PATH_INFO, str) < 0) {
        LOGE("%s VBE ioctl LCD_IOC_CMD_GET_TCON_BIN_PATH_INFO fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_SetTconBinData(am_pq_bin_param_s *param)
{
    if (mLcdFd < 0) {
        LOGE("%s OPEN /dev/%s fail\n", __FUNCTION__,LCD_DEVICE_NAME);
        return ADAP_NOT_OK;
    }

    if (ioctl(mLcdFd, LCD_IOC_CMD_SET_TCON_BIN_DATA_INFO, param) < 0) {
        LOGE("%s VBE ioctl LCD_IOC_CMD_SET_TCON_BIN_DATA_INFO fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_SetPowerCtrl(UINT32 state)
{
    if (mLcdFd < 0) {
        LOGE("%s OPEN /dev/%s fail\n", __FUNCTION__,LCD_DEVICE_NAME);
        return ADAP_NOT_OK;
    }

    if (ioctl(mLcdFd, LCD_IOC_CMD_POWER_CTRL, &state) < 0) {
        LOGE("%s VBE ioctl LCD_IOC_CMD_POWER_CTRL fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_SetMuteCtrl(UINT32 state)
{
    if (mLcdFd < 0) {
        LOGE("%s OPEN /dev/%s fail\n", __FUNCTION__,LCD_DEVICE_NAME);
        return ADAP_NOT_OK;
    }

    if (ioctl(mLcdFd, LCD_IOC_CMD_MUTE_CTRL, &state) < 0) {
        LOGE("%s VBE ioctl LCD_IOC_CMD_MUTE_CTRL fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_SetPhyParam(phy_config_s *param)
{
    if (mLcdFd < 0) {
        LOGE("%s OPEN /dev/%s fail\n", __FUNCTION__,LCD_DEVICE_NAME);
        return ADAP_NOT_OK;
    }

    if (ioctl(mLcdFd, LCD_IOC_CMD_SET_PHY_PARAM, param) < 0) {
        LOGE("%s VBE ioctl LCD_IOC_CMD_SET_PHY_PARAM fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_GetPhyParam(phy_config_s *param)
{
    if (mLcdFd < 0) {
        LOGE("%s OPEN /dev/%s fail\n", __FUNCTION__,LCD_DEVICE_NAME);
        return ADAP_NOT_OK;
    }

    if (ioctl(mLcdFd, LCD_IOC_CMD_GET_PHY_PARAM, param) < 0) {
        LOGE("%s VBE ioctl LCD_IOC_CMD_GET_PHY_PARAM fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_SetSS(aml_lcd_ss_ctl_s *param)
{
    if (mLcdFd < 0) {
        LOGE("%s OPEN /dev/%s fail\n", __FUNCTION__,LCD_DEVICE_NAME);
        return ADAP_NOT_OK;
    }

    if (ioctl(mLcdFd, LCD_IOC_CMD_SET_SS, param) < 0) {
        LOGE("%s VBE ioctl LCD_IOC_CMD_SET_SS fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LCD_GetSS(aml_lcd_ss_ctl_s *param)
{
    if (mLcdFd < 0) {
        LOGE("%s OPEN /dev/%s fail\n", __FUNCTION__,LCD_DEVICE_NAME);
        return ADAP_NOT_OK;
    }

    if (ioctl(mLcdFd, LCD_IOC_CMD_GET_SS, param) < 0) {
        LOGE("%s VBE ioctl LCD_IOC_CMD_GET_SS fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}
