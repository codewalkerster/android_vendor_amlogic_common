/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef __SSM_HANDLER_H__
#define __SSM_HANDLER_H__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <utils/threads.h>
#include <vector>
#include <memory>

typedef enum SSM_status_e
{
    SSM_HEADER_INVALID = 0,
    SSM_HEADER_VALID = 1,
    SSM_HEADER_STRUCT_CHANGE = 2,
} SSM_status_t;

enum{
    CHKSUM_PROJECT_ID_OFFSET = 0,
    CHKSUM_MAC_ADDRESS_OFFSET,
    CHKSUM_HDCP_KEY_OFFSET,
    CHKSUM_BARCODE_OFFSET,
    SSM_RSV_W_CHARACTER_CHAR_START,
    SSM_CR_START,
    SSM_MARK_01_START,
    SSM_MARK_02_START,
    SSM_MARK_03_START,
    SSM_RSV0,
    SSM_RW_FBMF_START,
    SSM_RW_DEF_HDCP_START,
    SSM_RW_POWER_CHANNEL_START,
    SSM_RW_LAST_SOURCE_INPUT_START,
    SSM_RW_SYS_LANGUAGE_START,
    SSM_RW_AGING_MODE_START,
    SSM_RW_PANEL_TYPE_START,
    SSM_RW_POWER_ON_MUSIC_SWITCH_START,
    SSM_RW_POWER_ON_MUSIC_VOL_START,
    SSM_RW_SYS_SLEEP_TIMER_START,
    SSM_RW_INPUT_SRC_PARENTAL_CTL_START,
    SSM_RW_PARENTAL_CTL_SWITCH_START,
    SSM_RW_PARENTAL_CTL_PASSWORD_START,
    SSM_RW_SEARCH_NAVIGATE_FLAG_START,
    SSM_RW_INPUT_NUMBER_LIMIT_START,
    SSM_RW_SERIAL_ONOFF_FLAG_START,
    SSM_RW_STANDBY_MODE_FLAG_START,
    SSM_RW_HDMIEQ_MODE_START,
    SSM_RW_LOGO_ON_OFF_FLAG_START,
    SSM_RW_HDMIINTERNAL_MODE_START,
    SSM_RW_DISABLE_3D_START,
    SSM_RW_GLOBAL_OGO_ENABLE_START,
    SSM_RW_LOCAL_DIMING_START,
    SSM_RW_VDAC_2D_START,
    SSM_RW_VDAC_3D_START,
    SSM_RW_NON_STANDARD_START,
    SSM_RW_ADB_SWITCH_START,
    SSM_RW_SERIAL_CMD_SWITCH_START,
    SSM_RW_CA_BUFFER_SIZE_START,
    SSM_RW_NOISE_GATE_THRESHOLD_START,
    SSM_RW_DTV_TYPE_START,
    SSM_RW_UI_GRHPHY_BACKLIGHT_START,
    SSM_RW_FASTSUSPEND_FLAG_START,
    SSM_RW_BLACKOUT_ENABLE_START,
    SSM_RW_PANEL_ID_START,
    SSM_RSV_1,
    SSM_RSV_2,
    VPP_DATA_POS_COLOR_DEMO_MODE_START,
    VPP_DATA_POS_TEST_PATTERN_START,
    VPP_DATA_POS_DDR_SSC_START,
    VPP_DATA_POS_LVDS_SSC_START,
    VPP_DATA_POS_DREAM_PANEL_START,
    VPP_DATA_POS_BACKLIGHT_REVERSE_START,
    VPP_DATA_POS_SCENE_MODE_START,
    VPP_DATA_POS_DBC_START,
    VPP_DATA_PROJECT_ID_START,
    VPP_DATA_POS_DNLP_START,
    VPP_DATA_POS_PANORAMA_START,
    VPP_DATA_APL_START,
    VPP_DATA_APL2_START,
    VPP_DATA_BD_START,
    VPP_DATA_BP_START,
    VPP_DATA_USER_NATURE_SWITCH_START,
    VPP_DATA_DBC_BACKLIGHT_START,
    VPP_DATA_DBC_STANDARD_START,
    VPP_DATA_DBC_ENABLE_START,
    VPP_DATA_POS_FBC_BACKLIGHT_START,
    VPP_DATA_POS_FBC_ELECMODE_START,
    VPP_DATA_POS_FBC_COLORTEMP_START,
    VPP_DATA_POS_FBC_N310_BACKLIGHT_START,
    VPP_DATA_POS_FBC_N310_COLORTEMP_START,
    VPP_DATA_POS_FBC_N310_LIGHTSENSOR_START,
    VPP_DATA_POS_FBC_N310_MEMC_START,
    VPP_DATA_POS_FBC_N310_DREAMPANEL_START,
    VPP_DATA_POS_FBC_N310_MULTI_PQ_START,
    VPP_DATA_POS_N311_VBYONE_SPREAD_SPECTRUM_START,
    VPP_DATA_POS_N311_BLUETOOTH_VAL_START,
    VPP_DATA_POS_DLG_ENABLE_START,
    VPP_DATA_POS_VRR_ENABLE_START,
    VPP_DATA_PQMODULE_DEMO_STATE_START,
    SSM_RSV3,
    TVIN_DATA_POS_SOURCE_INPUT_START,
    TVIN_DATA_CVBS_STD_START,
    TVIN_DATA_POS_3D_MODE_START,
    TVIN_DATA_POS_3D_LRSWITCH_START,
    TVIN_DATA_POS_3D_DEPTH_START,
    TVIN_DATA_POS_3D_TO2D_START,
    TVIN_DATA_POS_3D_TO2DNEW_START,
    CUSTOMER_DATA_POS_HDMI1_EDID_START,
    CUSTOMER_DATA_POS_HDMI2_EDID_START,
    CUSTOMER_DATA_POS_HDMI3_EDID_START,
    CUSTOMER_DATA_POS_HDMI4_EDID_START,
    CUSTOMER_DATA_POS_HDMI_HDCP_SWITCHER_START,
    CUSTOMER_DATA_POS_HDMI_COLOR_RANGE_START,
    CUSTOMER_DATA_POS_AUTO_ASPECT,
    CUSTOMER_DATA_POS_43_STRETCH,
    CUSTOMER_DATA_POS_SCREEN_COLOR_START,
    CUSTOMER_DATA_POS_CHANNEL_LOCK_EN_START,
    SSM_DATA_MAX,
};

typedef enum ssm_source_input_e {
    SSM_SOURCE_INVALID = -1,
    SSM_SOURCE_TV = 0,
    SSM_SOURCE_AV1,
    SSM_SOURCE_AV2,
    SSM_SOURCE_YPBPR1,
    SSM_SOURCE_YPBPR2,
    SSM_SOURCE_HDMI1,
    SSM_SOURCE_HDMI2,
    SSM_SOURCE_HDMI3,
    SSM_SOURCE_HDMI4,
    SSM_SOURCE_VGA,
    SSM_SOURCE_MPEG,
    SSM_SOURCE_DTV,
    SSM_SOURCE_SVIDEO,
    SSM_SOURCE_IPTV,
    SSM_SOURCE_DUMMY,
    SSM_SOURCE_SPDIF,
    SSM_SOURCE_ADTV,
    SSM_SOURCE_MAX,
} ssm_source_input_t;

typedef enum pq_ssm_source_input_e {
    PQ_SSM_SOURCE_DEFAULT = 0,
    PQ_SSM_SOURCE_TV,
    PQ_SSM_SOURCE_AV1,
    PQ_SSM_SOURCE_AV2,
    PQ_SSM_SOURCE_YPBPR1,
    PQ_SSM_SOURCE_YPBPR2,
    PQ_SSM_SOURCE_HDMI1,
    PQ_SSM_SOURCE_HDMI2,
    PQ_SSM_SOURCE_HDMI3,
    PQ_SSM_SOURCE_HDMI4,
    PQ_SSM_SOURCE_VGA,
    PQ_SSM_SOURCE_MPEG,
    PQ_SSM_SOURCE_DTV,
    PQ_SSM_SOURCE_SVIDEO,
    PQ_SSM_SOURCE_IPTV,
    PQ_SSM_SOURCE_DUMMY,
    PQ_SSM_SOURCE_SPDIF,
    PQ_SSM_SOURCE_ADTV,
    PQ_SSM_SOURCE_MAX,
} pq_ssm_source_input_t;

typedef enum ssm_sig_fmt_e {
    SSM_FMT_DEFAULT = 0,
    SSM_FMT_SDR,
    SSM_FMT_HDR,
    SSM_FMT_HDRP,
    SSM_FMT_HLG,
    SSM_FMT_DOBLY,
    SSM_FMT_MAX,
} ssm_sig_fmt_t;

typedef enum ssm_pq_module_demo_e
{
    SSM_PQ_DEMO_MEMC = 0,
    SSM_PQ_DEMO_AISR,
    SSM_PQ_DEMO_MAX,
} ssm_pq_module_demo_t;

struct SSMHeader_section1_t
{
    unsigned int magic;
    unsigned int version;//count by ID and its size
    unsigned int count;//count by column of excel
    unsigned int rsv[6];//reserve
};

struct SSMHeader_section2_t
{
    unsigned int id;  //Id index
    unsigned int addr;//Id's addr
    unsigned int size;//this item size
    unsigned int valid;
    unsigned char rsv[6];
};

extern struct SSMHeader_section1_t gSSMHeader_section1;
extern struct SSMHeader_section2_t gSSMHeader_section2[];

class SSMHandler
{
public:
    static SSMHandler* GetSingletonInstance(const char *SSMHandlerPath);
    virtual ~SSMHandler();
    SSM_status_t SSMVerify(void);
    bool SSMRecreateHeader(void);
    unsigned int SSMGetActualAddr(int id);
    unsigned int SSMGetActualSize(int id);

private:
    int mFd;
    static SSMHandler *mSSMHandler;
    SSMHeader_section1_t mSSMHeader_section1;
    static android::Mutex sLock;
    char mSSMHandlerPath[128];

    bool Construct();
    explicit SSMHandler();
    SSM_status_t SSMSection1Verify();
    SSM_status_t  SSMSection2Verify(SSM_status_t);
    SSMHandler& operator = (const SSMHandler&);
};


#endif
