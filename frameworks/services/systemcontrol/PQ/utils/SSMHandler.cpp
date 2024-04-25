/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "SystemControl"
#define LOG_TV_TAG "SSMHandler"

#include "CPQLog.h"
#include "SSMHandler.h"
#include "PQType.h"

android::Mutex SSMHandler::sLock;
SSMHandler* SSMHandler::mSSMHandler = NULL;

struct SSMHeader_section2_t gSSMHeader_section2[] = {
    {.id =CHKSUM_PROJECT_ID_OFFSET, .addr = 0, .size = 2, .valid = 0, .rsv = {0}},
    {.id =CHKSUM_MAC_ADDRESS_OFFSET, .addr = 0, .size = 2, .valid = 0, .rsv = {0}},
    {.id =CHKSUM_HDCP_KEY_OFFSET, .addr = 0, .size = 2, .valid = 0, .rsv = {0}},
    {.id =CHKSUM_BARCODE_OFFSET, .addr = 0, .size = 4, .valid = 0, .rsv = {0}},
    {.id =SSM_RSV_W_CHARACTER_CHAR_START, .addr = 0, .size = 10, .valid = 0, .rsv = {0}},
    {.id =SSM_CR_START, .addr = 0, .size = 1536, .valid = 0, .rsv = {0}},
    {.id =SSM_MARK_01_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_MARK_02_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_MARK_03_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RSV0, .addr = 0, .size = 0, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_FBMF_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_DEF_HDCP_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_POWER_CHANNEL_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_LAST_SOURCE_INPUT_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_SYS_LANGUAGE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_AGING_MODE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_PANEL_TYPE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_POWER_ON_MUSIC_SWITCH_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_POWER_ON_MUSIC_VOL_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_SYS_SLEEP_TIMER_START, .addr = 0, .size = 4, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_INPUT_SRC_PARENTAL_CTL_START, .addr = 0, .size = 4, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_PARENTAL_CTL_SWITCH_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_PARENTAL_CTL_PASSWORD_START, .addr = 0, .size = 16, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_SEARCH_NAVIGATE_FLAG_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_INPUT_NUMBER_LIMIT_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_SERIAL_ONOFF_FLAG_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_STANDBY_MODE_FLAG_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_HDMIEQ_MODE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_LOGO_ON_OFF_FLAG_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_HDMIINTERNAL_MODE_START, .addr = 0, .size = 4, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_DISABLE_3D_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_GLOBAL_OGO_ENABLE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_LOCAL_DIMING_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_VDAC_2D_START, .addr = 0, .size = 2, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_VDAC_3D_START, .addr = 0, .size = 2, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_NON_STANDARD_START, .addr = 0, .size = 2, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_ADB_SWITCH_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_SERIAL_CMD_SWITCH_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_CA_BUFFER_SIZE_START, .addr = 0, .size = 2, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_NOISE_GATE_THRESHOLD_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_DTV_TYPE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_UI_GRHPHY_BACKLIGHT_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_FASTSUSPEND_FLAG_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_BLACKOUT_ENABLE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RW_PANEL_ID_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =SSM_RSV_1, .addr = 0, .size = 0, .valid = 0, .rsv = {0}},
    {.id =SSM_RSV_2, .addr = 0, .size = 0, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_COLOR_DEMO_MODE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_TEST_PATTERN_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_DDR_SSC_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_LVDS_SSC_START, .addr = 0, .size = 3, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_DREAM_PANEL_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_BACKLIGHT_REVERSE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_SCENE_MODE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_DBC_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_PROJECT_ID_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_DNLP_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_PANORAMA_START, .addr = 0, .size = SSM_SOURCE_MAX, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_APL_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_APL2_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_BD_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_BP_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_USER_NATURE_SWITCH_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_DBC_BACKLIGHT_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_DBC_STANDARD_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_DBC_ENABLE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_FBC_BACKLIGHT_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_FBC_ELECMODE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_FBC_COLORTEMP_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_FBC_N310_BACKLIGHT_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_FBC_N310_COLORTEMP_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_FBC_N310_LIGHTSENSOR_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_FBC_N310_MEMC_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_FBC_N310_DREAMPANEL_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_FBC_N310_MULTI_PQ_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_N311_VBYONE_SPREAD_SPECTRUM_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_N311_BLUETOOTH_VAL_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_DLG_ENABLE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_POS_VRR_ENABLE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =VPP_DATA_PQMODULE_DEMO_STATE_START, .addr = 0, .size = SSM_PQ_DEMO_MAX, .valid = 0, .rsv = {0}},
    {.id =SSM_RSV3, .addr = 0, .size = 0, .valid = 0, .rsv = {0}},
    {.id =TVIN_DATA_POS_SOURCE_INPUT_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =TVIN_DATA_CVBS_STD_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =TVIN_DATA_POS_3D_MODE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =TVIN_DATA_POS_3D_LRSWITCH_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =TVIN_DATA_POS_3D_DEPTH_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =TVIN_DATA_POS_3D_TO2D_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =TVIN_DATA_POS_3D_TO2DNEW_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =CUSTOMER_DATA_POS_HDMI1_EDID_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =CUSTOMER_DATA_POS_HDMI2_EDID_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =CUSTOMER_DATA_POS_HDMI3_EDID_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =CUSTOMER_DATA_POS_HDMI4_EDID_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =CUSTOMER_DATA_POS_HDMI_HDCP_SWITCHER_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =CUSTOMER_DATA_POS_HDMI_COLOR_RANGE_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =CUSTOMER_DATA_POS_AUTO_ASPECT, .addr = 0, .size = SSM_SOURCE_MAX, .valid = 0, .rsv = {0}},
    {.id =CUSTOMER_DATA_POS_43_STRETCH, .addr = 0, .size = SSM_SOURCE_MAX, .valid = 0, .rsv = {0}},
    {.id =CUSTOMER_DATA_POS_SCREEN_COLOR_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
    {.id =CUSTOMER_DATA_POS_CHANNEL_LOCK_EN_START, .addr = 0, .size = 1, .valid = 0, .rsv = {0}},
};

struct SSMHeader_section1_t gSSMHeader_section1 =
{
    .magic = 0x8f8f8f8f, .version = 2021811022, .count = SSM_DATA_MAX, .rsv = {0}
};

SSMHandler* SSMHandler::GetSingletonInstance(const char *SSMHandlerPath)
{
    android::Mutex::Autolock _l(sLock);

    if (!mSSMHandler) {
        mSSMHandler = new SSMHandler();
        if (strlen(SSMHandlerPath) < sizeof(mSSMHandler->mSSMHandlerPath)/sizeof(char)) {
            strcpy(mSSMHandler->mSSMHandlerPath, SSMHandlerPath);
        }

        if (mSSMHandler && !mSSMHandler->Construct()) {
            delete mSSMHandler;
            mSSMHandler = NULL;
        }
    }

    return mSSMHandler;
}

SSMHandler::SSMHandler()
{
    unsigned int sum = 0;

    memset(&mSSMHeader_section1, 0, sizeof (SSMHeader_section1_t));

    for (unsigned int i = 1; i < gSSMHeader_section1.count; i++) {
        sum += gSSMHeader_section2[i-1].size;

        gSSMHeader_section2[i].addr = sum;
    }

    mFd = -1;
}

SSMHandler::~SSMHandler()
{
    if (mFd > 0) {
        close(mFd);
        mFd = -1;
    }
}

bool SSMHandler::Construct()
{
    bool ret = true;

    mFd = open(mSSMHandlerPath, O_RDWR | O_SYNC | O_CREAT, S_IRUSR | S_IWUSR);

    if (-1 == mFd) {
        ret = false;
        SYS_LOGD ("%s, Open %s failure\n", __FUNCTION__, mSSMHandlerPath);
    }

    return ret;
}

SSM_status_t SSMHandler::SSMSection1Verify()
{
    SSM_status_t ret = SSM_HEADER_VALID;

    lseek(mFd, 0, SEEK_SET);
    ssize_t ssize = read(mFd, &mSSMHeader_section1, sizeof(SSMHeader_section1_t));

    if (ssize != sizeof(SSMHeader_section1_t) ||
        mSSMHeader_section1.magic != gSSMHeader_section1.magic ||
        mSSMHeader_section1.count != gSSMHeader_section1.count) {
        ret = SSM_HEADER_INVALID;
    }

    if (ret != SSM_HEADER_INVALID &&
        mSSMHeader_section1.version != gSSMHeader_section1.version) {
        ret = SSM_HEADER_STRUCT_CHANGE;
    }

    return ret;
}

SSM_status_t SSMHandler::SSMSection2Verify(SSM_status_t SSM_status)
{
    return SSM_status;
}

bool SSMHandler::SSMRecreateHeader()
{
    bool ret = true;

    if (ftruncate(mFd, 0) < 0)
        SYS_LOGE("%s ftruncate failed\n", __FUNCTION__);
    lseek(mFd, 0, SEEK_SET);

    //cal Addr and write
    write(mFd, &gSSMHeader_section1, sizeof (SSMHeader_section1_t));
    write(mFd, gSSMHeader_section2, gSSMHeader_section1.count * sizeof (SSMHeader_section2_t));

    return ret;
}

unsigned int SSMHandler::SSMGetActualAddr(int id)
{
    return gSSMHeader_section2[id].addr;
}

unsigned int SSMHandler::SSMGetActualSize(int id)
{
    return gSSMHeader_section2[id].size;
}

SSM_status_t SSMHandler::SSMVerify()
{
    return  SSMSection2Verify(SSMSection1Verify());
}

SSMHandler& SSMHandler::operator = (const SSMHandler& obj)
{
    return const_cast<SSMHandler&>(obj);
}
