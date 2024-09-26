/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_TAG "SystemControl"
#define LOG_TV_TAG "CPQControl"

#include <cutils/properties.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <dlfcn.h>

#include "CPQControl.h"
#include "tconless/demura/CTconDemura.h"
#include "tconless/pgamma/CTconPGamma.h"

#include "LocalDimming/aml_hal_ld.h"
#include "panel/aml_hal_lcd.h"

#ifdef SUPPORT_TVSERVICE
#include "TvServerHidlClient.h"
#endif

#define PROP_DEMURA_AUTO_GEN "ro.vendor.demura.autogen"
#define PROP_PGAMMA_AUTO_GEN "ro.vendor.pgamma.autogen"

#define PI 3.14159265358979
CPQControl *CPQControl::mInstance = NULL;
CPQControl *CPQControl::GetInstance()
{
    if (NULL == mInstance)
        mInstance = new CPQControl();
    return mInstance;
}

CPQControl::CPQControl()
{
    memset(&mCurrentSourceInputInfo, 0, sizeof(source_input_param_t));

    mPQConfigFile = CConfigFile::GetInstance();
    pqSysWrite = SysWrite::GetInstance();

    mDataBase = new CPQDataBase();
    mDataBase->Init(NULL);

    mWBDataBase = new WhitebalanceDataBase();
}

CPQControl::~CPQControl()
{
}

void CPQControl::CPQControlInit()
{
    SYS_LOGD("CPQControlInit start!\n");

    AML_HAL_LCD_INIT();
    AML_HAL_LD_INIT();

    //open vpp module
    mAmvideoFd = VPPOpenModule();
    if (mAmvideoFd < 0) {
        SYS_LOGE("Open PQ module failed\n");
    } else {
        SYS_LOGD("Open PQ module success\n");
    }

    //open DI module
    mDiFd = DIOpenModule();
    if (mDiFd < 0) {
        SYS_LOGE("Open DI module failed!\n");
    } else {
        SYS_LOGD("Open DI module success!\n");
    }

    //open MEMC module
    mMemcFd = MEMCOpenModule();
    if (mMemcFd < 0) {
        SYS_LOGE("Open MEMC module failed!\n");
    } else {
        SYS_LOGD("Open MEMC module success!\n");
    }

    //open VT module
    mVideoTunelFd = OpenVideotunnel();
    if (mVideoTunelFd < 0) {
        SYS_LOGE("Open VideoTunel module failed!\n");
    } else {
        SYS_LOGD("Open VideoTunel module success!\n");
    }

    //open VDIN module
    mVdin0DevFd = VDINOpenModule();
    if (mVdin0DevFd < 0) {
        SYS_LOGE("Open VDIN module failed!\n");
    } else {
        SYS_LOGD("Open VDIN module success!\n");
    }

    //Load config file
    SetFlagByCfg();

    //open DB
    int ret = -1;
    char dstPqDbPath[128] = {0};
    mPQdb = new CPQdb();
    mPQConfigFile->GetPqdbPath(dstPqDbPath);
    ret = mPQdb->openPqDB(dstPqDbPath);
    if (ret != 0) {
        mbDatabaseMatchChipStatus = false;
        SYS_LOGE("open pq DB failed!\n");
    } else {
        SYS_LOGD("open pq DB success!\n");
        mbDatabaseMatchChipStatus = isPqDatabaseMachChip();
    }

    //open Ext DB
    char pq_ext_db_path[128] = {0};
    mPQConfigFile->GetPqExtdbPath(pq_ext_db_path);
    mPQExtdb = new CPQExtdb();
    ret = mPQExtdb->openPqExtDB(pq_ext_db_path);
    if (ret != 0) {
        SYS_LOGE("open pq_ext.db failed!\n");
    } else {
        SYS_LOGD("open pq_ext.db success!\n");
    }

    //open overscan DB
    if (mbCpqCfg_separate_db_enable) {
        char dstOverscanDbPath[128] = {0};
        mpOverScandb = new COverScandb();
        mPQConfigFile->GetOverscandbPath(dstOverscanDbPath);
        ret = mpOverScandb->openOverScanDB(dstOverscanDbPath);
        if (ret != 0) {
            SYS_LOGE("open overscan DB failed!\n");
        } else {
            SYS_LOGD("open overscan DB success!\n");
        }
    }

    //check ldim.bin file path
    if (HasLocalDimming()) {
        char dstLdimBinPath[128] = {0};
        mPQConfigFile->GetLdimBinPath(dstLdimBinPath);
        SYS_LOGD("ldim.bin path:%s\n", dstLdimBinPath);
    }

    //SSM file check
    char SsmDataPath[128]        = {0};
    char SsmDataHandlerPath[128] = {0};
    char WBPath[128]             = {0};
    mPQConfigFile->GetSSMDataPath(SsmDataPath);
    mPQConfigFile->GetSSMDataHandlerPath(SsmDataHandlerPath);
    mPQConfigFile->GetWBFilePath(WBPath);
    mSSMAction = SSMAction::getInstance();
    mSSMAction->setObserver(this);
    mSSMAction->init(SsmDataPath, SsmDataHandlerPath, WBPath);

    //init source
    mCurrentSourceInputInfo.source_input = SOURCE_MPEG;
    mCurrentSourceInputInfo.sig_fmt      = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
    mCurrentSourceInputInfo.trans_fmt    = TVIN_TFMT_2D;

    //check output mode
    mCurrentOutputType = CheckOutPutMode(SOURCE_MPEG);

    //load DV config file
    char dvbinpath[128] = {0};
    char dvcfgpath[128] = {0};
    mPQConfigFile->GetDvFilePath(dvbinpath, dvcfgpath);
    mDolbyVision = new CDolbyVision(dvbinpath, dvcfgpath);

    //HLG tone mapping init
    mHlgToneMapping = new CHlgToneMapping();
    int gainValue[149] = {0};
    mHlgToneMapping->hlg_sdr_process(350, gainValue);
    SetHDRTMData(gainValue);

    //screen color
    //SetScreenColorForSignalChange(GetScreenColorForSignalChange(), 0);
    //static frame
    SetStaticFrameEnable(GetStaticFrameEnable(), 0);

    //Load PQ
    if (LoadPQSettings() < 0) {
        SYS_LOGE("Load PQ failed!\n");
    } else {
        SYS_LOGD("Load PQ success!\n");
    }

    //set backlight
    BacklightInit();

    //auto backlight
    DynamicBackLightInit();

    //cabc pq
    SetCabc();

    //aad pq
    SetAad();

    //Vframe size
    mCDevicePollCheckThread = sp<CDevicePollCheckThread>::make();
    mCDevicePollCheckThread->setObserver(this);
    mCDevicePollCheckThread->StartCheck();

    InitAutoNr();

    //for tconless
    InitTconGamma();

    InitTconlessBin();

    Cpq_SetOsdSharpness(GetOsdSharpness());

    mInitialized = true;
}

void CPQControl::CPQControlUnInit()
{
    //close moduel
    VPPCloseModule();
    //close DI module
    DICloseModule();
    //close VT module;
    CloseVideotunnel();
    //close VDIN module
    VDINCloseModule();

    if (mHlgToneMapping != NULL) {
        delete mHlgToneMapping;
        mHlgToneMapping = NULL;
    }

    if (mSSMAction!= NULL) {
        delete mSSMAction;
        mSSMAction = NULL;
    }

    if (mDolbyVision != NULL) {
        delete mDolbyVision;
        mDolbyVision = NULL;
    }

    if (mPQdb != NULL) {
        //closed DB
        mPQdb->closeDb();

        delete mPQdb;
        mPQdb = NULL;
    }

    if (mpOverScandb != NULL) {
        mpOverScandb->closeDb();

        delete mpOverScandb;
        mpOverScandb = NULL;
    }

    mCDevicePollCheckThread->requestExit();

    if (mPQConfigFile != NULL) {
        delete mPQConfigFile;
        mPQConfigFile = NULL;
    }

    if (mWBDataBase != NULL) {
        delete mWBDataBase;
        mWBDataBase = NULL;
    }

    if (mDataBase != NULL) {
        delete mDataBase;
        mDataBase = NULL;
    }
}

int CPQControl::pqWriteSys(ConstCharforSysNodeIndex index, const char *val)
{
    int len = -1;

    len = pqSysWrite->writeSysfs(index, val);

    return len;
}

int CPQControl::pqReadSys(ConstCharforSysNodeIndex index, char *buf, int count)
{
    int len = -1;

    len = pqSysWrite->readSysfs(index, buf, count);

    return len;
}

int CPQControl::VPPOpenModule(void)
{
    if (mAmvideoFd < 0) {
        mAmvideoFd = open(VPP_DEV_PATH, O_RDWR);
        if (mAmvideoFd < 0) {
            SYS_LOGE("Open amvecm module, error(%s)\n", strerror(errno));
            return -1;
        }
    } else {
        SYS_LOGD("vpp OpenModule has been opened before!\n");
    }

    return mAmvideoFd;
}

int CPQControl::VPPCloseModule(void)
{
    if (mAmvideoFd >= 0) {
        close ( mAmvideoFd);
        mAmvideoFd = -1;
    }
    return 0;
}

int CPQControl::VPPDeviceIOCtl(int request, ...)
{
    int ret = -1;
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    ret = ioctl(mAmvideoFd, request, arg);
    return ret;
}

int CPQControl::DIOpenModule(void)
{
    if (mDiFd < 0) {
        mDiFd = open(DI_DEV_PATH, O_RDWR);

        SYS_LOGD("DI OpenModule path: %s", DI_DEV_PATH);

        if (mDiFd < 0) {
            SYS_LOGE("Open DI module, error(%s)\n", strerror(errno));
            return -1;
        }
    }

    return mDiFd;
}

int CPQControl::DICloseModule(void)
{
    if (mDiFd>= 0) {
        close ( mDiFd);
        mDiFd = -1;
    }
    return 0;
}

int CPQControl::DIDeviceIOCtl(int request, ...)
{
    int tmp_ret = -1;
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    tmp_ret = ioctl(mDiFd, request, arg);
    return tmp_ret;
}

int CPQControl::AFEDeviceIOCtl ( int request, ... )
{
    int tmp_ret = -1;
    int afe_dev_fd = -1;
    va_list ap;
    void *arg;

    afe_dev_fd = open(AFE_DEV_PATH, O_RDWR );

    if ( afe_dev_fd >= 0 ) {
        va_start ( ap, request );
        arg = va_arg ( ap, void * );
        va_end ( ap );

        tmp_ret = ioctl ( afe_dev_fd, request, arg );

        close(afe_dev_fd);
        return tmp_ret;
    } else {
        SYS_LOGE ( "Open tvafe module error(%s).\n", strerror ( errno ));
        return -1;
    }
}

int CPQControl::LDOpenModule(void)
{
    if (!isFileExist(LDIM_PATH)) {
        SYS_LOGD("not support LocalDimming!\n");
        return -1;
    }

    if (mLdFd < 0) {
        mLdFd = open(LDIM_PATH, O_RDWR);
        if (mLdFd < 0) {
            SYS_LOGE("Open LocalDimming module, error(%s)!\n", strerror(errno));
            return -1;
        }
    } else {
        SYS_LOGD("LocalDimming OpenModule has been opened before!\n");
    }

    return mLdFd;
}

int CPQControl::LDCloseModule(void)
{
    if (mLdFd >= 0) {
        close ( mLdFd);
        mLdFd = -1;
    }
    return 0;
}

int CPQControl::LDDeviceIOCtl(int request, ...)
{
    int ret = -1;
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    ret = ioctl(mLdFd, request, arg);
    return ret;
}

int CPQControl::MEMCOpenModule(void)
{
    if (mMemcFd < 0) {
        mMemcFd = open(CPQ_MEMC_SYSFS, O_RDWR);

        SYS_LOGD("MEMC OpenModule path: %s", CPQ_MEMC_SYSFS);

        if (mMemcFd < 0) {
            SYS_LOGE("Open MEMC module, error(%s)!\n", strerror(errno));
            return -1;
        }
    }

    return mMemcFd;
}

int CPQControl::MEMCCloseModule(void)
{
    if (mMemcFd>= 0) {
        close ( mMemcFd);
        mMemcFd = -1;
    }
    return 0;
}

int CPQControl::MEMCDeviceIOCtl(int request, ...)
{
    int tmp_ret = -1;
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    tmp_ret = ioctl(mMemcFd, request, arg);
    return tmp_ret;
}

int CPQControl::LCDOpenModule(void)
{
    if (mLcdFd < 0) {
        mLcdFd = open(CPQ_LCD_SYSFS, O_RDWR);

        SYS_LOGD("LCD OpenModule path: %s", CPQ_LCD_SYSFS);

        if (mLcdFd < 0) {
            SYS_LOGE("Open LCD module, error(%s)!\n", strerror(errno));
            return -1;
        }
    }

    return mLcdFd;
}

int CPQControl::LCDCloseModule(void)
{
    if (mLcdFd>= 0) {
        close ( mLcdFd);
        mLcdFd = -1;
    }
    return 0;
}

int CPQControl::LCDDeviceIOCtl(int request, ...)
{
    int tmp_ret = -1;
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    tmp_ret = ioctl(mLcdFd, request, arg);
    return tmp_ret;
}

int CPQControl::VDINOpenModule()
{
    if (mVdin0DevFd < 0) {
        mVdin0DevFd = open(VDIN_DEV_PATH, O_RDWR );
    }

    SYS_LOGD("Vdin OpenModule path: %s", VDIN_DEV_PATH);

    if (mVdin0DevFd < 0) {
        SYS_LOGE("Open Vdin module, error(%s)!\n", strerror(errno));
        return -1;
    }

    return mVdin0DevFd;
}

int CPQControl::VDINCloseModule()
{
    if (mVdin0DevFd != -1) {
        close ( mVdin0DevFd );
        mVdin0DevFd = -1;
    }

    return 0;
}

int CPQControl::VDINDeviceIOCtl( int request, ... )
{
    int tmp_ret = -1;
    va_list ap;
    void *arg;

    va_start (ap, request);
    arg = va_arg (ap, void *);
    va_end (ap);

    tmp_ret = ioctl(mVdin0DevFd, request, arg);
    return tmp_ret;
}

void CPQControl::onVframeSizeChange()
{
    char temp[8];
    memset(temp, 0, sizeof(temp));
    int ret = pqReadSys(VIDEO_POLL_STATUS_CHANGE, temp, sizeof(temp));
    if (ret > 0) {
        int eventFlagValue = strtol(temp, NULL, 16);
        SYS_LOGD("%s: event value = %d(0x%x)\n", __FUNCTION__, eventFlagValue, eventFlagValue);
        int framesizeEventFlag       = (eventFlagValue & 0x1) >> 0;
        int hdrTypeEventFlag         = (eventFlagValue & 0x2) >> 1;
        int videoPlayStartEventFlag  = (eventFlagValue & 0x4) >> 2;
        int videoPlayStopEventFlag   = (eventFlagValue & 0x8) >> 3;
      //int videoPlayAxisEventFlag  = (eventFlagValue & 0x10) >> 4;
        int sliceNumFlag             = (eventFlagValue & 0x60) >> 5;
        int FilmmakerModeFlag        = (eventFlagValue & 0x20) >> 5;
        int FilmmakerModeDisableFlag = (eventFlagValue & 0x40) >> 6;

        /*
        SYS_LOGD("%s: framesizeEventFlag = %d,hdrTypeEventFlag = %d,"
                "videoPlayStartEventFlag = %d,videoPlayStopEventFlag = %d,"
                "videoPlayAxisEventFlag = %d!\n", __FUNCTION__,
                framesizeEventFlag, hdrTypeEventFlag, videoPlayStartEventFlag,
                videoPlayStopEventFlag, videoPlayAxisEventFlag, sliceNumFlag);
        */

        //check video play start or stop
        if ((videoPlayStartEventFlag == 1) && (videoPlayStopEventFlag == 0)) {
            mbVideoIsPlaying = true;
        } else if ((videoPlayStartEventFlag == 0) && (videoPlayStopEventFlag == 1)) {
            mbVideoIsPlaying = false;
        } else {
            SYS_LOGE("%s: invalid case\n", __FUNCTION__);
        }

        source_input_param_t new_source_input_param;
        new_source_input_param = GetCurrentSourceInputInfo();
        if (((new_source_input_param.source_input == SOURCE_DTV) || (new_source_input_param.source_input == SOURCE_MPEG))
            && (framesizeEventFlag == 1)) {
            if (isBootvideoStopped()) {
                new_source_input_param.sig_fmt = getVideoResolutionToFmt();
                SYS_LOGD("%s: sig_fmt = 0x%x(%d)\n", __FUNCTION__, new_source_input_param.sig_fmt, new_source_input_param.sig_fmt);
                SetCurrentSourceInputInfo(new_source_input_param);
            } else {
                SYS_LOGD("%s: bootvideo don't stop\n", __FUNCTION__);
            }
        }

        if (hdrTypeEventFlag == 0x1) {
            //get hdr type
            hdr_type_t newHdrType = HDR_TYPE_NONE;
            newHdrType            = Cpq_GetSourceHDRType(mCurrentSourceInputInfo);

            //notify hdr event to framework
            if (mCurrentHdrType != newHdrType) {
                mCurrentHdrType = newHdrType;
                if (mNotifyListener != NULL) {
                    SYS_LOGD("%s: send hdr event, info is %d\n", __FUNCTION__, mCurrentHdrType);
                    mNotifyListener->onHdrInfoChange(mCurrentHdrType);
                } else {
                    SYS_LOGE("%s: mNotifyListener is NULL\n", __FUNCTION__);
                }
            }
        } else {
            SYS_LOGD("%s: not hdrInfo event\n", __FUNCTION__);
        }

        if (sliceNumFlag == 3) {
            if (mPQdb->mDbMatchType == MATCH_TYPE_MBOX_T3X) {
                /* workarround for T968D4 4K120HZ panel
                ** slice_num was changed dynamically when switch timming/aisr/memc/etc in driver
                ** so whatever decoder or vdin, send event by /dev/amlvideo_poll that register in vpp
                ** when upper monitor event VIDEO_PROP_CHANGE_SLICE_NUM re-load sr table
                */
                SYS_LOGD("%s: VIDEO_PROP_CHANGE_SLICE_NUM trigger amvideo_poll event\n", __FUNCTION__);
                SetSrTable_WorkArroundByEvent();
            }
        }

        if (FilmmakerModeFlag == 1) {
            SYS_LOGD("%s: VIDEO_PROP_CHANGE_FMM trigger amlvideo_poll event wake_up\n", __FUNCTION__);
            SetFilmMakerFlag(1);
        } else if (FilmmakerModeDisableFlag == 1) {
            SYS_LOGD("%s: VIDEO_PROP_CHANGE_FMM_DISABLE trigger amlvideo_poll event wake_up\n", __FUNCTION__);
            SetFilmMakerFlag(0);
        }
    } else {
        SYS_LOGE("%s: read video event failed\n", __FUNCTION__);
    }
}

tvin_sig_fmt_t CPQControl::getVideoResolutionToFmt()
{
    int ret = -1;
    char buf[32] = {0};
    tvin_sig_fmt_t sig_fmt = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;

    ret = pqReadSys(VIDEO_FRAME_HEIGHT, buf, sizeof(buf));
    if (ret > 0) {
        int height = atoi(buf);
        if (height <= 480) {
            sig_fmt = TVIN_SIG_FMT_HDMI_720X480P_60HZ;
        } else if (height > 480 && height <= 576) {
            sig_fmt = TVIN_SIG_FMT_HDMI_720X576P_50HZ;
        } else if (height > 576 && height <= 720) {
            sig_fmt = TVIN_SIG_FMT_HDMI_1280X720P_60HZ;
        } else if (height > 720 && height <= 1088) {
            sig_fmt = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
        } else {
            sig_fmt = TVIN_SIG_FMT_HDMI_3840_2160_00HZ;
        }
    } else {
        SYS_LOGE("[%s] read error!\n", __FUNCTION__);
    }

    return sig_fmt;
}

void CPQControl::onTXStatusChange()
{
    SYS_LOGI("%s!\n", __FUNCTION__);
    SetCurrentSourceInputInfo(mCurrentSourceInputInfo);
}

int CPQControl::isGameMode() {
    /*vendor.media.omx.gamemode.status       R PF used for clound game
     *vendor.media.c2.vdec.game_low_latency  U PF used for clound game
     */
    bool isCodec2 = property_get_bool("vendor.media.codec2.support", false);
    if (isCodec2) {
        return property_get_int32("vendor.media.c2.vdec.game_low_latency", 0);
    } else {
        return property_get_int32("vendor.media.omx.gamemode.status", 0);
    }
}

int CPQControl::LoadPQSettings()
{
    if (IsDisableAllPQ()) {
        return 0;
    }

     SYS_LOGI("source_input: %d, sig_fmt: 0x%x(%d), trans_fmt: 0x%x\n", mCurrentSourceInputInfo.source_input,
              mCurrentSourceInputInfo.sig_fmt, mCurrentSourceInputInfo.sig_fmt, mCurrentSourceInputInfo.trans_fmt);

     SYS_LOGI("pq_source_input: %d, timming: %d, \n", CurSource, CurTimming);

     int ret = 0;
     IsDvApoTypeGame = 0;
     ret |= Set_PictureMode((PICTURE_MODE)GetPQMode(), PQ_MODE_SWITCH_TYPE_INIT);
     ret |= LoadPQTableSettings();

     return ret;
}

int CPQControl::LoadPQTableSettings()
{
    int ret = 0;

    ret |= Cpq_SetXVYCCMode(VPP_XVYCC_MODE_STANDARD, mCurrentSourceInputInfo);

    ret |= Cpq_SetDIModuleParam(mCurrentSourceInputInfo);

    PICTURE_SETTING_BY_SRC BySrc;
    if (GetPictureStructDataBySrc(&BySrc)) {
        ret |= Cpq_SetMcDiMode((vpp_mcdi_mode_t)BySrc.McDiMode, mCurrentSourceInputInfo);

        ret |= SetDisplayMode((vpp_display_mode_t)BySrc.DisplayMode, 0);
    } else {
        SYS_LOGE("GetPictureStructDataBySrc fail\n");
    }

    PICTURE_SETTING_GLOBAL GLOBAL;
    if (GetPictureStructDataGlobal(&GLOBAL)) {
        ret |= Cpq_SetLocalDimming((vpp_pq_level_t)GLOBAL.LocalDimming);

        ret |= Cpq_SetColorBaseMode((vpp_color_basemode_t)GLOBAL.colorbase, mCurrentSourceInputInfo);

        ret |= Cpq_SetAiSrMode((aisr_mode_e)GLOBAL.aisr_mode, mCurrentSourceInputInfo);

        ret |= Cpq_SetAipqMode((aipq_mode_e)GLOBAL.aipq_mode, mCurrentSourceInputInfo);

        ret |= Cpq_SetAiColor(GLOBAL.ai_color);

        ret |= Cpq_SetSDR2HDR(GLOBAL.Sdr2Hdr);
    } else {
        SYS_LOGE("GetPictureStructDataGlobal fail\n");
    }

    TABLE_CMS CMS;
    if (GetColorCustomizeData(&CMS)) {
        if (CMS.CmsEnable == 0)
            memset(&CMS, 0, sizeof(TABLE_CMS));

        for (int i = COLOR_RED; i < COLOR_MAX; i++) {
            ret |= Cpq_SetColorCustomize((CMS_COLOR)i, Type_Saturation, CMS.CmsColor[i].Saturation);
            ret |= Cpq_SetColorCustomize((CMS_COLOR)i, Type_Hue, CMS.CmsColor[i].Hue);
            ret |= Cpq_SetColorCustomize((CMS_COLOR)i, Type_Luma, CMS.CmsColor[i].Luma);
        }
    }

    if (ret < 0) {
        SYS_LOGE("%s Some modules failed to set in PQTableSettings, please check!\n",__FUNCTION__);
    }

    SYS_LOGD("%s Done!\n",__FUNCTION__);
    return ret;
}

bool CPQControl::IsDisableAllPQ(void)
{
    int ret = 0;
    const char *config_value;
    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_ALL_PQ_MODULE_ENABLE, "enable");
    if ((strcmp(config_value, "enable") == 0) && (IsDongleLowPowerPqOff() == false)) {
        return false;
    }

    SYS_LOGD("All PQ module disabled!\n");

    pq_ctrl_t pqControlVal;
    memset(&pqControlVal, 0, sizeof(pq_ctrl_t));
    vpp_pq_ctrl_t amvecmConfigVal;
    amvecmConfigVal.length = 14;//this is the count of pq_ctrl_s option
    amvecmConfigVal.ptr = (long long)&pqControlVal;

    if (VPPDeviceIOCtl(AMVECM_IOC_S_PQ_CTRL, &amvecmConfigVal) < 0) {
        SYS_LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
        return false;
    }

    return true;
}

int CPQControl::PQModuleDemoInit()
{
    int ret = 0;
    for (int modules = PQ_DEMO_MEMC; modules < PQ_DEMO_MAX; modules++) {
        ret |= SetPQModuleDemoState((pq_module_demo_t)modules, (pq_module_demo_state_t)GetPQModuleDemoState(modules));
    }

    ret |= SetPQModuleDemoAisrWin((pq_module_demo_aisr_win_t)GetPQModuleDemoAisrWin());

    return ret;
}

int CPQControl::Cpq_LoadRegs(am_regs_t regs)
{
    if (regs.length == 0) {
        SYS_LOGE("%s--Regs is NULL!\n", __FUNCTION__);
        return -1;
    }

    int count_retry = 20;
    int ret = 0;
    while (count_retry) {
        ret = VPPDeviceIOCtl(AMVECM_IOC_LOAD_REG, &regs);
        if (ret < 0) {
            SYS_LOGE("%s, error(%s), errno(%d)\n", __FUNCTION__, strerror(errno), errno);
            if (errno == EBUSY) {
                SYS_LOGE("%s, %s, retry...\n", __FUNCTION__, strerror(errno));
                count_retry--;
                continue;
            }
        }
        break;
    }

    return ret;
}

int CPQControl::Cpq_LoadDisplayModeRegs(ve_pq_load_t regs)
{
    if (regs.length == 0) {
        SYS_LOGE("%s--Regs is NULL!\n", __FUNCTION__);
        return -1;
    }

    int count_retry = 20;
    int ret = 0;
    while (count_retry) {
        ret = VPPDeviceIOCtl(AMVECM_IOC_SET_OVERSCAN, &regs);
        if (ret < 0) {
            SYS_LOGE("%s, error(%s), errno(%d)\n", __FUNCTION__, strerror(errno), errno);
            if (errno == EBUSY) {
                SYS_LOGE("%s, %s, retry...\n", __FUNCTION__, strerror(errno));
                count_retry--;
                continue;
            }
        }
        break;
    }

    return ret;
}

int CPQControl::DI_LoadRegs(am_pq_param_t di_regs)
{
    int count_retry = 20;
    int ret = 0;
    while (count_retry) {
        ret = DIDeviceIOCtl(AMDI_IOC_SET_PQ_PARM, &di_regs);
        if (ret < 0) {
            SYS_LOGE("%s, error(%s), errno(%d)\n", __FUNCTION__, strerror(errno), errno);
            if (errno == EBUSY) {
                SYS_LOGE("%s, %s, retry...\n", __FUNCTION__, strerror(errno));
                count_retry--;
                continue;
            }
        }
        break;
    }

    return ret;
}

int CPQControl::LoadCpqLdimRegs()
{
    bool ret = 0;
    int ldFd = -1;

    if (!isFileExist(LDIM_PATH)) {
        SYS_LOGE("Don't have ldim module!\n");
    } else {
        ldFd = open(LDIM_PATH, O_RDWR);

        if (ldFd < 0) {
            SYS_LOGE("Open ldim module, error(%s)!\n", strerror(errno));
            ret = -1;
        } else {
            vpu_ldim_param_s *ldim_param_temp = new vpu_ldim_param_s();

            if (ldim_param_temp) {
                if (!mPQdb->PQ_GetLDIM_Regs(ldim_param_temp) || ioctl(ldFd, LDIM_IOC_PARA, ldim_param_temp) < 0) {
                   SYS_LOGE("LoadCpqLdimRegs, error(%s)!\n", strerror(errno));
                   ret = -1;
                }

                delete ldim_param_temp;
            }
                close (ldFd);
        }
    }

    return ret;
}

int CPQControl::BacklightInit(void)
{
    int ret = 0;
    int backlight = 0;
    for (int i = 1; i < 4; i++) {
        backlight = GetBacklight(i);
        SYS_LOGD("%s i = %d, backlight = %d!\n", __FUNCTION__, i, backlight);
        ret = SetBacklight(backlight, i, 1);
        if (ret != 0) {
            SYS_LOGE("%s failed!\n", __FUNCTION__);
            return ret;
        }
    }

    return ret;
}

int CPQControl::Cpq_SetDIModuleParam(source_input_param_t source_input_param)
{
    if (!mbCpqCfg_di_enable) {
        SYS_LOGD("DI module disabled!\n");
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetDIParams(source_input_param, &regs) < 0) {
        SYS_LOGE("%s GetDIParams failed!\n",__FUNCTION__);
        return -1;
    }

    if (regs.length == 0) {
        SYS_LOGE("%s: get DI Module Param failed!\n",__FUNCTION__);
        return -1;
    }

    am_pq_param_t di_regs;
    memset(&di_regs, 0x0, sizeof(am_pq_param_t));
    di_regs.table_name = TABLE_NAME_DI;
    di_regs.table_len = regs.length;
    am_reg_t tmp_buf[regs.length];
    for (unsigned int i = 0; i < regs.length; i++) {
          tmp_buf[i].addr = regs.am_reg[i].addr;
          tmp_buf[i].mask = regs.am_reg[i].mask;
          tmp_buf[i].type = regs.am_reg[i].type;
          tmp_buf[i].val  = regs.am_reg[i].val;
    }

    di_regs.table_ptr = (long long)tmp_buf;

    if (DI_LoadRegs(di_regs) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::SetPQMode(int pq_mode, int is_save , int is_autoswitch)
{
    SYS_LOGI("%s, source: %d, timming: %d, pq_mode: %d\n", __FUNCTION__, CurSource, CurTimming, pq_mode);
    int ret = -1;

    mLastPictureMode = (PICTURE_MODE)GetPQMode();
    if (mLastPictureMode == pq_mode) {
        SYS_LOGD("Same PQ mode,no need set again!\n");
        ret = 0;
        return ret;
    }

    if (is_autoswitch < PQ_MODE_SWITCH_TYPE_MANUAL || is_autoswitch >= PQ_MODE_SWITCH_TYPE_MAX) {
        is_autoswitch = PQ_MODE_SWITCH_TYPE_INIT;
    }

    if (is_save == 1) {
        SavePQMode(pq_mode);
        SaveLastPQMode((int)mLastPictureMode);
    }

    IsDvApoTypeGame = 0;
    ret = Set_PictureMode((PICTURE_MODE)pq_mode, (pq_mode_switch_type_t)is_autoswitch);

    if (is_save == 1) {
        if ((CurSource >= PQ_SRC_HDMI1) && (CurSource <= PQ_SRC_HDMI4)) {
            ret = SetDisplayMode((vpp_display_mode_t)GetDisplayMode(), 0);
        }
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::GetPQMode(void)
{
    int PictureMode = PICTURE_MODE_STANDARD;
    PICTURE_MODE_DEFAULT params;
    if (!GetPictureMode(&params)) {
        SYS_LOGE("%s: GetPictureMode fail \n", __FUNCTION__);
        return PictureMode;
    }

    switch (mCurrentHdrType) {
        case HDR_TYPE_DOVI:
            PictureMode = params.Picture_AMDOLBY;
            break;
        case HDR_TYPE_HDR10:
            PictureMode = params.Picture_HDR;
            break;
        case HDR_TYPE_HDR10PLUS:
            PictureMode = params.Picture_HDRP;
            break;
        case HDR_TYPE_HLG:
            PictureMode = params.Picture_HLG;
            break;
        case HDR_TYPE_SDR:
            PictureMode = params.Picture;
            break;
        default:
            break;
    }

    if (IsDvApoTypeGame) {
        PictureMode = PICTURE_MODE_GAME;
    } else if (mbFilmmakerModeFlag) {
        PictureMode = PICTURE_MODE_FILMMAKER;
    }

    return PictureMode;
}

int CPQControl::SavePQMode(int pq_mode)
{
    PICTURE_MODE_DEFAULT params;
    if (GetPictureMode(&params) != true) {
        SYS_LOGE("%s: GetPictureMode fail \n", __FUNCTION__);
        return -1;
    }

    switch (mCurrentHdrType) {
        case HDR_TYPE_DOVI:
            params.Picture_AMDOLBY = pq_mode;
            break;
        case HDR_TYPE_HDR10:
            params.Picture_HDR = pq_mode;
            break;
        case HDR_TYPE_HDR10PLUS:
            params.Picture_HDRP = pq_mode;
            break;
        case HDR_TYPE_HLG:
            params.Picture_HLG = pq_mode;
            break;
        case HDR_TYPE_SDR:
            params.Picture = pq_mode;
            break;
        default:
            params.Picture = pq_mode;
            break;
    }

    if (SetPictureMode(&params) != true) {
        SYS_LOGE("%s: SetPictureMode fail\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s: success\n", __FUNCTION__);
    return 0;
}

int CPQControl::GetLastPQMode(void)
{
    int PictureMode = PICTURE_MODE_STANDARD;
    PICTURE_MODE_DEFAULT params;
    if (!GetLastPictureMode(&params)) {
        SYS_LOGE("%s: GetLastPictureMode fail \n", __FUNCTION__);
        return PictureMode;
    }

    switch (mCurrentHdrType) {
        case HDR_TYPE_DOVI:
            PictureMode = params.Picture_AMDOLBY;
            break;
        case HDR_TYPE_HDR10:
            PictureMode = params.Picture_HDR;
            break;
        case HDR_TYPE_HDR10PLUS:
            PictureMode = params.Picture_HDRP;
            break;
        case HDR_TYPE_HLG:
            PictureMode = params.Picture_HLG;
            break;
        case HDR_TYPE_SDR:
            PictureMode = params.Picture;
            break;
        default:
            break;
    }

    return PictureMode;
}

int CPQControl::SaveLastPQMode(int pq_mode)
{
    PICTURE_MODE_DEFAULT params;
    if (GetLastPictureMode(&params) != true) {
        SYS_LOGE("%s: GetLastPictureMode fail \n", __FUNCTION__);
        return -1;
    }

    switch (mCurrentHdrType) {
        case HDR_TYPE_DOVI:
            params.Picture_AMDOLBY = pq_mode;
            break;
        case HDR_TYPE_HDR10:
            params.Picture_HDR = pq_mode;
            break;
        case HDR_TYPE_HDR10PLUS:
            params.Picture_HDRP = pq_mode;
            break;
        case HDR_TYPE_HLG:
            params.Picture_HLG = pq_mode;
            break;
        case HDR_TYPE_SDR:
            params.Picture = pq_mode;
            break;
        default:
            params.Picture = pq_mode;
            break;
    }

    if (SetLastPictureMode(&params) != true) {
        SYS_LOGE("%s: SetLastPictureMode fail\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s: success\n", __FUNCTION__);
    return 0;
}


#ifdef SUPPORT_TVSERVICE
static Mutex amLock;
static sp<TvServerHidlClient> mTvService = nullptr;
static const sp<TvServerHidlClient> &getTvService()
{
    Mutex::Autolock _l(amLock);
    if (mTvService == nullptr) {
        mTvService = sp<TvServerHidlClient>::make(CONNECT_TYPE_HAL);
    }

    return mTvService;
}
#endif

int CPQControl::setPQModeByTvService(pq_status_update_e gameStatus, pq_status_update_e pcStatus, int autoSwitchMonitorModeFlag)
{
    SYS_LOGD("%s: gameStatus: %d, pcStatus: %d!\n", __FUNCTION__, gameStatus, pcStatus);

    int ret = -1;
#ifdef SUPPORT_TVSERVICE
    const sp<TvServerHidlClient> &TvService = getTvService();
    if ( TvService == NULL) {
        SYS_LOGE("%s: get tvservice failed!\n", __FUNCTION__);
    } else {
        ret = TvService->vdinUpdateForPQ(gameStatus, pcStatus, autoSwitchMonitorModeFlag);
    }
#else
    SYS_LOGD("%s: don't support tvservice!\n", __FUNCTION__);
    ret = 0;
#endif

    if (ret < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n", __FUNCTION__);
    }
    return ret;
}

//color temperature
int CPQControl::SetColorTemperature(int temp_mode, int is_save)
{
    int ret = 0;
    SYS_LOGI("%s: source:%d, mode: %d\n", __FUNCTION__, mCurrentSourceInputInfo.source_input, temp_mode);

    if (is_save == 1) {
        SaveColorTemperature(temp_mode);
    }

    ret |= Cpq_SetColorTemperature(temp_mode);
    ret |= Cpq_LoadGamma((vpp_gamma_mode_t)GetGammaValue(), (vpp_color_temperature_mode_t)temp_mode);

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetColorTemperature(void)
{
    int mode = COLOR_TMP_MODE_STANDARD;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return mode;
    }

    mode = para.ColorTemperature;

    if (mode < COLOR_TMP_MODE_STANDARD || mode >= COLOR_TMP_MODE_MAX) {
        mode = COLOR_TMP_MODE_STANDARD;
    }

    SYS_LOGD("%s: source: %d, timming: %d mode: %d!\n",__FUNCTION__, CurSource, CurTimming, mode);
    return mode;
}

int CPQControl::SaveColorTemperature(int temp_mode)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, temp_mode);
        return -1;
    }

    para.ColorTemperature = temp_mode;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, temp_mode);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetColorTemperature(int level)
{
    if (!mbCpqCfg_whitebalance_enable) {
        SYS_LOGD("%s ColorTemperature disabled!\n",__FUNCTION__);
        return 0;
    }

    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, level)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n",__FUNCTION__);
        return -1;
    }

    if (FactoryGetWhitebalanceRGBGainOffsetData(&ColorTemp.rgbgo, level)) {
        SYS_LOGD("%s Get ColorTemp from CRI_DATA\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s Get ColorTemp from TV_PICTURE\n",__FUNCTION__);
    }

    tcon_rgb_ogo_t rgbogo;
    memset(&rgbogo, 0, sizeof(tcon_rgb_ogo_t));
    rgbogo.en = 1;
    rgbogo.r_gain = ColorTemp.rgbgo.RGAIN + ColorTemp.ColorTempOffset.r_gain_value;
    rgbogo.g_gain = ColorTemp.rgbgo.GGAIN + ColorTemp.ColorTempOffset.g_gain_value;
    rgbogo.b_gain = ColorTemp.rgbgo.BGAIN + ColorTemp.ColorTempOffset.b_gain_value;
    rgbogo.r_post_offset = ColorTemp.rgbgo.ROFFSET + ColorTemp.ColorTempOffset.r_offset_value;
    rgbogo.g_post_offset = ColorTemp.rgbgo.GOFFSET + ColorTemp.ColorTempOffset.g_offset_value;
    rgbogo.b_post_offset = ColorTemp.rgbgo.BOFFSET + ColorTemp.ColorTempOffset.b_offset_value;

    if (GetEyeProtectionMode(mCurrentSourceInputInfo.source_input))//if eye protection mode is enable, b_gain / 2.
        rgbogo.g_gain /= 2;

    if (Cpq_SetRGBOGO(&rgbogo) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("level       = %5d, Enable      = %5d\n"
             "rgain       = %5d, ggain       = %5d, bgain       = %5d\n"
             "roffset     = %5d, goffset     = %5d, boffset     = %5d\n"
             "rpre_offset = %5d, gpre_offset = %5d, bpre_offset = %5d\n",
             level, rgbogo.en,
             rgbogo.r_gain, rgbogo.g_gain, rgbogo.b_gain,
             rgbogo.r_post_offset, rgbogo.g_post_offset, rgbogo.b_post_offset,
             rgbogo.r_pre_offset, rgbogo.g_pre_offset, rgbogo.b_pre_offset);

    return 0;
}

int CPQControl::SetColorTemperatureUserParam(int temp_mode, int is_save, rgb_ogo_type_t rgb_ogo_type, int value)
{
    SYS_LOGD("%s: temp_mode: %d, rgb_ogo_type: %d value %d, is_save: %d\n", __FUNCTION__, temp_mode, rgb_ogo_type, value, is_save);
    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, temp_mode)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n", __FUNCTION__);
        return -1;
    }

    switch (rgb_ogo_type)
    {
        case R_GAIN:
            ColorTemp.ColorTempOffset.r_gain_value = value;
        break;
        case G_GAIN:
            ColorTemp.ColorTempOffset.g_gain_value = value;
        break;
        case B_GAIN:
            ColorTemp.ColorTempOffset.b_gain_value = value;
        break;
        case R_POST_OFFSET:
            ColorTemp.ColorTempOffset.r_offset_value = value;
        break;
        case G_POST_OFFSET:
            ColorTemp.ColorTempOffset.g_offset_value = value;
        break;
        case B_POST_OFFSET:
            ColorTemp.ColorTempOffset.b_offset_value = value;
        break;
        default:
            SYS_LOGE("%s rgb_ogo_type out of range! fail\n", __FUNCTION__);
            return -1;
        break;
    }

    if (is_save == 1) {
        if (!SetColorTemperatureData(&ColorTemp, temp_mode)) {
            SYS_LOGE("%s SetColorTemperatureData fail\n", __FUNCTION__);
            return -1;
        }
    }

    if (Cpq_SetColorTemperature(temp_mode) < 0) {
        SYS_LOGE("%s Cpq_SetColorTemperature failed!\n",__FUNCTION__);
        return -1;
    }

    return 0;
}

tcon_rgb_ogo_t CPQControl::GetColorTemperatureUserParam(void)
{
    tcon_rgb_ogo_t param;
    memset(&param, 0, sizeof(tcon_rgb_ogo_t));

    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, GetColorTemperature())) {
        SYS_LOGE("%s GetColorTemperatureData fail\n", __FUNCTION__);
        return param;
    }

    param.en = 1;
    param.r_gain = ColorTemp.ColorTempOffset.r_gain_value;
    param.g_gain = ColorTemp.ColorTempOffset.g_gain_value;
    param.b_gain = ColorTemp.ColorTempOffset.b_gain_value;
    param.r_post_offset = ColorTemp.ColorTempOffset.r_offset_value;
    param.g_post_offset = ColorTemp.ColorTempOffset.g_offset_value;
    param.b_post_offset = ColorTemp.ColorTempOffset.b_offset_value;

    SYS_LOGD("%s: rgo[%d] ggo[%d] bgo[%d] roo[%d] goo[%d] boo[%d].\n",
            __FUNCTION__,
             param.r_gain, param.g_gain, param.b_gain,
             param.r_post_offset, param.g_post_offset, param.b_post_offset);

    return param;
}

int CPQControl::SetColorTemperatureParams(vpp_color_temperature_mode_t Tempmode, tcon_rgb_ogo_t params)
{
    COLORTEMP_DATA pData;
    if (!GetColorTemperatureData(&pData, (int)Tempmode)) {
        SYS_LOGE("%s, GetColorTemperatureData error.\n", __FUNCTION__);
        return -1;
    }

    pData.rgbgo.RGAIN = params.r_gain;
    pData.rgbgo.GGAIN = params.g_gain;
    pData.rgbgo.BGAIN = params.b_gain;
    pData.rgbgo.ROFFSET = params.r_post_offset;
    pData.rgbgo.GOFFSET = params.g_post_offset;
    pData.rgbgo.BOFFSET = params.b_post_offset;

    if (!SetColorTemperatureData(&pData, (int)Tempmode)) {
        SYS_LOGE("%s, SetColorTemperatureData error.\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::GetColorTemperatureParams(vpp_color_temperature_mode_t Tempmode, tcon_rgb_ogo_t *params)
{
    if (params == NULL) {
        return -1;
    }

    COLORTEMP_DATA pData;
    if (!GetColorTemperatureData(&pData, (int)Tempmode)) {
        SYS_LOGE("%s, GetColorTemperatureData error.\n", __FUNCTION__);
        return -1;
    }

    params->en = 1;
    params->r_gain = pData.rgbgo.RGAIN;
    params->g_gain = pData.rgbgo.GGAIN;
    params->b_gain = pData.rgbgo.BGAIN;
    params->r_post_offset = pData.rgbgo.ROFFSET;
    params->g_post_offset = pData.rgbgo.GOFFSET;
    params->b_post_offset = pData.rgbgo.BOFFSET;
    params->r_pre_offset = 0;
    params->g_pre_offset = 0;
    params->b_pre_offset = 0;

    SYS_LOGD("%s, Tempmode:%d rgain[%d], ggain[%d],bgain[%d],roffset[%d],goffset[%d],boffset[%d]\n", __FUNCTION__, Tempmode,
         params->r_gain, params->g_gain, params->b_gain, params->r_post_offset,
         params->g_post_offset, params->b_post_offset);

    return 0;
}

//Brightness
int CPQControl::SetBrightness(int value, int is_save)
{
    int ret = 0;
    SYS_LOGI("%s, source: %d, value = %d\n", __FUNCTION__, CurSource, value);

    ret = Cpq_SetBrightness(value, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveBrightness(value);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return 0;
}

int CPQControl::GetBrightness(void)
{
    int data = 50;
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, (PICTURE_MODE)GetPQMode())) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d data: %d fail\n",__FUNCTION__, CurSource, CurTimming, data);
        return data;
    }

    data = para.Brightness;

    if (data < 0 || data > 100) {
        data = 50;
    }

    SYS_LOGI("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, data);
    return data;
}

int CPQControl::SaveBrightness(int value)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    para.Brightness = value;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetBrightness(int value, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_amvecm_basic_enable && !mbCpqCfg_amvecm_basic_withOSD_enable) {
        SYS_LOGD("%s: brightness module disabled!\n", __FUNCTION__);
        return 0;
    }

    if (value < 0 || value > 100) {
        SYS_LOGE("%s: out off range!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetVideoBrightness(GetNonlinearOsdRemapVal(_BRIGHTNESS, value)) < 0) {
        SYS_LOGE("Cpq_SetVideoBrightness failed!\n");
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetVideoBrightness(int value)
{
    SYS_LOGD("Cpq_SetVideoBrightness brightness : %d", value);
    int data = (((value == 255) ? 256 : value) * (BRIGHTNESS_MAX - BRIGHTNESS_MIN) / 256) - BRIGHTNESS_MAX; //-512 ~ 512
    if (data > BRIGHTNESS_MAX) data = BRIGHTNESS_MAX;
    if (data < BRIGHTNESS_MIN) data = BRIGHTNESS_MIN;

    SYS_LOGD("to Driver brightness : %d", data);

    am_pic_mode_t params;
    memset(&params, 0, sizeof(params));

    if (mbCpqCfg_amvecm_basic_enable) {
        params.flag |= 0x1;
        params.brightness = data;
    }

    if (mbCpqCfg_amvecm_basic_withOSD_enable) {
        params.flag |= (0x1 << 1);
        params.brightness2 = data;
    }

    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_PIC_MODE, &params);
    if (ret < 0) {
        SYS_LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

//Contrast
int CPQControl::SetContrast(int value, int is_save)
{
    int ret = 0;
    SYS_LOGI("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, value);
    ret = Cpq_SetContrast(value, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveContrast(value);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetContrast(void)
{
    int data = 50;
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, (PICTURE_MODE)GetPQMode())) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d data: %d fail\n",__FUNCTION__, CurSource, CurTimming, data);
        return data;
    }

    data = para.Contrast;

    if (data < 0 || data > 100) {
        data = 50;
    }

    SYS_LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, data);
    return data;
}

int CPQControl::SaveContrast(int value)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    para.Contrast = value;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetContrast(int value, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_amvecm_basic_enable && !mbCpqCfg_amvecm_basic_withOSD_enable) {
        SYS_LOGD("%s:  contrast module disabled!\n", __FUNCTION__);
        return 0;
    }

    if (value < 0 || value > 100) {
        SYS_LOGE("%s: out off range!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetVideoContrast(GetNonlinearOsdRemapVal(_CONTRAST, value)) < 0) {
        SYS_LOGE("%s: Cpq_SetVideoContrast failed!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetVideoContrast(int value)
{
    SYS_LOGD("Cpq_SetVideoContrast: %d", value);

    int data = ((((value == 255) ? 256 : value) * ((CONTRAST_MAX + 1) - CONTRAST_MIN)) / 256) - (CONTRAST_MAX + 1);
    if (data > CONTRAST_MAX) data = CONTRAST_MAX;
    if (data < CONTRAST_MIN) data = CONTRAST_MIN;

    SYS_LOGD("to Driver Contrast: %d", data);

    am_pic_mode_t params;
    memset(&params, 0, sizeof(params));

    if (mbCpqCfg_amvecm_basic_enable) {
        params.flag |= (0x1 << 4);
        params.contrast = data;
    }

    if (mbCpqCfg_amvecm_basic_withOSD_enable) {
        params.flag |= (0x1 << 5);
        params.contrast2 = data;
    }

    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_PIC_MODE, &params);
    if (ret < 0) {
        SYS_LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

//Saturation
int CPQControl::SetSaturation(int value, int is_save)
{
    int ret = 0;
    SYS_LOGI("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, value);
    ret = Cpq_SetSaturation(value, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveSaturation(value);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetSaturation(void)
{
    int data = 50;
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, (PICTURE_MODE)GetPQMode())) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d data: %d fail\n",__FUNCTION__, CurSource, CurTimming, data);
        return data;
    }

    data = para.Saturation;

    if (data < 0 || data > 100) {
        data = 50;
    }

    SYS_LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, data);
    return data;
}

int CPQControl::SaveSaturation(int value)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    para.Saturation = value;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetSaturation(int value, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_amvecm_basic_enable && !mbCpqCfg_amvecm_basic_withOSD_enable) {
        SYS_LOGD("%s: saturation module disabled!\n", __FUNCTION__);
        return 0;
    }

    if (value < 0 || value > 100) {
        SYS_LOGD("%s: out off range!\n", __FUNCTION__);
        return -1;
    }

    int saturation = GetNonlinearOsdRemapVal(_SATURATION, value);
    int hue        = GetNonlinearOsdRemapVal(_HUE, GetHue());
    if (Cpq_SetVideoSaturationHue(saturation, hue) < 0) {
        SYS_LOGE("%s: Cpq_SetVideoSaturationHue failed!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

//Hue
int CPQControl::SetHue(int value, int is_save)
{
    int ret = 0;
    SYS_LOGI("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, value);
    ret = Cpq_SetHue(value, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveHue(value);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetHue(void)
{
    int data = 50;
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, (PICTURE_MODE)GetPQMode())) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d data: %d fail\n",__FUNCTION__, CurSource, CurTimming, data);
        return data;
    }

    data = para.Hue;

    if (data < 0 || data > 100) {
        data = 50;
    }

    SYS_LOGD("%s, source: %d, timming: %d value = %d\n", __FUNCTION__, CurSource, CurTimming, data);
    return data;
}

int CPQControl::SaveHue(int value)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    para.Hue = value;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetHue(int value, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_amvecm_basic_enable && !mbCpqCfg_amvecm_basic_withOSD_enable) {
        SYS_LOGD("%s: hue module disabled!\n", __FUNCTION__);
        return 0;
    }

    if (value < 0 || value > 100) {
        SYS_LOGE("%s: value out off range!\n", __FUNCTION__);
        return -1;
    }

    int saturation_params = GetNonlinearOsdRemapVal(_SATURATION, GetSaturation());
    int hue_params        = GetNonlinearOsdRemapVal(_HUE, value);
    if (Cpq_SetVideoSaturationHue(saturation_params, hue_params) < 0) {
        SYS_LOGE("Cpq_SetVideoSaturationHue failed!\n");
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetVideoSaturationHue(int satVal, int hueVal)
{
    SYS_LOGD("%s, Cpq_SetVideoSaturationHue: %d %d", __FUNCTION__, satVal, hueVal);
    int data_hue = ((((hueVal == 255) ? 256 : hueVal) * (HUE_MAX - HUE_MIN)) / 256) - HUE_MAX;
    if (data_hue > HUE_MAX) data_hue = HUE_MAX;
    if (data_hue < HUE_MIN) data_hue = HUE_MIN;

    int data_sat = (((satVal == 255) ? 256 : satVal) * ((SATURATION_MAX + 1) - SATURATION_MIN) / 256) - (SATURATION_MAX + 1);
    if (data_sat > SATURATION_MAX) data_sat = SATURATION_MAX;
    if (data_sat < SATURATION_MIN) data_sat = SATURATION_MIN;

    SYS_LOGD("%s: to Driver Saturation = %d, Hue = %d",__FUNCTION__, data_sat, data_hue);

    signed long temp;
    am_pic_mode_t params;
    memset(&params, 0, sizeof(params));
    video_set_saturation_hue(data_sat, data_hue, &temp);

    if (mbCpqCfg_amvecm_basic_enable) {
        params.flag |= (0x1 << 2);
        params.saturation_hue = temp;
    }

    if (mbCpqCfg_amvecm_basic_withOSD_enable) {
        params.flag |= (0x1 << 3);
        params.saturation_hue_post = temp;
    }

    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_PIC_MODE, &params);
    if (ret < 0) {
        SYS_LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

void CPQControl::video_set_saturation_hue(signed char saturation, signed char hue, signed long *mab)
{
    signed short ma = (signed short) (cos((float) hue * PI / 128.0) * ((float) saturation / 128.0 + 1.0) * 256.0);
    signed short mb = (signed short) (sin((float) hue * PI / 128.0) * ((float) saturation / 128.0 + 1.0) * 256.0);

    if (ma >  511) ma =  511;
    if (ma < -512) ma = -512;
    if (mb >  511) mb =  511;
    if (mb < -512) mb = -512;

    *mab = ((ma & 0x3ff) << 16) | (mb & 0x3ff);
}

void CPQControl::video_get_saturation_hue(signed char *sat, signed char *hue, signed long *mab)
{
    signed long temp = *mab;
    signed int ma = (signed int) ((temp << 6) >> 22);
    signed int mb = (signed int) ((temp << 22) >> 22);
    signed int sat16 = (signed int) ((sqrt(((float) ma * (float) ma + (float) mb * (float) mb) / 65536.0) - 1.0) * 128.0);
    signed int hue16 = (signed int) (atan((float) mb / (float) ma) * 128.0 / PI);

    if (sat16 >  127) sat16 =  127;
    if (sat16 < -128) sat16 = -128;
    if (hue16 >  127) hue16 =  127;
    if (hue16 < -128) hue16 = -128;

    *sat = (signed char) sat16;
    *hue = (signed char) hue16;
}

//sharpness
int CPQControl::SetSharpness(int value, int is_enable __unused, int is_save)
{
    SYS_LOGI("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, value);
    int ret = Cpq_SetSharpness(value, mCurrentSourceInputInfo);

    if ((ret== 0) && (is_save == 1)) {
        ret = SaveSharpness(value);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetSharpness(void)
{
    int data = 50;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d data: %d fail\n",__FUNCTION__, CurSource, CurTimming, data);
        return data;
    }

    data = para.Sharpness;

    if (data < 0 || data > 100) {
        data = 50;
    }

    SYS_LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, data);
    return data;
}

int CPQControl::SaveSharpness(int value)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    para.Sharpness = value;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetSharpness(int value, source_input_param_t source_input_param)
{
    int ret = 0;
    int sharpness = GetNonlinearOsdRemapVal(_SHARPNESS, value);
    ret |= Cpq_SetSharpness0Level(sharpness, source_input_param);
    ret |= Cpq_SetSharpness1Level(sharpness, source_input_param);
    ret |= Cpq_SetSharpnessPiLevel(sharpness, source_input_param);

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetSharpness0Level(int value, source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        SYS_LOGE("%s: pq.db don't match chip!\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpness0_enable) {
        SYS_LOGD("%s: sharpness0 module disabled!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetSharpness0Params(source_input_param, value, &regs) < 0) {
        SYS_LOGE("%s: PQ_GetSharpness0Params failed!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n", __FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetSharpness1Level(int value, source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        SYS_LOGE("%s: pq.db don't match chip!\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpness1_enable) {
        SYS_LOGD("%s: sharpness1 module disabled!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetSharpness1Params(source_input_param, value, &regs) < 0) {
        SYS_LOGE("%s: PQ_GetSharpness1Params failed!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n", __FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetSharpnessPiLevel(int value, source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        SYS_LOGE("%s: pq.db don't match chip!\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpnesspi_enable) {
        SYS_LOGD("%s: sharpnesspi module disabled!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetSharpnessPiParams(source_input_param, value, &regs) < 0) {
        SYS_LOGE("%s: PQ_GetSharpnessPiParams failed!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n", __FUNCTION__);
    return 0;
}

int CPQControl::SetOsdSharpness(bool enable, int is_save)
{
    int ret = 0;
    SYS_LOGI("%s, source: %d, value = %d\n", __FUNCTION__, CurSource, enable);

    ret = Cpq_SetOsdSharpness(enable);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveOsdSharpness(enable);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return 0;
}

bool CPQControl::GetOsdSharpness(void)
{
    int data = 0;
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return data;
    }

    data = pData.osd_sharpness;

    if (data < 0 || data > 1) {
        data = 0;
    }

    SYS_LOGI("%s, data:%d\n", __FUNCTION__, data);
    return (bool)data;
}

int CPQControl::SaveOsdSharpness(bool enable)
{
    SYS_LOGI("%s, SaveOsdSharpness enable:%d\n", __FUNCTION__, enable);
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    pData.osd_sharpness = enable ? 1 : 0;

    if (!SetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s SetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetOsdSharpness(bool enable)
{
    if (!mbCpqCfg_osd_sharpness_enable) {
        SYS_LOGD("%s: osd sharpness module disabled!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;

    memset(&regs, 0x0, sizeof(am_regs_t));

    if (mPQExtdb->PQ_GetOsdSharpnessParams(enable ? 1 : 0, mCurrentSourceInputInfo, &regs) < 0) {
        SYS_LOGE("%s PQ_GetOsdSharpnessParams failed!\n", __FUNCTION__);
        return -1;
    }

    for (int i = 0; i < regs.length; i++) {
        SYS_LOGD("%s: am_reg[%d]: %d, %d, %x, %x\n", __FUNCTION__,
            i,
            regs.am_reg[i].type, regs.am_reg[i].addr, regs.am_reg[i].mask, regs.am_reg[i].val);
    }

    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s: Cpq_LoadRegs failed!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

//SuperResolution
int CPQControl::SetSuperResolution(int value, int is_save)
{
    if (is_save) {
        SaveSharpness(value);
    }

    if (Cpq_SetSuperResolution(value, mCurrentSourceInputInfo) < 0) {
        SYS_LOGE("%s: fail\n", __FUNCTION__, value);
        return -1;
    }

    SYS_LOGD("%s: success! value: %d, is_save: %d\n", __FUNCTION__, value, is_save);
    return 0;
}

int CPQControl::GetSuperResolution(void)
{
    int value = VPP_PQ_LV_OFF;
    PICTURE_MODE_DATA params;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&params, pq_mode)) {
        SYS_LOGE("%s: fail\n",__FUNCTION__);
        return value;
    }

    value = params.SuperResolution;

    if (value < VPP_PQ_LV_OFF || value > VPP_PQ_LV_MAX) {
        value = VPP_PQ_LV_OFF;
    }

    SYS_LOGD("%s: success value: %d\n", __FUNCTION__, value);
    return value;
}

int CPQControl::SaveSuperResolution(int value)
{
    PICTURE_MODE_DATA params;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&params, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData fail\n",__FUNCTION__);
        return -1;
    }

    params.SuperResolution = value;

    if (!SetPictureModeData(&params, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData fail\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s: success SuperResolution = %d\n", __FUNCTION__, value);
    return 0;
}

int CPQControl::Cpq_SetSuperResolution(int value, source_input_param_t source_input_param)
{
    int ret = 0;
    ret |= Cpq_SetSharpness0FixedParam(value, source_input_param);
    ret |= Cpq_SetSharpness1FixedParam(value, source_input_param);
    ret |= Cpq_SetSharpnessPiFixedParam(value, source_input_param);

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetSharpness0FixedParam(int value, source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        SYS_LOGE("%s: pq.db don't match chip!\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpness0_enable) {
        SYS_LOGD("%s: sharpness0 module disabled!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetSharpness0FixedParams(source_input_param, &regs) < 0) {
        SYS_LOGE("%s: PQ_GetSharpness0FixedParams failed!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n", __FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetSharpness1FixedParam(int value, source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        SYS_LOGE("%s: pq.db don't match chip!\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpness1_enable) {
        SYS_LOGD("%s: sharpness1 module disabled!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetSharpness1FixedParams(source_input_param, &regs) < 0) {
        SYS_LOGE("%s: PQ_GetSharpness1FixedParams failed!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n", __FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetSharpnessPiFixedParam(int value, source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        SYS_LOGE("%s: pq.db don't match chip!\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpnesspi_enable) {
        SYS_LOGD("%s: sharpnesspi module disabled!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetSharpnessPiFixedParams(source_input_param, &regs) < 0) {
        SYS_LOGE("%s: PQ_GetSharpnessPiFixedParams failed!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n", __FUNCTION__);
    return 0;
}

int CPQControl::SetFacColorParams(source_input_param_t source_input_param)
{
    int ret = 0;

    ret |= Cpq_SetSharpness0VariableParam(source_input_param);
    ret |= Cpq_SetSharpness1VariableParam(source_input_param);
    ret |= Cpq_SetSharpnessPiVariableParam(source_input_param);

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetSharpness0VariableParam(source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        SYS_LOGE("%s: pq.db don't match chip!\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpness0_enable) {
        SYS_LOGD("%s: sharpness0 module disabled!\n", __FUNCTION__);
        return 0;
    }

    if (mPQdb->PQ_SetSharpness0VariableParams(source_input_param) < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n", __FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetSharpness1VariableParam(source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        SYS_LOGE("%s: pq.db don't match chip!\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpness1_enable) {
        SYS_LOGD("%s: sharpness1 module disabled!\n", __FUNCTION__);
        return 0;
    }

    if (mPQdb->PQ_SetSharpness1VariableParams(source_input_param) < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGI("%s success!\n", __FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetSharpnessPiVariableParam(source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        SYS_LOGE("%s: pq.db don't match chip!\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpnesspi_enable) {
        SYS_LOGD("%s: sharpnesspi module disabled!\n", __FUNCTION__);
        return 0;
    }

    if (mPQdb->PQ_SetSharpnessPiVariableParams(source_input_param) < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n", __FUNCTION__);
    return 0;
}

void CPQControl::InitAutoNr(void)
{
    int ret = -1;
    const char *buff = NULL;
    int buf[128] = {0};

    buff = mPQConfigFile->GetString(CFG_SECTION_AUTO_NR, CFG_AUTO_NR_MOTION_TH, NULL);
    pqTransformStringToInt(buff, buf);
    if (buff != NULL) {
        char str1[128] = {0};
        sprintf(str1, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", "motion_th",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14]);
        SYS_LOGE("%s str1 = %s\n", __FUNCTION__, str1);
        ret = pqWriteSys(AML_AUTO_NR_PARAMS, str1);
    }

    buff = mPQConfigFile->GetString(CFG_SECTION_AUTO_NR, CFG_AUTO_NR_MOTION_LP_YGAIN, NULL);
    pqTransformStringToInt(buff, buf);
    if (buff != NULL) {
        char str2[128] = {0};
        sprintf(str2, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", "motion_lp_ygain",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
        SYS_LOGE("%s str2 = %s\n", __FUNCTION__, str2);
        ret |= pqWriteSys(AML_AUTO_NR_PARAMS, str2);
    }

    buff = mPQConfigFile->GetString(CFG_SECTION_AUTO_NR, CFG_AUTO_NR_MOTION_HP_YGAIN, NULL);
    pqTransformStringToInt(buff, buf);
    if (buff != NULL) {
        char str3[128] = {0};
        sprintf(str3, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", "motion_hp_ygain",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
        SYS_LOGE("%s str3 = %s\n", __FUNCTION__, str3);
        ret |= pqWriteSys(AML_AUTO_NR_PARAMS, str3);
    }

    buff = mPQConfigFile->GetString(CFG_SECTION_AUTO_NR, CFG_AUTO_NR_MOTION_LP_CGAIN, NULL);
    pqTransformStringToInt(buff, buf);
    if (buff != NULL) {
        char str4[128] = {0};
        sprintf(str4, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", "motion_lp_cgain",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
        SYS_LOGE("%s str4 = %s\n", __FUNCTION__, str4);

        ret |= pqWriteSys(AML_AUTO_NR_PARAMS, str4);
    }

    buff = mPQConfigFile->GetString(CFG_SECTION_AUTO_NR, CFG_AUTO_NR_MOTION_HP_CGAIN, NULL);
    pqTransformStringToInt(buff, buf);
    if (buff != NULL) {
        char str5[128] = {0};
        sprintf(str5, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", "motion_hp_cgain",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
        SYS_LOGE("%s str5 = %s\n", __FUNCTION__, str5);
        ret |= pqWriteSys(AML_AUTO_NR_PARAMS, str5);
    }

    buff = mPQConfigFile->GetString(CFG_SECTION_AUTO_NR, CFG_AUTO_NR_APL_GAIN, NULL);
    pqTransformStringToInt(buff, buf);
    if (buff != NULL) {
        char str6[128] = {0};
        sprintf(str6, "%s %d %d %d %d %d %d %d %d", "apl_gain",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
        SYS_LOGE("%s str6 = %s\n", __FUNCTION__, str6);
        ret |= pqWriteSys(AML_AUTO_NR_PARAMS, str6);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed\n", __FUNCTION__);
    }

    return;
}

//NoiseReductionMode
int CPQControl::SetNoiseReductionMode(int nr_mode, int is_save)
{
    SYS_LOGI("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, nr_mode);
    int ret = Cpq_SetNoiseReductionMode((vpp_noise_reduction_mode_t)nr_mode, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveNoiseReductionMode((vpp_noise_reduction_mode_t)nr_mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetNoiseReductionMode(void)
{
    int mode = VPP_NOISE_REDUCTION_MODE_MID;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return mode;
    }

    mode = para.Nr;

    if (mode < VPP_NOISE_REDUCTION_MODE_OFF || mode > VPP_NOISE_REDUCTION_MODE_AUTO) {
        mode = VPP_NOISE_REDUCTION_MODE_MID;
    }

    SYS_LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, mode);
    return mode;
}

int CPQControl::SaveNoiseReductionMode(int nr_mode)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, nr_mode);
        return -1;
    }

    para.Nr = nr_mode;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, nr_mode);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetNoiseReductionMode(vpp_noise_reduction_mode_t nr_mode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_nr_enable) {
        SYS_LOGD("%s: NoiseReduction module disabled!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetNR2Params((vpp_noise_reduction_mode_t)nr_mode, source_input_param, &regs) < 0) {
        SYS_LOGE("PQ_GetNR2Params failed!\n");
        return -1;
    }

    am_pq_param_t di_regs;
    memset(&di_regs, 0x0, sizeof(am_pq_param_t));
    di_regs.table_name = TABLE_NAME_NR;
    di_regs.table_len = regs.length;
    am_reg_t tmp_buf[regs.length];
    for (unsigned int i = 0; i < regs.length; i++) {
          tmp_buf[i].addr = regs.am_reg[i].addr;
          tmp_buf[i].mask = regs.am_reg[i].mask;
          tmp_buf[i].type = regs.am_reg[i].type;
          tmp_buf[i].val  = regs.am_reg[i].val;
    }
    di_regs.table_ptr = (long long)tmp_buf;

    if (DI_LoadRegs(di_regs) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

//MultipointGamma
int CPQControl::SetMultipointGamma(int channel, int point, int offset)
{
    SYS_LOGD("%s, channel = %d, point = %d, offset = %d\n", __FUNCTION__, channel, point, offset);
    if (channel >= MAX_CH) {
        SYS_LOGE("%s, channel = %d, out of range\n", __FUNCTION__, channel);
        return -1;
    }

    if (point >= MAX_WB_GAMMA_POINT) {
        SYS_LOGE("%s, point = %d, out of range\n", __FUNCTION__, point);
        return -1;
    }

    int colortemp = GetColorTemperature();
    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, colortemp)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n",__FUNCTION__);
        return -1;
    }

    if (channel == RED_CH)
        ColorTemp.GammaOffset.Gamma.R_offset[point] = offset;
    else if (channel == GREEN_CH)
        ColorTemp.GammaOffset.Gamma.G_offset[point] = offset;
    else if (channel == BLUE_CH)
        ColorTemp.GammaOffset.Gamma.B_offset[point] = offset;
    else
        return -1;

    if (!SetColorTemperatureData(&ColorTemp, colortemp)) {
        SYS_LOGE("%s SetColorTemperatureData fail\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetMultipointGamma((vpp_gamma_mode_t)GetGammaValue(), (TEMP_MODE)colortemp) < 0) {
        SYS_LOGE("%s, Cpq_SetMultipointGamma fail\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::GetMultipointGamma(int channel, int point)
{
    int offset = 0;
    if (channel >= MAX_CH) {
        SYS_LOGE("%s, channel = %d, out of range\n", __FUNCTION__, channel);
        return offset;
    }

    if (point >= MAX_WB_GAMMA_POINT) {
        SYS_LOGE("%s, point = %d, out of range\n", __FUNCTION__, point);
        return offset;
    }

    int colortemp = GetColorTemperature();
    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, colortemp)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n",__FUNCTION__);
        return -1;
    }

    if (channel == RED_CH)
        offset = ColorTemp.GammaOffset.Gamma.R_offset[point];
    else if (channel == GREEN_CH)
        offset = ColorTemp.GammaOffset.Gamma.G_offset[point];
    else if (channel == BLUE_CH)
        offset = ColorTemp.GammaOffset.Gamma.B_offset[point];
    else
        offset =  0;

    return offset;
}

int CPQControl::Cpq_SetMultipointGamma(vpp_gamma_mode_t gamma_curve, TEMP_MODE colortemp_mode)
{
    if (!mInitialized) {
        return 0;
    }

    if (!mbCpqCfg_gamma_enable) {
        SYS_LOGD("Gamma module disabled!\n");
        return 0;
    }

    int ret = 0;
    GAMMA_TABLE Gamma;
    memset(&Gamma, 0, sizeof(GAMMA_TABLE));
    ret |= GetBaseGammaData((int)colortemp_mode, &Gamma);
    ret |= GetMultipointWBGammaData((int)colortemp_mode, &Gamma);
    ret |= GetGammaPowerData((int)gamma_curve, &Gamma);

    // to driver
    if (ret < 0) {
        SYS_LOGE("%s, mixed Gamma failed!\n", __FUNCTION__);
    } else {
        ret |= Cpq_SetGammaTbl_R((unsigned short *)Gamma.R.data);
        ret |= Cpq_SetGammaTbl_G((unsigned short *)Gamma.G.data);
        ret |= Cpq_SetGammaTbl_B((unsigned short *)Gamma.B.data);
    }

    return ret;
}

int CPQControl::FactorySetMultipointGamma(int colortemp, int channel, int point, int offset)
{
    if (channel >= MAX_CH) {
        SYS_LOGE("%s, channel = %d, out of range\n", __FUNCTION__, channel);
        return -1;
    }

    if (point >= MAX_WB_GAMMA_POINT) {
        SYS_LOGE("%s, point = %d, out of range\n", __FUNCTION__, point);
        return -1;
    }

    CheckCriDataMultipointGammaData();

    Multipoint_GAMMA_DATA pData;
    if (!FactoryGetMultipointGammaData(&pData, colortemp)) {
        SYS_LOGE("%s, FactoryGetMultipointGammaData fail\n", __FUNCTION__);
        return -1;
    }

    if (channel == RED_CH)
        pData.R_offset[point] = offset;
    else if (channel == GREEN_CH)
        pData.G_offset[point] = offset;
    else if (channel == BLUE_CH)
        pData.B_offset[point] = offset;
    else
        return -1;

    if (!FactorySetMultipointGammaData(&pData, colortemp)) {
        SYS_LOGE("%s, FactorySetMultipointGammaData fail\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadGamma((vpp_gamma_mode_t)GetGammaValue(), (vpp_color_temperature_mode_t)colortemp) < 0) {
        SYS_LOGE("%s, Cpq_LoadGamma fail\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetMultipointGamma(int colortemp, int channel, int point)
{
    int offset = 0;
    if (channel >= MAX_CH) {
        SYS_LOGE("%s, channel = %d, out of range\n", __FUNCTION__, channel);
        return offset;
    }

    if (point >= MAX_WB_GAMMA_POINT) {
        SYS_LOGE("%s, point = %d, out of range\n", __FUNCTION__, point);
        return offset;
    }

    CheckCriDataMultipointGammaData();

    Multipoint_GAMMA_DATA pData;
    if (!FactoryGetMultipointGammaData(&pData, colortemp)) {
        SYS_LOGE("%s, FactoryGetMultipointGammaData fail\n", __FUNCTION__);
        return -1;
    }

    if (channel == RED_CH)
        offset = pData.R_offset[point];
    else if (channel == GREEN_CH)
        offset = pData.G_offset[point];
    else if (channel == BLUE_CH)
        offset = pData.B_offset[point];
    else
        offset =  0;

    return offset;
}

int CPQControl::SetMultipointGammaEnable(int enable)
{
    int level = GetColorTemperature();
    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, level)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n", __FUNCTION__);
        return -1;
    }

    if (ColorTemp.GammaOffset.enable != enable) {
        ColorTemp.GammaOffset.enable = enable;
        if (!SetColorTemperatureData(&ColorTemp, level)) {
            SYS_LOGE("%s SetColorTemperatureData fail\n", __FUNCTION__);
            return -1;
        }
    } else {
        SYS_LOGD("%s same status : %d\n", __FUNCTION__, enable);
        return 0;
    }

    if (Cpq_LoadGamma((vpp_gamma_mode_t)GetGammaValue(), (vpp_color_temperature_mode_t)level) < 0) {
        SYS_LOGE("%s, Cpq_LoadGamma fail\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success\n",__FUNCTION__);
    return 0;
}

int CPQControl::GetMultipointGammaEnable(void)
{
    int enable = 0;
    int level = GetColorTemperature();
    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, level)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n", __FUNCTION__);
        return enable;
    }

    enable = ColorTemp.GammaOffset.enable;

    if (enable < 0 || enable > 1) {
        enable = 0;
    }

    SYS_LOGD("%s enable: %d\n",__FUNCTION__, enable);
    return enable;
}

int CPQControl::SetMultipointGammaMode(int mode)
{
    int level = GetColorTemperature();
    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, level)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n", __FUNCTION__);
        return -1;
    }

    if (mode != ColorTemp.GammaOffset.MultipointGammaMode) {
        ColorTemp.GammaOffset.MultipointGammaMode = mode;
        if (!SetColorTemperatureData(&ColorTemp, level)) {
            SYS_LOGE("%s SetColorTemperatureData fail\n", __FUNCTION__);
            return -1;
        }
    } else {
        SYS_LOGD("%s, same mode %d!\n", __FUNCTION__, mode);
        return 0;
    }

    if (Cpq_LoadGamma((vpp_gamma_mode_t)GetGammaValue(), (vpp_color_temperature_mode_t)level) < 0) {
        SYS_LOGE("%s, Cpq_LoadGammafail\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s, success!\n", __FUNCTION__);
    return 0;
}

int CPQControl::GetMultipointGammaMode(void)
{
    int mode = WB_GAMMA_MODE_11POINT;
    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, GetColorTemperature())) {
        SYS_LOGE("%s GetColorTemperatureData fail\n", __FUNCTION__);
        return mode;
    }

    mode = ColorTemp.GammaOffset.MultipointGammaMode;

    if (mode < WB_GAMMA_MODE_2POINT || mode >= WB_GAMMA_MODE_MAX) {
        mode = WB_GAMMA_MODE_11POINT;
    }

    return mode;
}

//Gamma
int CPQControl::SetGammaValue(vpp_gamma_curve_t gamma_curve, int is_save)
{
    SYS_LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, gamma_curve);
    int ret = -1;

    ret = Cpq_LoadGamma((vpp_gamma_mode_t)gamma_curve, (vpp_color_temperature_mode_t)GetColorTemperature());

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveGammaValue((int)gamma_curve);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }
    return ret;
}

int CPQControl::GetGammaValue(void)
{
    int gammaValue = 0;
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d gamma: %d fail\n",__FUNCTION__, CurSource, CurTimming, gammaValue);
        return gammaValue;
    }

    gammaValue = para.GammaMidLuminance;

    SYS_LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, gammaValue);
    return gammaValue;

}

int CPQControl::SaveGammaValue(int gamma)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d gamma: %d fail\n",__FUNCTION__, CurSource, CurTimming, gamma);
        return -1;
    }

    para.GammaMidLuminance = gamma;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d gamma: %d fail\n",__FUNCTION__, CurSource, CurTimming, gamma);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_LoadGamma(vpp_gamma_mode_t gamma_curve, vpp_color_temperature_mode_t colortemp_mode)
{
    if (!mInitialized) {
        return 0;
    }

    if (!mbCpqCfg_gamma_enable) {
        SYS_LOGD("Gamma module disabled!\n");
        return 0;
    }

    int ret = 0;
    GAMMA_TABLE Gamma;
    memset(&Gamma, 0, sizeof(GAMMA_TABLE));
    ret |= GetBaseGammaData((int)colortemp_mode, &Gamma);
    ret |= GetMultipointGammaData((int)colortemp_mode, &Gamma);
    ret |= GetGammaPowerData((int)gamma_curve, &Gamma);

    // to driver
    if (ret < 0) {
        SYS_LOGE("%s, mixed Gamma failed!\n", __FUNCTION__);
    } else {
        ret |= Cpq_SetGammaTbl_R((unsigned short *)Gamma.R.data);
        ret |= Cpq_SetGammaTbl_G((unsigned short *)Gamma.G.data);
        ret |= Cpq_SetGammaTbl_B((unsigned short *)Gamma.B.data);
    }

    return ret;
}

int CPQControl::GetBaseGammaData(int level, GAMMA_TABLE *pData)
{
    if (pData == NULL) {
        SYS_LOGE("%s, pData is NULL\n", __FUNCTION__);
        return -1;
    }

    int ret = 0;
    ret |= mPQdb->PQ_GetWhiteBalanceGammaSpecialTable((vpp_color_temperature_mode_t)level, "Red",   &pData->R);
    ret |= mPQdb->PQ_GetWhiteBalanceGammaSpecialTable((vpp_color_temperature_mode_t)level, "Green", &pData->G);
    ret |= mPQdb->PQ_GetWhiteBalanceGammaSpecialTable((vpp_color_temperature_mode_t)level, "Blue",  &pData->B);

    if (ret < 0) {
        for (int i = 0; i < mPQdb->Gamma_nodes; i++) {
            pData->R.data[i] = (i * 4);
            if (pData->R.data[i] > 1023)
                pData->R.data[i] = 1023;
            pData->G.data[i] = (i * 4);
            if (pData->G.data[i] > 1023)
                pData->G.data[i] = 1023;
            pData->B.data[i] = (i * 4);
            if (pData->B.data[i] > 1023)
                pData->B.data[i] = 1023;
        }

        SYS_LOGE("%s: from pq.db fail, gen a linearity Table\n", __FUNCTION__);
        ret = 0;
    }

    if (ret < 0) {
        SYS_LOGE("%s: Fail\n", __FUNCTION__);
    } else {
        SYS_LOGD("%s: Success! level: %d, Gamma node = %d\n", __FUNCTION__, level, mPQdb->Gamma_nodes);
    }

    return ret;
}

unsigned int BT1886_GAMMA_256[256] = {
    0x00000,0x000A1,0x0019F,0x002D1,0x0042C,0x005A8,0x00741,0x008F3,0x00ABD,0x00C9C,0x00E8F,0x01094,0x012AB,0x014D2,0x01709,0x0194F,
    0x01BA3,0x01E05,0x02074,0x022F0,0x02578,0x0280C,0x02AAB,0x02D56,0x0300B,0x032CB,0x03596,0x0386A,0x03B49,0x03E30,0x04122,0x0441C,
    0x04720,0x04A2C,0x04D41,0x0505E,0x05384,0x056B2,0x059E8,0x05D26,0x0606C,0x063B9,0x0670E,0x06A6A,0x06DCE,0x07138,0x074AA,0x07823,
    0x07BA3,0x07F29,0x082B7,0x0864B,0x089E5,0x08D86,0x0912E,0x094DB,0x0988F,0x09C49,0x0A009,0x0A3D0,0x0A79C,0x0AB6E,0x0AF46,0x0B324,
    0x0B707,0x0BAF1,0x0BEDF,0x0C2D4,0x0C6CE,0x0CACD,0x0CED2,0x0D2DC,0x0D6EB,0x0DB00,0x0DF1A,0x0E339,0x0E75D,0x0EB86,0x0EFB4,0x0F3E8,
    0x0F820,0x0FC5D,0x1009F,0x104E6,0x10932,0x10D82,0x111D8,0x11632,0x11A90,0x11EF3,0x122EA,0x126BD,0x12A92,0x12E68,0x13241,0x1361C,
    0x139F8,0x13DD7,0x141B7,0x14599,0x1497D,0x14D63,0x1514B,0x15534,0x1591F,0x15D0C,0x160FA,0x164EB,0x168DD,0x16CD1,0x170C6,0x174BD,
    0x178B6,0x17CB0,0x180AC,0x184AA,0x188AA,0x18CAA,0x190AD,0x194B1,0x198B7,0x19CBE,0x1A0C7,0x1A4D1,0x1A8DD,0x1ACEA,0x1B0F9,0x1B50A,
    0x1B91C,0x1BD2F,0x1C144,0x1C55A,0x1C972,0x1CD8B,0x1D1A6,0x1D5C2,0x1D9DF,0x1DDFE,0x1E21E,0x1E640,0x1EA63,0x1EE87,0x1F2AD,0x1F6D4,
    0x1FAFD,0x1FF27,0x20352,0x2077F,0x20BAC,0x20FDC,0x2140C,0x2183E,0x21C71,0x220A5,0x224DB,0x22912,0x22D4A,0x23183,0x235BE,0x239FA,
    0x23E37,0x24276,0x246B5,0x24AF6,0x24F38,0x2537B,0x257C0,0x25C06,0x2604C,0x26495,0x268DE,0x26D28,0x27174,0x275C0,0x27A0E,0x27E5D,
    0x282AE,0x286FF,0x28B51,0x28FA5,0x293FA,0x29850,0x29CA7,0x2A0FF,0x2A558,0x2A9B2,0x2AE0E,0x2B26A,0x2B6C8,0x2BB26,0x2BF86,0x2C3E7,
    0x2C849,0x2CCAC,0x2D110,0x2D575,0x2D9DB,0x2DE42,0x2E2AA,0x2E713,0x2EB7D,0x2EFE9,0x2F455,0x2F8C2,0x2FD31,0x301A0,0x30610,0x30A82,
    0x30EF4,0x31367,0x317DC,0x31C51,0x320C7,0x3253E,0x329B7,0x32E30,0x332AA,0x33725,0x33BA1,0x3401F,0x3449D,0x3491C,0x34D9C,0x3521C,
    0x3569E,0x35B21,0x35FA5,0x36429,0x368AF,0x36D35,0x371BD,0x37645,0x37ACE,0x37F58,0x383E3,0x3886F,0x38CFC,0x3918A,0x39619,0x39AA8,
    0x39F39,0x3A3CA,0x3A85C,0x3ACEF,0x3B183,0x3B618,0x3BAAE,0x3BF44,0x3C3DC,0x3C874,0x3CD0D,0x3D1A7,0x3D642,0x3DADE,0x3DF7A,0x3E418,
};
unsigned int BT1886_GAMMA_257[257] = {
    0x00000,0x000A0,0x0019D,0x002CD,0x00426,0x005A0,0x00737,0x008E7,0x00AAF,0x00C8B,0x00E7B,0x0107E,0x01292,0x014B6,0x016EA,0x0192D,
    0x01B7E,0x01DDC,0x02048,0x022C0,0x02545,0x027D5,0x02A71,0x02D18,0x02FCA,0x03286,0x0354D,0x0381D,0x03AF8,0x03DDC,0x040C9,0x043BF,
    0x046BF,0x049C7,0x04CD8,0x04FF1,0x05312,0x0563C,0x0596E,0x05CA7,0x05FE8,0x06331,0x06681,0x069D9,0x06D38,0x0709E,0x0740B,0x0777F,
    0x07AFA,0x07E7C,0x08205,0x08594,0x08929,0x08CC5,0x09068,0x09410,0x097BF,0x09B74,0x09F2F,0x0A2F1,0x0A6B8,0x0AA84,0x0AE57,0x0B230,
    0x0B60E,0x0B9F2,0x0BDDB,0x0C1CA,0x0C5BF,0x0C9B9,0x0CDB8,0x0D1BC,0x0D5C6,0x0D9D5,0x0DDEA,0x0E203,0x0E622,0x0EA45,0x0EE6E,0x0F29B,
    0x0F6CE,0x0FB05,0x0FF42,0x10383,0x107C9,0x10C13,0x11062,0x114B6,0x1190F,0x11D6C,0x12192,0x12561,0x12931,0x12D03,0x130D7,0x134AE,
    0x13886,0x13C5F,0x1403B,0x14419,0x147F8,0x14BD9,0x14FBC,0x153A1,0x15787,0x15B70,0x15F59,0x16345,0x16733,0x16B22,0x16F12,0x17305,
    0x176F9,0x17AEF,0x17EE6,0x182DF,0x186DA,0x18AD6,0x18ED4,0x192D3,0x196D4,0x19AD6,0x19EDA,0x1A2E0,0x1A6E7,0x1AAF0,0x1AEFA,0x1B305,
    0x1B712,0x1BB21,0x1BF31,0x1C342,0x1C755,0x1CB6A,0x1CF7F,0x1D397,0x1D7AF,0x1DBC9,0x1DFE5,0x1E401,0x1E820,0x1EC3F,0x1F060,0x1F482,
    0x1F8A6,0x1FCCB,0x200F1,0x20519,0x20942,0x20D6C,0x21197,0x215C4,0x219F2,0x21E22,0x22252,0x22684,0x22AB8,0x22EEC,0x23322,0x23759,
    0x23B91,0x23FCA,0x24405,0x24841,0x24C7E,0x250BC,0x254FB,0x2593C,0x25D7E,0x261C1,0x26605,0x26A4A,0x26E91,0x272D9,0x27721,0x27B6B,
    0x27FB6,0x28403,0x28850,0x28C9F,0x290EE,0x2953F,0x29991,0x29DE4,0x2A238,0x2A68D,0x2AAE3,0x2AF3A,0x2B393,0x2B7EC,0x2BC47,0x2C0A2,
    0x2C4FF,0x2C95D,0x2CDBC,0x2D21B,0x2D67C,0x2DADE,0x2DF41,0x2E3A5,0x2E80A,0x2EC70,0x2F0D7,0x2F53F,0x2F9A9,0x2FE13,0x3027E,0x306EA,
    0x30B57,0x30FC5,0x31434,0x318A4,0x31D15,0x32187,0x325FA,0x32A6E,0x32EE3,0x33359,0x337D0,0x33C48,0x340C0,0x3453A,0x349B5,0x34E30,
    0x352AD,0x3572A,0x35BA8,0x36028,0x364A8,0x36929,0x36DAB,0x3722E,0x376B2,0x37B37,0x37FBC,0x38443,0x388CA,0x38D53,0x391DC,0x39666,
    0x39AF1,0x39F7D,0x3A40A,0x3A898,0x3AD26,0x3B1B5,0x3B646,0x3BAD7,0x3BF69,0x3C3FC,0x3C88F,0x3CD24,0x3D1B9,0x3D650,0x3DAE7,0x3DF7F,
    0x3E418,
};
int CPQControl::GetGammaPowerData(int level, GAMMA_TABLE *pData)
{
    if (pData == NULL) {
        SYS_LOGE("%s, pData is NULL\n", __FUNCTION__);
        return -1;
    }

    int ret = 0;
    if (level == VPP_GAMMA_CURVE_BT1886) {
        if (mPQdb->Gamma_nodes == 256) {
            ret |= DBGammaBlend(&pData->R, BT1886_GAMMA_256);
            ret |= DBGammaBlend(&pData->G, BT1886_GAMMA_256);
            ret |= DBGammaBlend(&pData->B, BT1886_GAMMA_256);
        } else {
            ret |= DBGammaBlend(&pData->R, BT1886_GAMMA_257);
            ret |= DBGammaBlend(&pData->G, BT1886_GAMMA_257);
            ret |= DBGammaBlend(&pData->B, BT1886_GAMMA_257);
        }
        SYS_LOGD("%s: GammaPower: BT1886\n", __FUNCTION__);
    } else {
        double GammaPower = GetGammaPower((vpp_gamma_curve_t)level);
        ret |= GammaOperation::GetInstance()->GammaOperation_BaseGammaConvert(pData->R.data, 2.2, GammaPower, mPQdb->Gamma_nodes);
        ret |= GammaOperation::GetInstance()->GammaOperation_BaseGammaConvert(pData->G.data, 2.2, GammaPower, mPQdb->Gamma_nodes);
        ret |= GammaOperation::GetInstance()->GammaOperation_BaseGammaConvert(pData->B.data, 2.2, GammaPower, mPQdb->Gamma_nodes);
        SYS_LOGD("%s: GammaPower: %.2f\n", __FUNCTION__, GammaPower);
    }

    if (ret < 0) {
        SYS_LOGE("%s, fail\n", __FUNCTION__);
    }

    return ret;
}

float x_2point[4]   = {GRAY_0,  GRAY_20, GRAY_80, GRAY_100};
float x_10point[11] = {GRAY_0,  GRAY_10, GRAY_20, GRAY_30, GRAY_40, GRAY_50, GRAY_60, GRAY_70, GRAY_80, GRAY_90, GRAY_100};
float x_11point[12] = {GRAY_0,  GRAY_5,  GRAY_10, GRAY_20, GRAY_30, GRAY_40, GRAY_50, GRAY_60, GRAY_70, GRAY_80, GRAY_90, GRAY_100};
float x_20point[21] = {GRAY_0,  GRAY_5,  GRAY_10, GRAY_15, GRAY_20, GRAY_25, GRAY_30, GRAY_35, GRAY_40, GRAY_45, GRAY_50, GRAY_55,
                       GRAY_60, GRAY_65, GRAY_70, GRAY_75, GRAY_80, GRAY_85, GRAY_90, GRAY_95, GRAY_100};
int CPQControl::GetMultipointGammaData(int level, GAMMA_TABLE *pData)
{
    if (pData == NULL) {
        SYS_LOGE("%s, pData is NULL\n", __FUNCTION__);
        return -1;
    }

    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, level)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n",__FUNCTION__);
        return -1;
    }

    if (ColorTemp.GammaOffset.enable == 0) {
        SYS_LOGD("%s MultipointGamma disable\n",__FUNCTION__);
        return 0;
    }

    if (FactoryGetMultipointGammaData(&ColorTemp.GammaOffset.Gamma, level)) {
        SYS_LOGD("%s Get MultipointGamma from CRI_DATA\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s Get MultipointGamma from TV_PICTURE\n",__FUNCTION__);
    }

    interpolation_info_t output;
    int PointNum = 0;
    switch (ColorTemp.GammaOffset.MultipointGammaMode) {
        case WB_GAMMA_MODE_2POINT:
            PointNum = 4;
            output.x = x_2point;
        break;
        case WB_GAMMA_MODE_10POINT:
            PointNum = 11;
            output.x = x_10point;
        break;
        case WB_GAMMA_MODE_11POINT:
            PointNum = 12;
            output.x = x_11point;
        break;
        case WB_GAMMA_MODE_20POINT:
            PointNum = 21;
            output.x = x_20point;
        break;
        default:
            PointNum = 12;
            output.x = x_11point;
        break;
    }

    if (mPQdb->Gamma_nodes == 257)
        output.x[PointNum - 1] = GRAY_100_257;

    SYS_LOGD("%s MultipointGammaMode is %d, PointNum is %d.\n",__FUNCTION__, ColorTemp.GammaOffset.MultipointGammaMode, PointNum);

    float *y_r = (float *)malloc(PointNum * sizeof(float));
    float *y_g = (float *)malloc(PointNum * sizeof(float));
    float *y_b = (float *)malloc(PointNum * sizeof(float));
    for (int i = 0; i < PointNum; i++) {
        int index = output.x[i];
        y_r[i] = (float)(pData->R.data[index] + ColorTemp.GammaOffset.Gamma.R_offset[i]);
        if (y_r[i] < 0.0) y_r[i] = 0.0;
        if (y_r[i] > 1023.0) y_r[i] = 1023.0;

        y_g[i] = (float)(pData->G.data[index] + ColorTemp.GammaOffset.Gamma.G_offset[i]);
        if (y_g[i] < 0.0) y_g[i] = 0.0;
        if (y_g[i] > 1023.0) y_g[i] = 1023.0;

        y_b[i] = (float)(pData->B.data[index] + ColorTemp.GammaOffset.Gamma.B_offset[i]);
        if (y_b[i] < 0.0) y_b[i] = 0.0;
        if (y_b[i] > 1023.0) y_b[i] = 1023.0;
    }

    output.y = y_r;
    if (CubeInterpolationProcess(output, pData->R.data, PointNum) != 0) {
        SYS_LOGE("%s CubeInterpolationProcess R fail\n",__FUNCTION__);
        free(y_r);
        free(y_g);
        free(y_b);
        return -1;
    }

    output.y = y_g;
    if (CubeInterpolationProcess(output, pData->G.data, PointNum) != 0) {
        SYS_LOGE("%s CubeInterpolationProcess G fail\n",__FUNCTION__);
        free(y_r);
        free(y_g);
        free(y_b);
        return -1;
    }

    output.y = y_b;
    if (CubeInterpolationProcess(output, pData->B.data, PointNum) != 0) {
        SYS_LOGE("%s CubeInterpolationProcess B fail\n",__FUNCTION__);
        free(y_r);
        free(y_g);
        free(y_b);
        return -1;
    }

    free(y_r);
    free(y_g);
    free(y_b);

    return 0;
}

int CPQControl::GetMultipointWBGammaData(int level, GAMMA_TABLE *pData)
{
    if (pData == NULL) {
        SYS_LOGE("%s, pData is NULL\n", __FUNCTION__);
        return -1;
    }

    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, level)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n",__FUNCTION__);
        return -1;
    }

    if (ColorTemp.GammaOffset.enable == 0) {
        SYS_LOGD("%s MultipointGamma disable\n",__FUNCTION__);
        return 0;
    }

    interpolation_info_t output;
    int PointNum = 0;
    switch (ColorTemp.GammaOffset.MultipointGammaMode) {
        case WB_GAMMA_MODE_2POINT:
            PointNum = 4;
            output.x = x_2point;
        break;
        case WB_GAMMA_MODE_10POINT:
            PointNum = 11;
            output.x = x_10point;
        break;
        case WB_GAMMA_MODE_11POINT:
            PointNum = 12;
            output.x = x_11point;
        break;
        case WB_GAMMA_MODE_20POINT:
            PointNum = 21;
            output.x = x_20point;
        break;
        default:
            PointNum = 12;
            output.x = x_11point;
        break;
    }

    if (mPQdb->Gamma_nodes == 257)
        output.x[PointNum - 1] = GRAY_100_257;

    SYS_LOGD("%s MultipointGammaMode is %d, PointNum is %d.\n",__FUNCTION__, ColorTemp.GammaOffset.MultipointGammaMode, PointNum);
    float *y_r = (float *)malloc(PointNum * sizeof(float));
    float *y_g = (float *)malloc(PointNum * sizeof(float));
    float *y_b = (float *)malloc(PointNum * sizeof(float));
    for (int i = 0; i < PointNum; i++) {
        int index = output.x[i];
        y_r[i] = (float)(pData->R.data[index] + ColorTemp.GammaOffset.Gamma.R_offset[i]);
        if (y_r[i] < 0.0) y_r[i] = 0.0;
        if (y_r[i] > 1023.0) y_r[i] = 1023.0;

        y_g[i] = (float)(pData->G.data[index] + ColorTemp.GammaOffset.Gamma.G_offset[i]);
        if (y_g[i] < 0.0) y_g[i] = 0.0;
        if (y_g[i] > 1023.0) y_g[i] = 1023.0;

        y_b[i] = (float)(pData->B.data[index] + ColorTemp.GammaOffset.Gamma.B_offset[i]);
        if (y_b[i] < 0.0) y_b[i] = 0.0;
        if (y_b[i] > 1023.0) y_b[i] = 1023.0;
    }

    output.y = y_r;
    if (CubeInterpolationProcess(output, pData->R.data, PointNum) != 0) {
        SYS_LOGE("%s CubeInterpolationProcess R fail\n",__FUNCTION__);
        free(y_r);
        free(y_g);
        free(y_b);
        return -1;
    }

    output.y = y_g;
    if (CubeInterpolationProcess(output, pData->G.data, PointNum) != 0) {
        SYS_LOGE("%s CubeInterpolationProcess G fail\n",__FUNCTION__);
        free(y_r);
        free(y_g);
        free(y_b);
        return -1;
    }

    output.y = y_b;
    if (CubeInterpolationProcess(output, pData->B.data, PointNum) != 0) {
        SYS_LOGE("%s CubeInterpolationProcess B fail\n",__FUNCTION__);
        free(y_r);
        free(y_g);
        free(y_b);
        return -1;
    }

    free(y_r);
    free(y_g);
    free(y_b);

    return 0;
}

int CPQControl::CubeInterpolationProcess(interpolation_info_t output, unsigned short *gamma, int num_points)
{
    interpolation_info_t *output_ptr = NULL;
    int i;                 /* loop index, as usual */
    int ret = -1;

    /* result of the interpolation */
    float result;
    output_ptr = GammaOperation::GetInstance()->nat_cubic_spline(num_points, &output);

    /* Now use our spline on each val we made up */
    for (i = 0; i < mPQdb->Gamma_nodes; i++) {
        /* Make sure to test the return value before we use the result */
        if ((ret = GammaOperation::GetInstance()->evaluate(output_ptr, (float)i, &result)) < 0) {
            /* Should fail on none of the inputs */
           SYS_LOGE("%s evaluate failed: %d\n", __FUNCTION__, ret);
           return ret;
        }
        /* print the input x value and the interpolated y */
        //SYS_LOGE("%s %.2f,%.2f\n", __FUNCTION__,  (float)i, result);

        gamma[i] = (unsigned short)result;
        if (i > 0) {
            if (gamma[i - 1] > gamma[i]) {
                gamma[i] = gamma[i - 1];
            }
        }
        if (gamma[i] >= 1020) {
            gamma[i] = 1020;
        }
        //SYS_LOGD("%s gamma.data[%d] = %hd\n", __FUNCTION__, i, gamma[i]);
    }

    return ret;
}

int CPQControl::DBGammaBlend(tcon_gamma_table_t *wb_gamma, unsigned int *index_gamma)
{
    unsigned int i, final_value;
    unsigned int blend_alp, blend_bet;
    tcon_gamma_table_t target_gamma;
    unsigned int Node = (unsigned int)mPQdb->Gamma_nodes;
    for (i = 1; i < (Node - 1); i++) {
        blend_alp = index_gamma[i] / 1000;
        blend_bet = index_gamma[i] % 1000;
        if (blend_alp > (Node - 1)) {
            SYS_LOGD("%s, blend_gamma->data[i] = %d\n", __FUNCTION__, i, index_gamma[i]);
            SYS_LOGD("%s, blend_alp = %d\n", __FUNCTION__, blend_alp);
            SYS_LOGD("%s, blend_bet = %d\n", __FUNCTION__, blend_bet);
            continue;
        }
        final_value = wb_gamma->data[blend_alp] + (wb_gamma->data[blend_alp + 1] - wb_gamma->data[blend_alp]) * blend_bet / 1000;
        target_gamma.data[i] = (unsigned short)final_value;
    }

    target_gamma.data[0] = wb_gamma->data[0];
    target_gamma.data[Node - 1] = wb_gamma->data[Node - 1];

    memcpy(wb_gamma, &target_gamma, sizeof(tcon_gamma_table_t));
    return 0;
}

double CPQControl::GetGammaPower(vpp_gamma_curve_t mode)
{
    double gamma_power = 2.2;
    switch (mode) {
        case VPP_GAMMA_CURVE_DEFAULT:
            gamma_power = 2.2;
        break;
        case VPP_GAMMA_CURVE_1:
            gamma_power = 1.7;
        break;
        case VPP_GAMMA_CURVE_2:
            gamma_power = 1.8;
        break;
        case VPP_GAMMA_CURVE_3:
            gamma_power = 1.9;
        break;
        case VPP_GAMMA_CURVE_4:
            gamma_power = 2.0;
        break;
        case VPP_GAMMA_CURVE_5:
            gamma_power = 2.1;
        break;
        case VPP_GAMMA_CURVE_6:
            gamma_power = 2.2;
        break;
        case VPP_GAMMA_CURVE_7:
            gamma_power = 2.3;
        break;
        case VPP_GAMMA_CURVE_8:
            gamma_power = 2.4;
        break;
        case VPP_GAMMA_CURVE_9:
            gamma_power = 2.5;
        break;
        case VPP_GAMMA_CURVE_10:
            gamma_power = 2.6;
        break;
        case VPP_GAMMA_CURVE_11:
            gamma_power = 2.7;
        break;
        default:
            gamma_power = 2.2;
        break;
    }

    return gamma_power;
}

int CPQControl::Cpq_SetGammaTbl_R(unsigned short red[GAMMA_NUMBER])
{
    struct tcon_gamma_table_s Redtbl;
    int ret = -1, i = 0;

    for (i = 0; i < GAMMA_NUMBER; i++) {
        Redtbl.data[i] = red[i];
    }

    ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_TABLE_R, &Redtbl);
    if (ret < 0) {
        SYS_LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    }
    return ret;
}

int CPQControl::Cpq_SetGammaTbl_G(unsigned short green[GAMMA_NUMBER])
{
    struct tcon_gamma_table_s Greentbl;
    int ret = -1, i = 0;

    for (i = 0; i < GAMMA_NUMBER; i++) {
        Greentbl.data[i] = green[i];
    }

    ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_TABLE_G, &Greentbl);
    if (ret < 0) {
        SYS_LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::Cpq_SetGammaTbl_B(unsigned short blue[GAMMA_NUMBER])
{
    struct tcon_gamma_table_s Bluetbl;
    int ret = -1, i = 0;

    for (i = 0; i < GAMMA_NUMBER; i++) {
        Bluetbl.data[i] = blue[i];
    }

    ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_TABLE_B, &Bluetbl);
    if (ret < 0) {
        SYS_LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::SetGammaPattern(int enable, int R, int G, int B)
{
    if (enable == 0) {
        return Cpq_LoadGamma((vpp_gamma_mode_t)GetGammaValue(), (vpp_color_temperature_mode_t)GetColorTemperature());
    }

    tcon_gamma_table_t WB_GAMMA_R, WB_GAMMA_G, WB_GAMMA_B;
    for (int i = 0; i < 257; i++) {
        WB_GAMMA_R.data[i] = R * 4;
        WB_GAMMA_G.data[i] = G * 4;
        WB_GAMMA_B.data[i] = B * 4;
        if (WB_GAMMA_R.data[i] > 1023) WB_GAMMA_R.data[i] = 1023;
        if (WB_GAMMA_G.data[i] > 1023) WB_GAMMA_G.data[i] = 1023;
        if (WB_GAMMA_B.data[i] > 1023) WB_GAMMA_B.data[i] = 1023;
    }

    Cpq_SetGammaTbl_R(WB_GAMMA_R.data);
    Cpq_SetGammaTbl_G(WB_GAMMA_G.data);
    Cpq_SetGammaTbl_B(WB_GAMMA_B.data);

    return true;
}

//MEMC
bool CPQControl::hasMemcFunc() {
    if (mMemcFd > 0) {
        SYS_LOGD("%s, has memc\n", __FUNCTION__);
        return true;
    }

    SYS_LOGD("%s, has NO memc\n", __FUNCTION__);
    return false;
}

int CPQControl::Memc_enable(int enable)
{
    int ret = -1;
    ret = MEMCDeviceIOCtl(MEMDEV_CONTRL, &enable);

    if (ret >= 0) {
        SYS_LOGD("%s, success\n", __FUNCTION__);
    } else {
        SYS_LOGD("%s, fail\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::SetMemcMode(int memc_mode, int is_save)
{
    SYS_LOGD("%s, mode = %d\n", __FUNCTION__, memc_mode);
    int ret = -1;
    ret = Cpq_SetMemcMode((MEMC_MODE)memc_mode, mCurrentSourceInputInfo);

    if (ret == 0 && is_save == 1) {
        ret = SaveMemcMode((MEMC_MODE)memc_mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetMemcMode(void)
{
    int level = MEMC_MODE_OFF;
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, (PICTURE_MODE)GetPQMode())) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d data: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
        return level;
    }

    level = para.Memc;

    if (level < MEMC_MODE_OFF || level >= MEMC_MODE_MAX) {
        level = MEMC_MODE_OFF;
    }

    SYS_LOGD("%s, source: %d, timming: %d, level = %d\n", __FUNCTION__, CurSource, CurTimming, level);
    return level;
}

int CPQControl::SaveMemcMode(MEMC_MODE memc_mode)
{
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, (PICTURE_MODE)GetPQMode())) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d, memc_mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, memc_mode);
        return -1;
    }

    para.Memc = (int)memc_mode;

    if (!SetPictureModeData(&para, (PICTURE_MODE)GetPQMode())) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d, memc_mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, memc_mode);
        return -1;
    }

    SYS_LOGD("%s success! source: %d, timming: %d, level: %d\n",__FUNCTION__, CurSource, CurTimming, memc_mode);
    return 0;
}

int CPQControl::Cpq_SetMemcMode(MEMC_MODE memc_mode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_memc_enable) {
        SYS_LOGD("%s memc Disabled!\n",__FUNCTION__);
        return 0;
    }

    if (isGameMode()) {
        memc_mode = MEMC_MODE_OFF;
        SYS_LOGE("%s, isGameMode, Set memc mode OFF!!!\n", __FUNCTION__);
    }

    PICTURE_SETTING_BY_SRC pData;
    if (!GetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s GetPictureStructDataBySrc failed\n", __FUNCTION__);
        return -1;
    }

    int ret = 0;
    int DeJudder_level = pData.memc[memc_mode].DeJudderLevel;
    int DeBlur_Level = pData.memc[memc_mode].DeBlurLevel;

    if (mPQdb->mDbMatchType == MATCH_TYPE_MBOX_T3X &&
        memc_mode == MEMC_MODE_OFF &&
        !isGameMode() &&
        mFrameRate <= 60) {
        SYS_LOGD("%s, though ui off level, but dejudder is 0\n", __FUNCTION__);
        DeJudder_level = 0;
        DeBlur_Level = 0;
    }

    ret |= Cpq_SetMemcDeJudderLevel(DeJudder_level, mCurrentSourceInputInfo);
    ret |= Cpq_SetMemcDeBlurLevel(DeBlur_Level, mCurrentSourceInputInfo);

    if (memc_mode == MEMC_MODE_OFF) {
        if (Cpq_GetMemcTrueFalseOff(memc_mode)) {
            ret |= Memc_enable(0);
        }
    } else {
        ret |= Memc_enable(1);
    }

    return ret;
}

bool CPQControl::Cpq_GetMemcTrueFalseOff(MEMC_MODE memc_mode)
{
    bool memc_off = true;

    if (mPQdb->mDbMatchType == MATCH_TYPE_MBOX_T3X) {
        if (memc_mode == MEMC_MODE_OFF) {
            if (mCurrentHdrType == HDR_TYPE_DOVI || isGameMode()) {
                memc_off = true;
            } else {
                memc_off = false;
            }
        }
    }

    SYS_LOGD("%s, memc_off = %d\n", __FUNCTION__, memc_off);
    return memc_off;
}

int CPQControl::SetMemcDeBlurLevel(int level, int is_save)
{
    if (is_save == 1) {
        SaveMemcDeBlurLevel(level);
    }

    if (Cpq_SetMemcDeBlurLevel(level, mCurrentSourceInputInfo) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s, success! source: %d, timming: %d, level = %d\n", __FUNCTION__, CurSource, CurTimming, level);
    return 0;
}

int CPQControl::GetMemcDeBlurLevel(void)
{
    int level = 0;
    PICTURE_SETTING_BY_SRC pData;
    if (!GetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s GetPictureStructDataBySrc failed\n", __FUNCTION__);
        return level;
    }

    int mode = GetMemcMode();
    if (mode < MEMC_MODE_OFF || mode >= MEMC_MODE_MAX) {
        mode = MEMC_MODE_OFF;
        SYS_LOGE("%s, mode out of range, use default!!\n", __FUNCTION__);
    }

    level = pData.memc[mode].DeBlurLevel;

    SYS_LOGD("%s, success! source: %d, timming: %d, level = %d\n", __FUNCTION__, CurSource, CurTimming, level);
    return level;
}

int CPQControl::SaveMemcDeBlurLevel(int level)
{
    PICTURE_SETTING_BY_SRC pData;
    if (!GetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s GetPictureStructDataBySrc failed\n", __FUNCTION__);
        return -1;
    }

    int mode = GetMemcMode();
    if (mode < MEMC_MODE_OFF || mode >= MEMC_MODE_MAX) {
        mode = MEMC_MODE_OFF;
        SYS_LOGE("%s, mode out of range, use default!!\n", __FUNCTION__);
    }

    pData.memc[mode].DeBlurLevel = level;

    if (!SetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s SetPictureStructDataBySrc failed\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s, success! source: %d, timming: %d, level = %d\n", __FUNCTION__, CurSource, CurTimming, level);
    return 0;
}

int CPQControl::Cpq_SetMemcDeBlurLevel(int level, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_memc_enable) {
        SYS_LOGD("%s memc Disabled!\n",__FUNCTION__);
        return 0;
    }

    if (MEMCDeviceIOCtl(FRC_IOC_SET_DEBLUR_LEVEL, &level) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s, success! source: %d, timming: %d, level = %d\n", __FUNCTION__, CurSource, CurTimming, level);
    return 0;
}

int CPQControl::SetMemcDeJudderLevel(int level, int is_save)
{
    if (is_save == 1) {
        SaveMemcDeJudderLevel(level);
    }

    if (Cpq_SetMemcDeJudderLevel(level, mCurrentSourceInputInfo) < 0) {
        SYS_LOGE("%s failed! source: %d, timming: %d, level = %d\n", __FUNCTION__, CurSource, CurTimming, level);
        return -1;
    }

    SYS_LOGD("%s, success! source: %d, timming: %d, level = %d\n", __FUNCTION__, CurSource, CurTimming, level);
    return 0;
}

int CPQControl::GetMemcDeJudderLevel(void)
{
    int level = 0;
    PICTURE_SETTING_BY_SRC pData;
    if (!GetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s GetPictureStructDataBySrc failed\n", __FUNCTION__);
        return level;
    }

    int mode = GetMemcMode();
    if (mode < MEMC_MODE_OFF || mode >= MEMC_MODE_MAX) {
        mode = MEMC_MODE_OFF;
        SYS_LOGE("%s, mode out of range, use default!!\n", __FUNCTION__);
    }

    level = pData.memc[mode].DeJudderLevel;

    SYS_LOGD("%s, source: %d, timming: %d, level = %d\n", __FUNCTION__, CurSource, CurTimming, level);
    return level;
}

int CPQControl::SaveMemcDeJudderLevel(int level)
{
    PICTURE_SETTING_BY_SRC pData;
    if (!GetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s GetPictureStructDataBySrc failed\n", __FUNCTION__);
        return -1;
    }

    int mode = GetMemcMode();
    if (mode < MEMC_MODE_OFF || mode >= MEMC_MODE_MAX) {
        mode = MEMC_MODE_OFF;
        SYS_LOGE("%s, mode out of range, use default!!\n", __FUNCTION__);
    }

    pData.memc[mode].DeJudderLevel = level;

    if (!SetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s SetPictureStructDataBySrc failed\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s, success! source: %d, timming: %d, level = %d\n", __FUNCTION__, CurSource, CurTimming, level);
    return 0;
}

int CPQControl::Cpq_SetMemcDeJudderLevel(int level, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_memc_enable) {
        SYS_LOGD("%s memc Disabled!\n",__FUNCTION__);
        return 0;
    }

    if (MEMCDeviceIOCtl(FRC_IOC_SET_MEMC_LEVEL, &level) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s, success! source: %d, timming: %d, level = %d\n", __FUNCTION__, CurSource, CurTimming, level);
    return 0;
}

//Displaymode
int CPQControl::SetDisplayMode(vpp_display_mode_t display_mode, int is_save)
{
    SYS_LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, display_mode);
    int ret = -1;

    //dtvkit process afd function,driver need output full
    if (display_mode == VPP_DISPLAY_MODE_NORMAL) {
        pqWriteSys(VPP_AFD_MODULE_ASPECT_MODE, "0 0");//set auto to afd before pq display
    }
    if (mbDtvKitEnable && (display_mode == VPP_DISPLAY_MODE_NORMAL)) {
        ret = Cpq_SetDisplayModeAllTiming(mCurrentSourceInputInfo.source_input, display_mode);
        ret = Cpq_SetDisplayModeScreenMode(mCurrentSourceInputInfo.source_input, display_mode);
    } else if ((mCurrentSourceInputInfo.source_input == SOURCE_DTV)
        || (mCurrentSourceInputInfo.source_input == SOURCE_TV)
        || (mCurrentSourceInputInfo.source_input == SOURCE_AV1)
        || (mCurrentSourceInputInfo.source_input == SOURCE_AV2)) {
        ret = Cpq_SetDisplayModeAllTiming(mCurrentSourceInputInfo.source_input, display_mode);
    } else {
        ret = Cpq_SetDisplayModeAllTiming(mCurrentSourceInputInfo.source_input, display_mode);
        ret = Cpq_SetDisplayModeOneTiming(mCurrentSourceInputInfo.source_input, display_mode);
    }
    if (display_mode != VPP_DISPLAY_MODE_NORMAL) {
        pqWriteSys(VPP_AFD_MODULE_ASPECT_MODE, "0 5");//set custom to afd after pq display
    }

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveDisplayMode(display_mode);
    }

    return ret;
}

int CPQControl::GetDisplayMode()
{
    int mode = VPP_DISPLAY_MODE_169;
    PICTURE_SETTING_BY_SRC pData;
    if (!GetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s GetPictureStructDataBySrc failed\n", __FUNCTION__);
        return mode;
    }

    mode = pData.DisplayMode;

    if (mode < VPP_DISPLAY_MODE_169 || mode >= VPP_DISPLAY_MODE_MAX) {
        mode = VPP_DISPLAY_MODE_169;
    }

    SYS_LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, mode);
    return mode;
}

int CPQControl::SaveDisplayMode(vpp_display_mode_t display_mode)
{
    PICTURE_SETTING_BY_SRC pData;
    if (!GetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s GetPictureStructDataBySrc failed\n", __FUNCTION__);
        return -1;
    }

    pData.DisplayMode = (int)display_mode;

    if (!SetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s SetPictureStructDataBySrc failed\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetDisplayModeCrop(tv_source_input_t source_input, vpp_display_mode_t display_mode)
{
    int ret = -1;
    tvin_cutwin_t cutwin;
    if (mbCpqCfg_display_overscan_enable) {
        if (mbCpqCfg_separate_db_enable) {
            ret = mpOverScandb->PQ_GetOverscanParams(mCurrentSourceInputInfo, display_mode, &cutwin);
        }
    } else {
        SYS_LOGD("%s: Overscan module disabled\n", __FUNCTION__);
        ret = 0;
        cutwin.he = 0;
        cutwin.hs = 0;
        cutwin.ve = 0;
        cutwin.vs = 0;
    }

    if (ret == 0) {
       if (source_input == SOURCE_DTV) {//DTVKIT
            cutwin.vs = 0;
            cutwin.hs = 0;
            cutwin.ve = 0;
            cutwin.he = 0;
        } else if (source_input == SOURCE_MPEG) {//MPEG
            cutwin.vs = 0;
            cutwin.hs = 0;
            cutwin.ve = 0;
            cutwin.he = 0;
        } else if ((source_input >= SOURCE_HDMI1) && (source_input <= SOURCE_HDMI4)) {//hdmi source
            if (GetPQMode() == VPP_PICTURE_MODE_MONITOR) {//hdmi monitor mode
                cutwin.vs = 0;
                cutwin.hs = 0;
                cutwin.ve = 0;
                cutwin.he = 0;
            }
        }

        SYS_LOGD("%s: display_mode:%d hs:%d he:%d vs:%d ve:%d\n", __FUNCTION__, display_mode, cutwin.hs, cutwin.he, cutwin.vs, cutwin.ve);
        Cpq_SetVideoCrop(cutwin.vs, cutwin.hs, cutwin.ve, cutwin.he);
    } else {
        SYS_LOGD("PQ_GetOverscanParams failed\n");
    }

    return ret;
}

int CPQControl::Cpq_SetDisplayModeScreenMode(tv_source_input_t source_input, vpp_display_mode_t display_mode)
{
    int ret = 0;

    int ScreenModeValue = Cpq_GetScreenModeValue(display_mode);
    if (source_input == SOURCE_DTV) {//DTVKIT
        if ((mbDtvKitEnable && (ScreenModeValue == SCREEN_MODE_NORMAL)) || (!mbDtvKitEnable)) {
            ScreenModeValue = SCREEN_MODE_FULL_STRETCH;
        }
    } else if (source_input == SOURCE_MPEG) {//MPEG
        if ((mbDtvKitEnable && (ScreenModeValue == SCREEN_MODE_NORMAL)) || (!mbDtvKitEnable)) {
            ScreenModeValue = SCREEN_MODE_FULL_STRETCH;
        }
    } else if ((source_input >= SOURCE_HDMI1) && (source_input <= SOURCE_HDMI4)) {//hdmi source
        if (display_mode == VPP_DISPLAY_MODE_NORMAL) {//auto mode
            if (mCurrentAfdInfo == TVIN_ASPECT_4x3_FULL) {
                ScreenModeValue = SCREEN_MODE_4_3;
            } else if (mCurrentAfdInfo == TVIN_ASPECT_14x9_FULL) {
                ScreenModeValue = SCREEN_MODE_NORMAL;
            } else if (mCurrentAfdInfo == TVIN_ASPECT_16x9_FULL) {
                ScreenModeValue = SCREEN_MODE_16_9;
            } else {
                SYS_LOGD("%s: invalid AFD status\n", __FUNCTION__);
            }
        }
    }

    SYS_LOGD("%s: screenmode:%d\n", __FUNCTION__, ScreenModeValue);
    Cpq_SetVideoScreenMode(ScreenModeValue);

    return ret;
}

int CPQControl::Cpq_SetDisplayModeOneTiming(tv_source_input_t source_input, vpp_display_mode_t display_mode)
{
    int ret = -1;
    tvin_cutwin_t cutwin;
    if (mbCpqCfg_display_overscan_enable) {
        if (mbCpqCfg_separate_db_enable) {
            ret = mpOverScandb->PQ_GetOverscanParams(mCurrentSourceInputInfo, display_mode, &cutwin);
        }
    } else {
        SYS_LOGD("%s: Overscan module disabled!\n", __FUNCTION__);
        ret = 0;
        cutwin.he = 0;
        cutwin.hs = 0;
        cutwin.ve = 0;
        cutwin.vs = 0;
    }

    if (ret == 0) {
        int ScreenModeValue = Cpq_GetScreenModeValue(display_mode);
       if (source_input == SOURCE_DTV) {//DTVKIT
            cutwin.vs = 0;
            cutwin.hs = 0;
            cutwin.ve = 0;
            cutwin.he = 0;
            if ((mbDtvKitEnable && (ScreenModeValue == SCREEN_MODE_NORMAL))
                ||(!mbDtvKitEnable)) {
                    ScreenModeValue = SCREEN_MODE_FULL_STRETCH;
            }
        } else if (source_input == SOURCE_MPEG) {//MPEG
            cutwin.vs = 0;
            cutwin.hs = 0;
            cutwin.ve = 0;
            cutwin.he = 0;
            if ((mbDtvKitEnable && (ScreenModeValue == SCREEN_MODE_NORMAL))
                ||(!mbDtvKitEnable)) {
                    ScreenModeValue = SCREEN_MODE_FULL_STRETCH;
            }
        } else if ((source_input >= SOURCE_HDMI1) && (source_input <= SOURCE_HDMI4)) {//hdmi source
            if (GetPQMode() == VPP_PICTURE_MODE_MONITOR) {//hdmi monitor mode
                cutwin.vs = 0;
                cutwin.hs = 0;
                cutwin.ve = 0;
                cutwin.he = 0;
            }

            if (display_mode == VPP_DISPLAY_MODE_NORMAL) {//auto mode
                if (mCurrentAfdInfo == TVIN_ASPECT_4x3_FULL) {
                    ScreenModeValue = SCREEN_MODE_4_3;
                } else if (mCurrentAfdInfo == TVIN_ASPECT_14x9_FULL) {
                    ScreenModeValue = SCREEN_MODE_NORMAL;
                } else if (mCurrentAfdInfo == TVIN_ASPECT_16x9_FULL) {
                    ScreenModeValue = SCREEN_MODE_16_9;
                } else {
                    SYS_LOGE("%s: invalid AFD status.\n", __FUNCTION__);
                }
            }
        }

        SYS_LOGD("%s: screenmode:%d hs:%d he:%d vs:%d ve:%d\n", __FUNCTION__, ScreenModeValue, cutwin.hs, cutwin.he, cutwin.vs, cutwin.ve);
        Cpq_SetVideoCrop(cutwin.vs, cutwin.hs, cutwin.ve, cutwin.he);
        Cpq_SetVideoScreenMode(ScreenModeValue);
    } else {
        SYS_LOGE("PQ_GetOverscanParams failed!\n");
    }

    return ret;
}

int CPQControl::Cpq_SetDisplayModeAllTiming(tv_source_input_t source_input, vpp_display_mode_t display_mode)
{
    int i = 0, ScreenModeValue = 0, AFDFlag = 0, adapted_mode = 0;
    int ret = -1;
    ve_pq_load_t ve_pq_load_reg;
    memset(&ve_pq_load_reg, 0, sizeof(ve_pq_load_t));

    ve_pq_load_reg.param_id = TABLE_NAME_OVERSCAN;
    ve_pq_load_reg.length = SIG_TIMING_TYPE_MAX;

    ve_pq_table_t ve_pq_table[SIG_TIMING_TYPE_MAX];
    tvin_cutwin_t cutwin[SIG_TIMING_TYPE_MAX];
    memset(ve_pq_table, 0, sizeof(ve_pq_table));
    memset(cutwin, 0, sizeof(cutwin));

    tvin_sig_fmt_t sig_fmt[SIG_TIMING_TYPE_MAX];
    ve_pq_timing_type_t flag[SIG_TIMING_TYPE_MAX];
    sig_fmt[0] = TVIN_SIG_FMT_HDMI_720X480P_60HZ;
    sig_fmt[1] = TVIN_SIG_FMT_HDMI_720X576P_50HZ;
    sig_fmt[2] = TVIN_SIG_FMT_HDMI_1280X720P_60HZ;
    sig_fmt[3] = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
    sig_fmt[4] = TVIN_SIG_FMT_HDMI_3840_2160_00HZ;
    sig_fmt[5] = TVIN_SIG_FMT_CVBS_NTSC_M;
    sig_fmt[6] = TVIN_SIG_FMT_CVBS_NTSC_443;
    sig_fmt[7] = TVIN_SIG_FMT_CVBS_PAL_I;
    sig_fmt[8] = TVIN_SIG_FMT_CVBS_PAL_M;
    sig_fmt[9] = TVIN_SIG_FMT_CVBS_PAL_60;
    sig_fmt[10] = TVIN_SIG_FMT_CVBS_PAL_CN;
    sig_fmt[11] = TVIN_SIG_FMT_CVBS_SECAM;
    sig_fmt[12] = TVIN_SIG_FMT_CVBS_NTSC_50;
    flag[0] = SIG_TIMING_TYPE_SD_480;
    flag[1] = SIG_TIMING_TYPE_SD_576;
    flag[2] = SIG_TIMING_TYPE_HD;
    flag[3] = SIG_TIMING_TYPE_FHD;
    flag[4] = SIG_TIMING_TYPE_UHD;
    flag[5] = SIG_TIMING_TYPE_NTSC_M;
    flag[6] = SIG_TIMING_TYPE_NTSC_443;
    flag[7] = SIG_TIMING_TYPE_PAL_I;
    flag[8] = SIG_TIMING_TYPE_PAL_M;
    flag[9] = SIG_TIMING_TYPE_PAL_60;
    flag[10] = SIG_TIMING_TYPE_PAL_CN;
    flag[11] = SIG_TIMING_TYPE_SECAM;
    flag[12] = SIG_TIMING_TYPE_NTSC_50;

    source_input_param_t source_input_param;
    source_input_param.source_input = source_input;
    source_input_param.trans_fmt = mCurrentSourceInputInfo.trans_fmt;
    ScreenModeValue = Cpq_GetScreenModeValue(display_mode);

    //non dtvkit,driver process afd
    if (display_mode == VPP_DISPLAY_MODE_NORMAL) { //auto mode
        AFDFlag = 1;
    } else {
        AFDFlag = 0;
    }

    //dtvkit process afd function,driver need output full
    if (mbDtvKitEnable && (ScreenModeValue == SCREEN_MODE_NORMAL)) {
        adapted_mode    = 0;
        ScreenModeValue = SCREEN_MODE_FULL_STRETCH;
        AFDFlag         = 0;

        ve_pq_table[0].src_timing = (adapted_mode << 31) |
                                    (AFDFlag << 30) |
                                    ((ScreenModeValue & 0x7f) << 24) |
                                    ((source_input & 0x7f) << 16 ) |
                                    (0x0);
        ve_pq_table[0].value1 = 0;
        ve_pq_table[0].value2 = 0;
        ve_pq_load_reg.param_ptr = (long long)ve_pq_table;

        ret = 0;
    } else if (source_input == SOURCE_DTV) {//DTV
        for (i = 0; i < SIG_TIMING_TYPE_NTSC_M; i++) {
            adapted_mode = 1;
            ve_pq_table[i].src_timing = (adapted_mode << 31) |
                                        (AFDFlag << 30) |
                                        ((ScreenModeValue & 0x7f) << 24) |
                                        ((source_input & 0x7f) << 16 ) |
                                        (flag[i]);
            source_input_param.sig_fmt = sig_fmt[i];
            if  (display_mode >= VPP_DISPLAY_MODE_FULL_STRETCH && display_mode <= VPP_DISPLAY_MODE_169_COMBINED) {
                SYS_LOGI("%s: Project mode!\n", __FUNCTION__);
                ret = 0;
                cutwin[i].he = 0;
                cutwin[i].hs = 0;
                cutwin[i].ve = 0;
                cutwin[i].vs = 0;
            } else if (mbCpqCfg_display_overscan_enable) {
                if (mbCpqCfg_separate_db_enable) {
                    ret = mpOverScandb->PQ_GetOverscanParams(source_input_param, display_mode, cutwin+i);
                }
            } else {
                SYS_LOGD("%s: Overscan module disabled!\n", __FUNCTION__);
                ret = 0;
                cutwin[i].he = 0;
                cutwin[i].hs = 0;
                cutwin[i].ve = 0;
                cutwin[i].vs = 0;
            }

            //dtvkit process afd function,driver need output full
            if (mbDtvKitEnable && (ScreenModeValue == SCREEN_MODE_FULL_STRETCH)) {
                cutwin[i].he = 0;
                cutwin[i].hs = 0;
                cutwin[i].ve = 0;
                cutwin[i].vs = 0;
            }

            if (ret == 0) {
                SYS_LOGD("signal_fmt:0x%x, AFDFlag:%d,screen mode:%d he:%d hs:%d ve:%d vs:%d!\n", sig_fmt[i], AFDFlag, ScreenModeValue, cutwin[i].he, cutwin[i].hs, cutwin[i].ve, cutwin[i].vs);
                ve_pq_table[i].value1 = ((cutwin[i].he & 0xffff)<<16) | (cutwin[i].hs & 0xffff);
                ve_pq_table[i].value2 = ((cutwin[i].ve & 0xffff)<<16) | (cutwin[i].vs & 0xffff);
            } else {
                SYS_LOGE("PQ_GetOverscanParams failed!\n");
            }
        }
        ve_pq_load_reg.param_ptr = (long long)ve_pq_table;
    } else if ((source_input == SOURCE_TV) || (source_input == SOURCE_AV1) || (source_input == SOURCE_AV2)) {//ATV AV SOURCE
        for (i = SIG_TIMING_TYPE_NTSC_M; i < SIG_TIMING_TYPE_MAX; i++) {
            adapted_mode = 1;
            ve_pq_table[i].src_timing = (adapted_mode << 31) |
                                        (AFDFlag << 30) |
                                        ((ScreenModeValue & 0x7f) << 24) |
                                        ((source_input & 0x7f) << 16 ) |
                                        (flag[i]);
            source_input_param.sig_fmt = sig_fmt[i];

            if (mbCpqCfg_display_overscan_enable) {
                if (mbCpqCfg_separate_db_enable) {
                    ret = mpOverScandb->PQ_GetOverscanParams(source_input_param, display_mode, cutwin+i);
                }
            } else {
                SYS_LOGD("%s: Overscan module disabled!\n", __FUNCTION__);
                ret = 0;
                cutwin[i].he = 0;
                cutwin[i].hs = 0;
                cutwin[i].ve = 0;
                cutwin[i].vs = 0;
            }

            if (ret == 0) {
                SYS_LOGD("signal_fmt:0x%x, screen mode:%d hs:%d he:%d vs:%d ve:%d!\n", sig_fmt[i], ScreenModeValue, cutwin[i].he, cutwin[i].hs, cutwin[i].ve, cutwin[i].vs);
                ve_pq_table[i].value1 = ((cutwin[i].he & 0xffff)<<16) | (cutwin[i].hs & 0xffff);
                ve_pq_table[i].value2 = ((cutwin[i].ve & 0xffff)<<16) | (cutwin[i].vs & 0xffff);
            } else {
                SYS_LOGE("PQ_GetOverscanParams failed!\n");
            }
        }
        ve_pq_load_reg.param_ptr = (long long)ve_pq_table;
    } else {//HDMI && MPEG
        adapted_mode = 0;
        ve_pq_table[0].src_timing = (adapted_mode << 31) |
                                    (AFDFlag << 30) |
                                    ((ScreenModeValue & 0x7f) << 24) |
                                    ((source_input & 0x7f) << 16 ) |
                                    (0x0);
        ve_pq_table[0].value1 = 0;
        ve_pq_table[0].value2 = 0;
        ve_pq_load_reg.param_ptr = (long long)ve_pq_table;

        ret = 0;
    }

    if (ret == 0) {
        SYS_LOGD("source_input:%d, adapted_mode:%d, AFDFlag:%d,screen mode:%d\n", source_input, adapted_mode, AFDFlag, ScreenModeValue);
        ret = Cpq_LoadDisplayModeRegs(ve_pq_load_reg);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
        return 0;
    }

}

int CPQControl::Cpq_GetScreenModeValue(vpp_display_mode_t display_mode)
{
    int value = SCREEN_MODE_16_9;

    switch ( display_mode ) {
    case VPP_DISPLAY_MODE_169:
        value = SCREEN_MODE_16_9;
        break;
    case VPP_DISPLAY_MODE_MODE43:
        value = SCREEN_MODE_4_3;
        break;
    case VPP_DISPLAY_MODE_NORMAL:
        value = SCREEN_MODE_NORMAL;
        break;
    case VPP_DISPLAY_MODE_FULL:
        value = SCREEN_MODE_NONLINEAR;
        Cpq_SetNonLinearFactor(20);
        break;
    case VPP_DISPLAY_MODE_NOSCALEUP:
        value = SCREEN_MODE_NORMAL_NOSCALEUP;
        break;
    case VPP_DISPLAY_MODE_FULL_STRETCH:
        value = SCREEN_MODE_FULL_STRETCH;
        break;
    case VPP_DISPLAY_MODE_43_IGNORE:
        value = SCREEN_MODE_4_3_IGNORE;
        break;
    case VPP_DISPLAY_MODE_43_LETTER_BOX:
        value = SCREEN_MODE_4_3_LETTER_BOX;
        break;
    case VPP_DISPLAY_MODE_43_PAN_SCAN:
        value = SCREEN_MODE_4_3_PAN_SCAN;
        break;
    case VPP_DISPLAY_MODE_43_COMBINED:
        value = SCREEN_MODE_4_3_COMBINED;
        break;
    case VPP_DISPLAY_MODE_169_IGNORE:
        value = SCREEN_MODE_16_9_IGNORE;
        break;
    case VPP_DISPLAY_MODE_169_LETTER_BOX:
        value = SCREEN_MODE_16_9_LETTER_BOX;
        break;
    case VPP_DISPLAY_MODE_169_PAN_SCAN:
        value = SCREEN_MODE_16_9_PAN_SCAN;
        break;
    case VPP_DISPLAY_MODE_169_COMBINED:
        value = SCREEN_MODE_16_9_COMBINED;
        break;
    case VPP_DISPLAY_MODE_MOVIE:
    case VPP_DISPLAY_MODE_PERSON:
    case VPP_DISPLAY_MODE_CAPTION:
    case VPP_DISPLAY_MODE_CROP:
    case VPP_DISPLAY_MODE_CROP_FULL:
    case VPP_DISPLAY_MODE_ZOOM:
    default:
        value = SCREEN_MODE_FULL_STRETCH;
        break;
    }

    return value;
}

int CPQControl::Cpq_SetVideoScreenMode(int value)
{
    SYS_LOGD("%s: %d\n", __FUNCTION__, value);

    char val[64] = {0};
    sprintf(val, "%d", value);
    if (isFileExist(pqSysWrite->getSysNode(VIDEO_SCREEN_MODE_PIP))) {
        pqWriteSys(VIDEO_SCREEN_MODE_PIP, val);
    }

    return pqWriteSys(VIDEO_SCREEN_MODE, val);
}

int CPQControl::Cpq_SetVideoCrop(int Voffset0, int Hoffset0, int Voffset1, int Hoffset1)
{
    SYS_LOGD("%s: %d %d %d %d\n", __FUNCTION__, Voffset0, Hoffset0, Voffset1, Hoffset1);

    char set_str[32];
    memset(set_str, 0, 32);
    sprintf(set_str, "%d %d %d %d", Voffset0, Hoffset0, Voffset1, Hoffset1);
    return pqWriteSys(VIDEO_CROP, set_str);
}

int CPQControl::Cpq_SetNonLinearFactor(int value)
{
    SYS_LOGD("%s: %d\n", __FUNCTION__, value);

    char val[64] = {0};
    sprintf(val, "%d", value);
    return pqWriteSys(VIDEO_NONLINEAR_FACTOR, val);
}

//Backlight
int CPQControl::SetBacklight(int value, int index, int is_save)
{
    int ret = -1;
    SYS_LOGD("%s: index = %d, value = %d\n", __FUNCTION__, index, value);
    if (value < 0 || value > 100) {
        value = DEFAULT_BACKLIGHT_BRIGHTNESS;
    }

    if (is_save == 1) {
        ret = SaveBacklight(value, index);
    }

    if (isFileExist(LDIM_PATH)) {//local diming
        int temp = (value * 255 / 100);
        ret = Cpq_SetBackLight(temp, index);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::GetBacklight(int index)
{
    int data = 0;
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return data;
    }

    if (index == 1)
        data = pData.Backlight.val_display;
    else if (index == 2)
        data = pData.Backlight.val_display1;
    else if (index == 3)
        data = pData.Backlight.val_display2;

    if (data < 0 || data > 100) {
        data = DEFAULT_BACKLIGHT_BRIGHTNESS;
    }

    return data;
}

int CPQControl::SaveBacklight(int value, int index)
{
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    if (index == 1)
        pData.Backlight.val_display = value;
    else if (index == 2)
        pData.Backlight.val_display1 = value;
    else if (index == 3)
        pData.Backlight.val_display2 = value;

    return 0;
}

int CPQControl::Cpq_SetBackLight(int value, int index)
{
    unsigned int temp = value;
    int ret = 0;
    if (index == 1)
        ret = write_backlight_value(&temp);
    else if (index == 2)
        ret = write_backlight2_value(&temp);
    else if (index == 3)
        ret = write_backlight3_value(&temp);

     if (ret == 0)
        SYS_LOGV("%s:succeed; index = %d, value = %d\n", __FUNCTION__, index, temp);
     else
        SYS_LOGV("%s:fail; index = %d, ret = %d\n", __FUNCTION__, index, ret);

     return ret;
}

void CPQControl::Cpq_GetBacklight(int *value, int index)
{
    int ret = 0;
    unsigned int temp = 0;
    if (index == 1)
        ret = read_backlight_value(&temp);
    else if (index == 2)
        ret =   read_backlight2_value(&temp);
    else if (index == 3)
        ret = read_backlight3_value(&temp);

    if (ret == 0) {
        SYS_LOGV("%s:succeed; index = %d, value = %d\n", __FUNCTION__, index, temp);
    } else {
        SYS_LOGV("%s:fail; index = %d, ret = %d\n", __FUNCTION__, index, ret);
    }

    *value = temp;
}

void CPQControl::Set_Backlight(int value)
{
    Cpq_SetBackLight(value, 1);
}

int CPQControl::read_backlight_value(unsigned int *temp)
{
    if (!temp)
        return -ENOBUFS;

    int bldev = open(VOUT_DEV, O_RDONLY);
    if (bldev < 0) {
        return -EBADFD;
    }

    if (ioctl(bldev, VOUT_IOC_CMD_GET_BL_BRIGHTNESS, (unsigned long)temp) != 0) {
        close(bldev);
        return -EINVAL;
    }

    close(bldev);
    return 0;
}

int CPQControl::read_backlight2_value(unsigned int *temp)
{
    if (!temp)
        return -ENOBUFS;

    int bldev = open(VOUT_DEV2, O_RDONLY);
    if (bldev < 0) {
        return -EBADFD;
    }

    if (ioctl(bldev, VOUT_IOC_CMD_GET_BL_BRIGHTNESS, (unsigned long)temp) != 0) {
        close(bldev);
        return -EINVAL;
    }

    close(bldev);
    return 0;
}

int CPQControl::read_backlight3_value(unsigned int *temp)
{
    if (!temp)
        return -ENOBUFS;

    int bldev = open(VOUT_DEV3, O_RDONLY);
    if (bldev < 0) {
        return -EBADFD;
    }

    if (ioctl(bldev, VOUT_IOC_CMD_GET_BL_BRIGHTNESS, (unsigned long)temp) != 0) {
        close(bldev);
        return -EINVAL;
    }

    close(bldev);
    return 0;
}

int CPQControl::write_backlight_value(unsigned int *temp)
{
    if (!temp)
        return -ENOBUFS;

    int bldev = open(VOUT_DEV, O_RDWR);
    if (bldev < 0) {
        return -EBADFD;
    }

    if (ioctl(bldev, VOUT_IOC_CMD_SET_BL_BRIGHTNESS, (unsigned long)temp) != 0) {
        close(bldev);
        return -EINVAL;
    }

    close(bldev);
    return 0;
}

int CPQControl::write_backlight2_value(unsigned int *temp)
{
    if (!temp)
        return -ENOBUFS;

    int bldev = open(VOUT_DEV2, O_RDWR);
    if (bldev < 0) {
        return -EBADFD;
    }

    if (ioctl(bldev, VOUT_IOC_CMD_SET_BL_BRIGHTNESS, (unsigned long)temp) != 0) {
        close(bldev);
        return -EINVAL;
    }

    close(bldev);
    return 0;
}

int CPQControl::write_backlight3_value(unsigned int *temp)
{
    if (!temp)
        return -ENOBUFS;

    int bldev = open(VOUT_DEV3, O_RDWR);
    if (bldev < 0) {
        return -EBADFD;
    }

    if (ioctl(bldev, VOUT_IOC_CMD_SET_BL_BRIGHTNESS, (unsigned long)temp) != 0) {
        close(bldev);
        return -EINVAL;
    }

    close(bldev);
    return 0;
}

//dynamic backlight
int CPQControl::SetDynamicBacklight(Dynamic_backlight_status_t mode, int is_save)
{
    SYS_LOGD("%s, mode = %d\n",__FUNCTION__, mode);
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, (PICTURE_MODE)GetPQMode())) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d data: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return -1;
    }

    para.DynamicBacklight = (int)mode;

    if (is_save == 1) {
        if (!SetPictureModeData(&para, (PICTURE_MODE)GetPQMode())) {
            SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d, data: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
            return -1;
        }
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::GetDynamicBacklight()
{
    int mode = -1;
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, (PICTURE_MODE)GetPQMode())) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d data: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return mode;
    }

    mode = para.DynamicBacklight;

    return mode;
    SYS_LOGD("%s: value is %d\n", __FUNCTION__, mode);
}

int CPQControl::DynamicBackLightInit(void)
{
    int ret = 0;
    Dynamic_backlight_status_t mode = (Dynamic_backlight_status_t)GetDynamicBacklight();
    ret = SetDynamicBacklight(mode, 1);

    if (!isFileExist(LDIM_PATH)) {
        if (isFileExist(pqSysWrite->getSysNode(BACKLIGHT_AML_BL_BRIGHTNESS))) {
            mDynamicBackLight = sp<CDynamicBackLight>::make();
            mDynamicBackLight->setObserver(this);
            mDynamicBackLight->startDected();
        } else {
            SYS_LOGD("No auto backlight module!\n");
        }
    }

    return ret;
}

int CPQControl::GetHistParam(ve_hist_t *hist)
{
    memset(hist, 0, sizeof(ve_hist_s));
    int ret = VPPDeviceIOCtl(AMVECM_IOC_G_HIST_AVG, hist);
    if (ret < 0) {
        //SYS_LOGE("GetAVGHistParam, error(%s)!\n", strerror(errno));
        hist->ave = -1;
    }

    return ret;
}

void CPQControl::GetDynamicBacklighConfig(int *thtf, int *lut_mode, int *height_param, int *low_param)
{
    *thtf = mPQConfigFile->GetInt(CFG_SECTION_BACKLIGHT, CFG_AUTOBACKLIGHT_THTF, 0);
    *lut_mode = mPQConfigFile->GetInt(CFG_SECTION_BACKLIGHT, CFG_AUTOBACKLIGHT_LUTMODE, 1);

    const char *buf = NULL;
    buf = mPQConfigFile->GetString(CFG_SECTION_BACKLIGHT, CFG_AUTOBACKLIGHT_LUTHIGH, NULL);
    pqTransformStringToInt(buf, height_param);

    buf = mPQConfigFile->GetString(CFG_SECTION_BACKLIGHT, CFG_AUTOBACKLIGHT_LUTLOW, NULL);
    pqTransformStringToInt(buf, low_param);
}

void CPQControl::GetDynamicBacklighParam(dynamic_backlight_Param_t *DynamicBacklightParam)
{
    int value = 0;
    ve_hist_t hist;
    memset(&hist, 0, sizeof(ve_hist_t));
    GetHistParam(&hist);
    DynamicBacklightParam->hist.ave = hist.ave;
    DynamicBacklightParam->hist.sum = hist.sum;
    DynamicBacklightParam->hist.width = hist.width;
    DynamicBacklightParam->hist.height = hist.height;

    Cpq_GetBacklight(&value, 1);
    DynamicBacklightParam->CurBacklightValue = value;
    DynamicBacklightParam->UiBackLightValue = GetBacklight(1);
    DynamicBacklightParam->CurDynamicBacklightMode = (Dynamic_backlight_status_t)GetDynamicBacklight();
    DynamicBacklightParam->VideoStatus = GetVideoPlayStatus();
}

int CPQControl::GetVideoPlayStatus(void)
{
    int curVideoState = 0;
    /*int offset = 0;
    char vframeMap[1024] = {0};
    char tmp[1024] = {0};
    char *findRet = NULL;
    char findStr1[20] = "provider";
    char findStr2[20] = "ionvideo";
    char findStr3[20] = "deinterlace(1)";
    int readRet =  pqReadSys(VFM_MAP, tmp, sizeof(tmp));
    strcpy(vframeMap, tmp);
    if (readRet > 0) {
        findRet = strstr(vframeMap, findStr1);
        if (findRet) {
            offset = findRet - vframeMap;
            memset(tmp, 0, sizeof(tmp));
            strncpy(tmp, vframeMap, offset);
            if (strstr(tmp, findStr2) || strstr(tmp, findStr3)) {
                curVideoState = 1;
            } else {
                curVideoState = 0;
            }
        }
    }*/

    if (mbVideoIsPlaying) {
        curVideoState = 1;//video playing
    } else {
        curVideoState = 0;//video stopping
    }

    //SYS_LOGD("%s: curVideoState = %d!\n",__FUNCTION__, curVideoState);
    return curVideoState;
}

int CPQControl::SetLocalContrastMode(local_contrast_mode_t mode, int is_save)
{
    SYS_LOGD("%s: mode is %d!\n",__FUNCTION__, mode);
    int ret = -1;
    ret = Cpq_SetLocalContrastMode(mode);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveLocalContrastMode(mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetLocalContrastMode(void)
{
    int mode = LOCAL_CONTRAST_MODE_MID;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return mode;
    }

    mode = para.LocalContrast;

    if (mode < LOCAL_CONTRAST_MODE_OFF || mode > LOCAL_CONTRAST_MODE_MAX) {
        mode = LOCAL_CONTRAST_MODE_MID;
    }

    SYS_LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, mode);
    return mode;
}

int CPQControl::SaveLocalContrastMode(local_contrast_mode_t mode)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return -1;
    }

    para.LocalContrast = (int)mode;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetLocalContrastMode(local_contrast_mode_t mode)
{
    if (!mbDatabaseMatchChipStatus) {
        SYS_LOGE("%s: pq.db don't match chip!\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_local_contrast_enable) {
        SYS_LOGD("%s: local contrast module disabled!\n",__FUNCTION__);
        return 0;
    }

    ve_lc_curve_parm_t lc_param;
    memset(&lc_param, 0x0, sizeof(ve_lc_curve_parm_t));
    if (mPQdb->PQ_GetLocalContrastNodeParams(mCurrentSourceInputInfo, mode, &lc_param) < 0) {
        SYS_LOGE("%s: PQ_GetLocalContrastNodeParams failed!\n", __FUNCTION__ );
        return -1;
    }
    if (VPPDeviceIOCtl(AMVECM_IOC_S_LC_CURVE, &lc_param) < 0) {
        SYS_LOGE("%s: VPPDeviceIOCtl failed!\n", __FUNCTION__ );
        return -1;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetLocalContrastRegParams(mCurrentSourceInputInfo, mode, &regs) < 0) {
        SYS_LOGE("%s: PQ_GetLocalContrastRegParams failed!\n", __FUNCTION__ );
        return -1;
    }
    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s: Cpq_LoadRegs failed!\n", __FUNCTION__ );
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::SetMpegNr(vpp_pq_level_t mode, int is_save)
{
    SYS_LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    int ret = -1;

    ret = Cpq_SetMpegNr(mode, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveMpegNr(mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed\n", __FUNCTION__);
    } else {
        SYS_LOGD("%s success\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::GetMpegNr(void)
{
    int mode = VPP_PQ_LV_OFF;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return mode;
    }

    mode = para.MpegNr;

    SYS_LOGD("%s: MpegNr = %d \n", __FUNCTION__, mode);
    return mode;
}

int CPQControl::SaveMpegNr(vpp_pq_level_t mode)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return -1;
    }

    para.MpegNr = (int)mode;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return -1;
    }

    SYS_LOGD("%s success!\n", __FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetMpegNr(vpp_pq_level_t mode, source_input_param_t source_input_param)
{
    int ret = 0;
    ret |= Cpq_SetDeblockMode((di_deblock_mode_t)mode, source_input_param);
    ret |= Cpq_SetDemoSquitoMode((di_demosquito_mode_t)mode, source_input_param);

    if (ret < 0)
        SYS_LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::SetDeblockMode(di_deblock_mode_t mode, int is_save)
{
    SYS_LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    int ret = -1;
    ret = Cpq_SetDeblockMode(mode, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveDeblockMode(mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed\n", __FUNCTION__);
    } else {
        SYS_LOGD("%s success\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::GetDeblockMode(void)
{
    int mode = DI_DEBLOCK_MODE_OFF;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return mode;
    }

    mode = para.Deblock;

    if (mode < DI_DEBLOCK_MODE_OFF || mode > DI_DEBLOCK_MODE_AUTO)
        mode = DI_DEBLOCK_MODE_OFF;

    return mode;
}

int CPQControl::SaveDeblockMode(di_deblock_mode_t mode)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return -1;
    }

    para.Deblock = (int)mode;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return -1;
    }

    SYS_LOGD("%s success\n", __FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetDeblockMode(di_deblock_mode_t deblock_mode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_deblock_enable) {
        SYS_LOGD("%s: deblock disabled\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetDeblockParams((di_deblock_mode_t)deblock_mode, source_input_param, &regs) < 0) {
        SYS_LOGE("PQ_GetDeblockParams failed!\n");
        return -1;
    }

    am_pq_param_t di_regs;
    memset(&di_regs, 0x0, sizeof(am_pq_param_t));
    di_regs.table_name = TABLE_NAME_DEBLOCK;
    di_regs.table_len = regs.length;
    am_reg_t tmp_buf[regs.length];
    for (unsigned int i = 0; i < regs.length; i++) {
          tmp_buf[i].addr = regs.am_reg[i].addr;
          tmp_buf[i].mask = regs.am_reg[i].mask;
          tmp_buf[i].type = regs.am_reg[i].type;
          tmp_buf[i].val  = regs.am_reg[i].val;
    }
    di_regs.table_ptr = (long long)tmp_buf;

    if (DI_LoadRegs(di_regs) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::SetDemoSquitoMode(di_demosquito_mode_t mode, int is_save)
{
    SYS_LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    int ret = -1;
    ret = Cpq_SetDemoSquitoMode(mode, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveDemoSquitoMode(mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed\n", __FUNCTION__);
    } else {
        SYS_LOGD("%s success\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::GetDemoSquitoMode(void)
{
    int mode = DI_DEMOSQUITO_MODE_OFF;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return mode;
    }

    mode = para.DeMoSquito;

    if (mode < DI_DEMOSQUITO_MODE_OFF || mode > DI_DEMOSQUITO_MODE_AUTO)
        mode = DI_DEMOSQUITO_MODE_OFF;

    return mode;
}

int CPQControl::SaveDemoSquitoMode(di_demosquito_mode_t mode)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return -1;
    }

    para.DeMoSquito = (int)mode;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return -1;
    }

    SYS_LOGD("%s success\n", __FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetDemoSquitoMode(di_demosquito_mode_t DeMosquito_mode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_demoSquito_enable) {
        SYS_LOGD("%s: demosquito disabled\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetDemoSquitoParams(DeMosquito_mode, source_input_param, &regs) < 0) {
        SYS_LOGE("DemoSquitoMode failed!\n");
        return -1;
    }

    am_pq_param_t di_regs;
    memset(&di_regs, 0x0, sizeof(am_pq_param_t));
    di_regs.table_name = TABLE_NAME_DEMOSQUITO;
    di_regs.table_len = regs.length;
    am_reg_t tmp_buf[regs.length];
    for (unsigned int i = 0; i < regs.length; i++) {
          tmp_buf[i].addr = regs.am_reg[i].addr;
          tmp_buf[i].mask = regs.am_reg[i].mask;
          tmp_buf[i].type = regs.am_reg[i].type;
          tmp_buf[i].val  = regs.am_reg[i].val;
    }
    di_regs.table_ptr = (long long)tmp_buf;

    if (DI_LoadRegs(di_regs) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::SetMcDiMode(vpp_mcdi_mode_e mode, int is_save)
{
    SYS_LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    int ret = -1;
    ret = Cpq_SetMcDiMode(mode , mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveMcDiMode(mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed\n", __FUNCTION__);
    } else {
        SYS_LOGD("%s success\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::GetMcDiMode(void)
{
    int mode = VPP_MCDI_MODE_OFF;
    PICTURE_SETTING_BY_SRC pData;
    if (!GetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s GetPictureStructDataBySrc failed\n", __FUNCTION__);
        return mode;
    }

    mode = pData.McDiMode;

    if (mode < VPP_MCDI_MODE_OFF || mode >= VPP_MCDI_MODE_MAX)
        mode = VPP_MCDI_MODE_OFF;

    SYS_LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    return mode;
}

int CPQControl::SaveMcDiMode(vpp_mcdi_mode_e mode)
{
    PICTURE_SETTING_BY_SRC pData;
    if (!GetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s GetPictureStructDataBySrc failed\n", __FUNCTION__);
        return -1;
    }

    pData.McDiMode = (int)mode;

    if (!SetPictureStructDataBySrc(&pData)) {
        SYS_LOGE("%s SetPictureStructDataBySrc failed\n", __FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success\n", __FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetMcDiMode(vpp_mcdi_mode_e McDi_mode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_mcdi_enable) {
        SYS_LOGD("%s: McDi disabled\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetMCDIParams(McDi_mode, source_input_param, &regs) < 0) {
        SYS_LOGE("MCDI failed!\n");
        return -1;
    }

    am_pq_param_t di_regs;
    memset(&di_regs, 0x0,sizeof(am_pq_param_t));
    di_regs.table_name = TABLE_NAME_MCDI;
    di_regs.table_len = regs.length;
    am_reg_t tmp_buf[regs.length];
    for (unsigned int i = 0; i < regs.length; i++) {
          tmp_buf[i].addr = regs.am_reg[i].addr;
          tmp_buf[i].mask = regs.am_reg[i].mask;
          tmp_buf[i].type = regs.am_reg[i].type;
          tmp_buf[i].val  = regs.am_reg[i].val;
    }
    di_regs.table_ptr = (long long)tmp_buf;

    if (DI_LoadRegs(di_regs) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

//static frame
int CPQControl::SetStaticFrameEnable(int enable, int isSave)
{
    SYS_LOGD("%s: StaticFrameEnable status is %d.\n", __FUNCTION__, enable);
    int ret = -1;
    if (enable == 1) {
        //ret = pqWriteSys(VIDEO_BLACKOUT_POLICY, "0");
        //enable static frame output
        ret = property_set(STATIC_FRAME_ENABLE_PROP, "1");
    } else {
        //ret = pqWriteSys(VIDEO_BLACKOUT_POLICY, "1");
        //disable static frame output
        ret = property_set(STATIC_FRAME_ENABLE_PROP, "0");
    }

    //if ((ret == 0) && (isSave == 1)) {
    if (isSave == 1) {
        Cpq_SSMWriteNTypes(SSM_RW_BLACKOUT_ENABLE_START, 1, enable, 0);
    }

    return 0;
}

int CPQControl::GetStaticFrameEnable()
{
    int ret = -1;
    int value = Cpq_SSMReadNTypes(SSM_RW_BLACKOUT_ENABLE_START, 1, 0);
    if (value < 0) {
        SYS_LOGE("%s failed.\n", __FUNCTION__);
        ret = 0;
    } else {
        ret = value;
    }

    SYS_LOGD("%s: StaticFrameEnable status is %d.\n", __FUNCTION__, ret);
    return ret;
}

int CPQControl::getSnowStatus()
{
    int ret = -1;
    char buf[8] = {0};

    ret = pqReadSys(VDIN_SNOW_FLAG, buf, sizeof(buf));
    if (ret > 0) {
        ret = strtol(buf, NULL, 10);
    } else {
        ret = 0;
    }

    return ret;
}

//screen color
int CPQControl::SetScreenColorForSignalChange(int screenColor, int isSave)
{
    SYS_LOGD("%s: screenColor = %s\n", __FUNCTION__, (screenColor==0)?"black":"blue");

    if (getSnowStatus() == 1) {
        if (screenColor == VIDEO_LAYER_COLOR_BLUE) {
            setVideoScreenColor(screenColor);
        } else {
            setVideoScreenColor(3);
        }
    } /*else if (screenColorEnable) {
        setVideoScreenColor(screenColor);
    }*/
    /*
    if (screenColor == 0) {//black screen
        SetVideoLayerColor(VIDEO_LAYER_COLOR_BLACK, VIDEO_LAYER_COLOR_BLACK);
    } else {//blue screen
        SetVideoLayerColor(VIDEO_LAYER_COLOR_BLACK, VIDEO_LAYER_COLOR_BLUE);
    }
    */
    if (isSave == 1) {
        Cpq_SSMWriteNTypes(CUSTOMER_DATA_POS_SCREEN_COLOR_START, 1, screenColor, 0);
    }

    return 0;
}

int CPQControl::GetScreenColorForSignalChange()
{
    int ret = -1;
    int value = Cpq_SSMReadNTypes(CUSTOMER_DATA_POS_SCREEN_COLOR_START, 1, 0);
    if (value < 0) {
        SYS_LOGE("%s failed.\n", __FUNCTION__);
        ret = 0;
    } else {
        ret = value;
    }
    SYS_LOGD("%s: status is %d.\n", __FUNCTION__, ret);
    return ret;
}

int CPQControl::SetVideoLayerColor(video_layer_color_t signalColor, video_layer_color_t nosignalColor)
{
    int ret = 0;
    char val[64] = {0};
    int value_y=0, value_u=0, value_v=0;
    switch (nosignalColor) {
        case VIDEO_LAYER_COLOR_BLUE:
            value_y = 41;
            value_u = 240;
            value_v = 110;
            break;
        case VIDEO_LAYER_COLOR_BLACK:
            value_y = 16;
            value_u = 128;
            value_v = 128;
            break;
        default:
            value_y = 16;
            value_u = 128;
            value_v = 128;
            break;
    }
    unsigned long signalColorValue = (1 << 24) | (16 << 16 ) | (128 << 8) | (128);//default black
    unsigned long nosignalColorValue = 1 << 24;
    nosignalColorValue |= (unsigned int)(value_y << 16) | (unsigned int) (value_u << 8) | (unsigned int)value_v;
    sprintf(val, "0x%lx 0x%lx", signalColorValue, nosignalColorValue);
    ret = pqWriteSys(VIDEO_BACKGROUND_COLOR, val);
    return ret;
}

int CPQControl::setVideoScreenColor (int vdin_blending_mask, int y, int u, int v)
{
    int ret = 0;
    unsigned long value = vdin_blending_mask << 24;
    value |= ( unsigned int ) ( y << 16 ) | ( unsigned int ) ( u << 8 ) | ( unsigned int ) ( v );

    char val[64] = {0};
    sprintf(val, "0x%lx", ( unsigned long ) value);
    ret = pqWriteSys(VIDEO_TEST_SCREEN, val);
    return ret;
}

int CPQControl::setVideoScreenColor (int color)
{
    SYS_LOGD("%s:  %d\n", __FUNCTION__, color);
    int ret = 0;
    switch (color) {
        case VIDEO_LAYER_COLOR_BLUE:
            pqWriteSys(VIDEO_DISABLE_VIDEO, "1");
            ret = setVideoScreenColor(0, 41, 240, 110);
            screenColorEnable = true;
            break;
        case VIDEO_LAYER_COLOR_BLACK:
            pqWriteSys(VIDEO_DISABLE_VIDEO, "1");
            ret = setVideoScreenColor(0, 16, 128, 128);
            screenColorEnable = true;
            break;
        default:
            ret = setVideoScreenColor(0, 16, 128, 128);
            pqWriteSys(VIDEO_DISABLE_VIDEO, "2");
            screenColorEnable = false;
            break;
    }
    return ret;
}

int CPQControl::setVideoScreenColorByVT( int window, int Color, int frequency)
{
    return SetVideotunnelSolidColor((video__color_Window)window, (video_color_frame)Color, (video_color_frame_time)frequency);
}

int CPQControl::OpenVideotunnel()
{
    int ret = -1;
    ret = meson_vt_open();
    if (ret < 0) {
        SYS_LOGE("%s: open meson_vt error!",__FUNCTION__);
        return ret;
    }
    mVideoTunelFd = ret;
    return mVideoTunelFd;
}

int CPQControl::CloseVideotunnel()
{
    int ret = -1;
    if (mVideoTunelFd >= 0) {
        ret = meson_vt_close(mVideoTunelFd);
        mVideoTunelFd = -1;
    } else {
        SYS_LOGD("%s: needn't close meson_vt!",__FUNCTION__);
    }
    return ret;
}

int CPQControl::SetVideotunnelSolidColor(video__color_Window window, video_color_frame cmd, video_color_frame_time cmd_data)
{
    int ret = -1;
    if (mVideoTunelFd < 0) {
        SYS_LOGD("%s: Video tunnel not yet opened!",__FUNCTION__);
        return ret;
    }
    SYS_LOGD("%s: window:%d, color:%d, times:%d", __FUNCTION__, window, cmd,cmd_data);
    ret = meson_vt_set_solid_color(mVideoTunelFd, window, (vt_color_cmd)cmd, (vt_color_data)cmd_data);
    return ret;
}

tvin_cutwin_t CPQControl::GetOverscanParams(vpp_display_mode_t display_mode)
{
    int ret = -1;
    tvin_cutwin_t cutwin_t;
    memset(&cutwin_t, 0, sizeof(cutwin_t));

    SYS_LOGD("%s:display_mode=%d source=%d,sigFmt=%d(0x%x)\n", __FUNCTION__,
                                                                 display_mode,
                                                                 mCurrentSourceInputInfo.source_input,
                                                                 mCurrentSourceInputInfo.sig_fmt,
                                                                 mCurrentSourceInputInfo.sig_fmt);

    if (mbCpqCfg_separate_db_enable) {
        ret = mpOverScandb->PQ_GetOverscanParams(mCurrentSourceInputInfo, display_mode, &cutwin_t);
    }

    if (ret != 0) {
        SYS_LOGE("%s failed\n", __FUNCTION__);
    } else {
        SYS_LOGD("%s success\n", __FUNCTION__);
    }

    SYS_LOGD("he:%d hs:%d ve:%d vs:%d\n", cutwin_t.he, cutwin_t.hs, cutwin_t.ve, cutwin_t.vs);

    return cutwin_t;
}

//PQ Factory
int CPQControl::FactoryResetPQMode(void)
{
    ResetPictureModeDataAll();
    ResetPictureModeAll();
    return 0;
}

int CPQControl::FactoryResetColorTemp(void)
{
    ResetColorTemperatureDataAll();
    return 0;
}

int CPQControl::FactorySetPQMode_Brightness(source_input_param_t source_input_param, int pq_mode, int brightness)
{
    pq_source_input_t source = CheckPQSource(source_input_param.source_input);
    PICTURE_MODE_DATA pData;
    if (!GetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s GetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    pData.Brightness = brightness;

    if (!SetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s SetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetPQMode_Brightness(source_input_param_t source_input_param, int pq_mode)
{
    pq_source_input_t source = CheckPQSource(source_input_param.source_input);
    PICTURE_MODE_DATA pData;
    if (!GetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s GetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    return pData.Brightness;
}

int CPQControl::FactorySetPQMode_Contrast(source_input_param_t source_input_param, int pq_mode, int contrast)
{
    pq_source_input_t source = CheckPQSource(source_input_param.source_input);
    PICTURE_MODE_DATA pData;
    if (!GetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s GetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    pData.Contrast = contrast;

    if (!SetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s SetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetPQMode_Contrast(source_input_param_t source_input_param, int pq_mode)
{
    pq_source_input_t source = CheckPQSource(source_input_param.source_input);
    PICTURE_MODE_DATA pData;
    if (!GetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s GetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    return pData.Contrast;
}

int CPQControl::FactorySetPQMode_Saturation(source_input_param_t source_input_param, int pq_mode, int saturation)
{
    pq_source_input_t source = CheckPQSource(source_input_param.source_input);
    PICTURE_MODE_DATA pData;
    if (!GetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s GetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    pData.Saturation = saturation;

    if (!SetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s SetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetPQMode_Saturation(source_input_param_t source_input_param, int pq_mode)
{
    pq_source_input_t source = CheckPQSource(source_input_param.source_input);
    PICTURE_MODE_DATA pData;
    if (!GetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s GetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    return pData.Saturation;
}

int CPQControl::FactorySetPQMode_Hue(source_input_param_t source_input_param, int pq_mode, int hue)
{
    pq_source_input_t source = CheckPQSource(source_input_param.source_input);
    PICTURE_MODE_DATA pData;
    if (!GetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s GetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    pData.Hue = hue;

    if (!SetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s SetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetPQMode_Hue(source_input_param_t source_input_param, int pq_mode)
{
    pq_source_input_t source = CheckPQSource(source_input_param.source_input);
    PICTURE_MODE_DATA pData;
    if (!GetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s GetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    return pData.Hue;
}

int CPQControl::FactorySetPQMode_Sharpness(source_input_param_t source_input_param, int pq_mode, int sharpness)
{
    pq_source_input_t source = CheckPQSource(source_input_param.source_input);
    PICTURE_MODE_DATA pData;
    if (!GetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s GetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    pData.Sharpness = sharpness;

    if (!SetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s SetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetPQMode_Sharpness(source_input_param_t source_input_param, int pq_mode)
{
    pq_source_input_t source = CheckPQSource(source_input_param.source_input);
    PICTURE_MODE_DATA pData;
    if (!GetPictureModeCustomData(&pData, (PICTURE_MODE)pq_mode, source, CurTimming)) {
        SYS_LOGE("%s GetPictureModeCustomData failed\n", __FUNCTION__);
        return -1;
    }

    return pData.Sharpness;
}

int CPQControl::FactorySetColorTemp_Rgain(int source_input,int colortemp_mode, int rgain)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, colortemp_mode)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n",__FUNCTION__);
        return -1;
    }

    if (!FactoryGetWhitebalanceRGBGainOffsetData(&ColorTemp.rgbgo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    tcon_rgb_ogo_t rgbogo;
    rgbogo.en = 1;
    rgbogo.r_gain = rgain + ColorTemp.ColorTempOffset.r_gain_value;
    rgbogo.g_gain = ColorTemp.rgbgo.GGAIN + ColorTemp.ColorTempOffset.g_gain_value;
    rgbogo.b_gain = ColorTemp.rgbgo.BGAIN + ColorTemp.ColorTempOffset.b_gain_value;
    rgbogo.r_post_offset = ColorTemp.rgbgo.ROFFSET + ColorTemp.ColorTempOffset.r_offset_value;
    rgbogo.g_post_offset = ColorTemp.rgbgo.GOFFSET + ColorTemp.ColorTempOffset.g_offset_value;
    rgbogo.b_post_offset = ColorTemp.rgbgo.BOFFSET + ColorTemp.ColorTempOffset.b_offset_value;
    rgbogo.r_pre_offset = 0;
    rgbogo.g_pre_offset = 0;
    rgbogo.b_pre_offset = 0;

    SYS_LOGD("%s, source[%d], colortemp_mode[%d], rgain[%d].", __FUNCTION__, source_input, colortemp_mode, rgain);

    if (Cpq_SetRGBOGO(&rgbogo) == 0) {
        return 0;
    }

    return -1;
}

int CPQControl::FactorySaveColorTemp_Rgain(int source_input __unused, int colortemp_mode, int rgain)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    RGB_GAIN_OFFSET rgbogo;
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    rgbogo.BGAIN = rgain;

    if (!FactorySetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactorySetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetColorTemp_Rgain(int source_input __unused, int colortemp_mode)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    RGB_GAIN_OFFSET rgbogo;
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    return rgbogo.RGAIN;
}

int CPQControl::FactorySetColorTemp_Ggain(int source_input, int colortemp_mode, int ggain)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, colortemp_mode)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n",__FUNCTION__);
        return -1;
    }

    if (!FactoryGetWhitebalanceRGBGainOffsetData(&ColorTemp.rgbgo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    tcon_rgb_ogo_t rgbogo;
    rgbogo.en = 1;
    rgbogo.r_gain = ColorTemp.rgbgo.RGAIN + ColorTemp.ColorTempOffset.r_gain_value;
    rgbogo.g_gain = ggain + ColorTemp.ColorTempOffset.g_gain_value;
    rgbogo.b_gain = ColorTemp.rgbgo.BGAIN + ColorTemp.ColorTempOffset.b_gain_value;
    rgbogo.r_post_offset = ColorTemp.rgbgo.ROFFSET + ColorTemp.ColorTempOffset.r_offset_value;
    rgbogo.g_post_offset = ColorTemp.rgbgo.GOFFSET + ColorTemp.ColorTempOffset.g_offset_value;
    rgbogo.b_post_offset = ColorTemp.rgbgo.BOFFSET + ColorTemp.ColorTempOffset.b_offset_value;
    rgbogo.r_pre_offset = 0;
    rgbogo.g_pre_offset = 0;
    rgbogo.b_pre_offset = 0;

    SYS_LOGD("%s, source[%d], colortemp_mode[%d], ggain[%d].", __FUNCTION__, source_input, colortemp_mode, ggain);
    if (Cpq_SetRGBOGO(&rgbogo) == 0) {
        return 0;
    }

    return -1;
}

int CPQControl::FactorySaveColorTemp_Ggain(int source_input __unused, int colortemp_mode, int ggain)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    RGB_GAIN_OFFSET rgbogo;
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    rgbogo.GGAIN = ggain;

    if (!FactorySetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetColorTemp_Ggain(int source_input __unused, int colortemp_mode)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    RGB_GAIN_OFFSET rgbogo;
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    return rgbogo.GGAIN;
}

int CPQControl::FactorySetColorTemp_Bgain(int source_input, int colortemp_mode, int bgain)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, colortemp_mode)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n",__FUNCTION__);
        return -1;
    }

    if (!FactoryGetWhitebalanceRGBGainOffsetData(&ColorTemp.rgbgo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    tcon_rgb_ogo_t rgbogo;
    rgbogo.en = 1;
    rgbogo.r_gain = ColorTemp.rgbgo.RGAIN + ColorTemp.ColorTempOffset.r_gain_value;
    rgbogo.g_gain = ColorTemp.rgbgo.BGAIN + ColorTemp.ColorTempOffset.g_gain_value;
    rgbogo.b_gain = bgain + ColorTemp.ColorTempOffset.b_gain_value;
    rgbogo.r_post_offset = ColorTemp.rgbgo.ROFFSET + ColorTemp.ColorTempOffset.r_offset_value;
    rgbogo.g_post_offset = ColorTemp.rgbgo.GOFFSET + ColorTemp.ColorTempOffset.g_offset_value;
    rgbogo.b_post_offset = ColorTemp.rgbgo.BOFFSET + ColorTemp.ColorTempOffset.b_offset_value;
    rgbogo.r_pre_offset = 0;
    rgbogo.g_pre_offset = 0;
    rgbogo.b_pre_offset = 0;

    SYS_LOGD("%s, source[%d], colortemp_mode[%d], bgain[%d].", __FUNCTION__, source_input, colortemp_mode, bgain);

    if (Cpq_SetRGBOGO(&rgbogo) == 0) {
        return 0;
    }

    return -1;
}

int CPQControl::FactorySaveColorTemp_Bgain(int source_input __unused, int colortemp_mode, int bgain)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    RGB_GAIN_OFFSET rgbogo;
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    rgbogo.BGAIN = bgain;

    if (!FactorySetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetColorTemp_Bgain(int source_input __unused, int colortemp_mode)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    RGB_GAIN_OFFSET rgbogo;
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    return rgbogo.BGAIN;
}

int CPQControl::FactorySetColorTemp_Roffset(int source_input, int colortemp_mode, int roffset)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, colortemp_mode)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n",__FUNCTION__);
        return -1;
    }

    if (!FactoryGetWhitebalanceRGBGainOffsetData(&ColorTemp.rgbgo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    tcon_rgb_ogo_t rgbogo;
    rgbogo.en = 1;
    rgbogo.r_gain = ColorTemp.rgbgo.RGAIN + ColorTemp.ColorTempOffset.r_gain_value;
    rgbogo.g_gain = ColorTemp.rgbgo.GGAIN + ColorTemp.ColorTempOffset.g_gain_value;
    rgbogo.b_gain = ColorTemp.rgbgo.BGAIN + ColorTemp.ColorTempOffset.b_gain_value;
    rgbogo.r_post_offset = roffset + ColorTemp.ColorTempOffset.r_offset_value;
    rgbogo.g_post_offset = ColorTemp.rgbgo.GOFFSET + ColorTemp.ColorTempOffset.g_offset_value;
    rgbogo.b_post_offset = ColorTemp.rgbgo.BOFFSET + ColorTemp.ColorTempOffset.b_offset_value;
    rgbogo.r_pre_offset = 0;
    rgbogo.g_pre_offset = 0;
    rgbogo.b_pre_offset = 0;

    SYS_LOGD("%s, source[%d], colortemp_mode[%d], r_post_offset[%d].", __FUNCTION__, source_input, colortemp_mode, roffset);

    if (Cpq_SetRGBOGO(&rgbogo) == 0) {
        return 0;
    }

    return -1;
}

int CPQControl::FactorySaveColorTemp_Roffset(int source_input __unused, int colortemp_mode, int roffset)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    RGB_GAIN_OFFSET rgbogo;
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    rgbogo.ROFFSET = roffset;

    if (!FactorySetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetColorTemp_Roffset(int source_input __unused, int colortemp_mode)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    RGB_GAIN_OFFSET rgbogo;
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    return rgbogo.ROFFSET;
}

int CPQControl::FactorySetColorTemp_Goffset(int source_input, int colortemp_mode, int goffset)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, colortemp_mode)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n",__FUNCTION__);
        return -1;
    }

    if (!FactoryGetWhitebalanceRGBGainOffsetData(&ColorTemp.rgbgo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    tcon_rgb_ogo_t rgbogo;
    rgbogo.en = 1;
    rgbogo.r_gain = ColorTemp.rgbgo.RGAIN + ColorTemp.ColorTempOffset.r_gain_value;
    rgbogo.g_gain = ColorTemp.rgbgo.GGAIN + ColorTemp.ColorTempOffset.g_gain_value;
    rgbogo.b_gain = ColorTemp.rgbgo.BGAIN + ColorTemp.ColorTempOffset.b_gain_value;
    rgbogo.r_post_offset = ColorTemp.rgbgo.ROFFSET + ColorTemp.ColorTempOffset.r_offset_value;
    rgbogo.g_post_offset = goffset + ColorTemp.ColorTempOffset.g_offset_value;
    rgbogo.b_post_offset = ColorTemp.rgbgo.BOFFSET + ColorTemp.ColorTempOffset.b_offset_value;
    rgbogo.r_pre_offset = 0;
    rgbogo.g_pre_offset = 0;
    rgbogo.b_pre_offset = 0;

    SYS_LOGD("%s, source[%d], colortemp_mode[%d], g_post_offset[%d].", __FUNCTION__, source_input, colortemp_mode, goffset);

    if (Cpq_SetRGBOGO(&rgbogo) == 0) {
        return 0;
    }

    return -1;
}

int CPQControl::FactorySaveColorTemp_Goffset(int source_input __unused, int colortemp_mode, int goffset)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    RGB_GAIN_OFFSET rgbogo;
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    rgbogo.GOFFSET = goffset;

    if (!FactorySetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactorySetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetColorTemp_Goffset(int source_input __unused, int colortemp_mode)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    RGB_GAIN_OFFSET rgbogo;
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    return rgbogo.GOFFSET;
}

int CPQControl::FactorySetColorTemp_Boffset(int source_input, int colortemp_mode, int boffset)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    COLORTEMP_DATA ColorTemp;
    memset(&ColorTemp, 0, sizeof(COLORTEMP_DATA));
    if (!GetColorTemperatureData(&ColorTemp, colortemp_mode)) {
        SYS_LOGE("%s GetColorTemperatureData fail\n",__FUNCTION__);
        return -1;
    }

    if (!FactoryGetWhitebalanceRGBGainOffsetData(&ColorTemp.rgbgo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    tcon_rgb_ogo_t rgbogo;
    rgbogo.en = 1;
    rgbogo.r_gain = ColorTemp.rgbgo.RGAIN + ColorTemp.ColorTempOffset.r_gain_value;
    rgbogo.g_gain = ColorTemp.rgbgo.GGAIN + ColorTemp.ColorTempOffset.g_gain_value;
    rgbogo.b_gain = ColorTemp.rgbgo.BGAIN + ColorTemp.ColorTempOffset.b_gain_value;
    rgbogo.r_post_offset = ColorTemp.rgbgo.ROFFSET + ColorTemp.ColorTempOffset.r_offset_value;
    rgbogo.g_post_offset = ColorTemp.rgbgo.GOFFSET + ColorTemp.ColorTempOffset.g_offset_value;
    rgbogo.b_post_offset = boffset + ColorTemp.ColorTempOffset.b_offset_value;
    rgbogo.r_pre_offset = 0;
    rgbogo.g_pre_offset = 0;
    rgbogo.b_pre_offset = 0;

    SYS_LOGD("%s, source_input[%d], colortemp_mode[%d], b_post_offset[%d].", __FUNCTION__, source_input, colortemp_mode, boffset);

    if (Cpq_SetRGBOGO(&rgbogo) == 0) {
        return 0;
    }

    return -1;
}

int CPQControl::FactorySaveColorTemp_Boffset(int source_input __unused, int colortemp_mode, int boffset)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    RGB_GAIN_OFFSET rgbogo;
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    rgbogo.BOFFSET = boffset;

    if (!FactorySetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactorySetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetColorTemp_Boffset(int source_input __unused, int colortemp_mode)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    RGB_GAIN_OFFSET rgbogo;
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        SYS_LOGE("%s, FactoryGetWhitebalanceRGBGainOffsetData fail.", __FUNCTION__);
        return -1;
    }

    return rgbogo.BOFFSET;
}

int CPQControl::FactorySetGammaTable(unsigned short *pData, int type, int level, int len)
{
    if (pData == NULL || len > GAMMA_NUMBER) {
        SYS_LOGE("%s invalid value len = %d\n",__FUNCTION__, len);
        return -1;
    }

    if (type >= MAX_CH) {
        SYS_LOGE("%s invalid value type = %d\n",__FUNCTION__, type);
        return -1;
    }

    int i;
    tcon_gamma_table_t Gamma;
    memset(&Gamma, 0, sizeof(tcon_gamma_table_t));
    for (i = 0; i < len; ++i) {
        Gamma.data[i] = pData[i];
    }
    for (; i < GAMMA_NUMBER; ++i) {
        Gamma.data[i] = 1023;
    }

    if (!mWBDataBase->SetGammaTableData(&Gamma, level, type)) {
        SYS_LOGE("%s mWBDataBase->SetGammaTableData faillen = %d\n",__FUNCTION__, len);
        return -1;
    }

    // to driver
    if (type == RED_CH)
        Cpq_SetGammaTbl_R((unsigned short *)Gamma.data);
    else if (type == GREEN_CH)
        Cpq_SetGammaTbl_G((unsigned short *)Gamma.data);
    else if (type == BLUE_CH)
        Cpq_SetGammaTbl_B((unsigned short *)Gamma.data);
    else
        return -1;

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::FactoryResetNonlinear(void)
{
    ResetNonlinearDataAll();
    return 0;
}

int CPQControl::FactorySetParamsDefault(void)
{
    FactoryResetPQMode();
    FactoryResetNonlinear();
    FactoryResetColorTemp();
    if (mbCpqCfg_separate_db_enable) {
        int ret = -1;
        char dstOverscanDbPath[128] = {0};

        ret = mpOverScandb->closeOverScanDB();
        if (ret == 0) {
            ret = unlink(PARAM_OVERSCAN_DB_PATH);
            if (ret == 0) {
                int display_mode = GetDisplayMode();

                mPQConfigFile->GetOverscandbPath(dstOverscanDbPath);
                mpOverScandb->openOverScanDB(dstOverscanDbPath);
                ret = SetDisplayMode((vpp_display_mode_t)display_mode, 0);
            } else
                SYS_LOGE("unlink[%s] faile,ret=%d\n", PARAM_OVERSCAN_DB_PATH, ret);
        } else
            SYS_LOGE("closeOverScanDB err,ret=%d\n", ret);
    } else {
        int ret = -1;
        char dstPqDbPath[128] = {0};

        ret = mPQdb->closePqDB();
        if (ret == 0) {
            ret = unlink(PARAM_PQ_DB_PATH);
            if (ret == 0) {
                int display_mode = GetDisplayMode();

                mPQConfigFile->GetPqdbPath(dstPqDbPath);
                mPQdb->openPqDB(dstPqDbPath);
                ret = SetDisplayMode((vpp_display_mode_t)display_mode, 0);
            }else
                SYS_LOGE("unlink[%s] faile,ret=%d\n", PARAM_PQ_DB_PATH, ret);
        }else
            SYS_LOGE("closeOverScanDB err,ret=%d\n", ret);
    }
    return 0;
}

int CPQControl::FactorySetNolineParams(source_input_param_t source_input_param, int type, noline_params_t noline_params)
{
    pq_source_input_t source = CheckPQSource(source_input_param.source_input);
    NonlinearModeType pData;
    if (!GetNonlinearCustomData(&pData, source, CurTimming)) {
        SYS_LOGE("%s: GetNonlinearCustomData fail\n", __FUNCTION__);
        return -1;
    }

    switch (type) {
    case NOLINE_PARAMS_TYPE_BRIGHTNESS:
        pData.Brightness_0 = noline_params.osd0;
        pData.Brightness_25 = noline_params.osd25;
        pData.Brightness_50 = noline_params.osd50;
        pData.Brightness_75 = noline_params.osd75;
        pData.Brightness_100 = noline_params.osd100;
        break;

    case NOLINE_PARAMS_TYPE_CONTRAST:
        pData.Contrast_0 = noline_params.osd0;
        pData.Contrast_25 = noline_params.osd25;
        pData.Contrast_50 = noline_params.osd50;
        pData.Contrast_75 = noline_params.osd75;
        pData.Contrast_100 = noline_params.osd100;
        break;

    case NOLINE_PARAMS_TYPE_SATURATION:
        pData.Saturation_0 = noline_params.osd0;
        pData.Saturation_25 = noline_params.osd25;
        pData.Saturation_50 = noline_params.osd50;
        pData.Saturation_75 = noline_params.osd75;
        pData.Saturation_100 = noline_params.osd100;
        break;

    case NOLINE_PARAMS_TYPE_HUE:
        pData.Hue_0 = noline_params.osd0;
        pData.Hue_25 = noline_params.osd25;
        pData.Hue_50 = noline_params.osd50;
        pData.Hue_75 = noline_params.osd75;
        pData.Hue_100 = noline_params.osd100;
        break;

    case NOLINE_PARAMS_TYPE_SHARPNESS:
        pData.Sharpness_0 = noline_params.osd0;
        pData.Sharpness_25 = noline_params.osd25;
        pData.Sharpness_50 = noline_params.osd50;
        pData.Sharpness_75 = noline_params.osd75;
        pData.Sharpness_100 = noline_params.osd100;
        break;
    default:
        break;
    }

    if (!SetNonlinearCustomData(&pData, source, CurTimming)) {
        SYS_LOGE("%s: GetNonlinearCustomData fail\n", __FUNCTION__);
        return -1;
    }

    switch (type) {
    case NOLINE_PARAMS_TYPE_BRIGHTNESS:
        Cpq_SetBrightness(GetBrightness(), mCurrentSourceInputInfo);
        break;
    case NOLINE_PARAMS_TYPE_CONTRAST:
        Cpq_SetContrast(GetContrast(), mCurrentSourceInputInfo);
        break;
    case NOLINE_PARAMS_TYPE_SATURATION:
        Cpq_SetSaturation(GetSaturation(), mCurrentSourceInputInfo);
        break;
    case NOLINE_PARAMS_TYPE_HUE:
        Cpq_SetHue(GetHue(), mCurrentSourceInputInfo);
        break;
    case NOLINE_PARAMS_TYPE_SHARPNESS:
        Cpq_SetSharpness(GetSharpness(), mCurrentSourceInputInfo);
        break;
    default:
        break;
    }

    return 0;
}

noline_params_t CPQControl::FactoryGetNolineParams(source_input_param_t source_input_param, int type)
{
    noline_params_t NolinerData;
    memset(&NolinerData, 0, sizeof(noline_params_t));

    pq_source_input_t source = CheckPQSource(source_input_param.source_input);
    NonlinearModeType pData;
    if (!GetNonlinearCustomData(&pData, source, CurTimming)) {
        SYS_LOGE("%s: GetNonlinearCustomData fail\n", __FUNCTION__);
        return NolinerData;
    }

    switch (type) {
    case NOLINE_PARAMS_TYPE_BRIGHTNESS:
        NolinerData.osd0 = pData.Brightness_0;
        NolinerData.osd25 = pData.Brightness_25;
        NolinerData.osd50 = pData.Brightness_50;
        NolinerData.osd75 = pData.Brightness_75;
        NolinerData.osd100 = pData.Brightness_100;
        break;

    case NOLINE_PARAMS_TYPE_CONTRAST:
        NolinerData.osd0 = pData.Contrast_0;
        NolinerData.osd25 = pData.Contrast_25;
        NolinerData.osd50 = pData.Contrast_50;
        NolinerData.osd75 = pData.Contrast_75;
        NolinerData.osd100 = pData.Contrast_100;
        break;

    case NOLINE_PARAMS_TYPE_SATURATION:
        NolinerData.osd0 = pData.Saturation_0;
        NolinerData.osd25 = pData.Saturation_25;
        NolinerData.osd50 = pData.Saturation_50;
        NolinerData.osd75 = pData.Saturation_75;
        NolinerData.osd100 = pData.Saturation_100;
        break;

    case NOLINE_PARAMS_TYPE_HUE:
        NolinerData.osd0 = pData.Hue_0;
        NolinerData.osd25 = pData.Hue_25;
        NolinerData.osd50 = pData.Hue_50;
        NolinerData.osd75 = pData.Hue_75;
        NolinerData.osd100 = pData.Hue_100;
        break;

    case NOLINE_PARAMS_TYPE_SHARPNESS:
        NolinerData.osd0 = pData.Sharpness_0;
        NolinerData.osd25 = pData.Sharpness_25;
        NolinerData.osd50 = pData.Sharpness_50;
        NolinerData.osd75 = pData.Sharpness_75;
        NolinerData.osd100 = pData.Sharpness_100;
        break;
    default:
        break;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return NolinerData;
}

int CPQControl::FactorySetHdrMode(int mode)
{
    return SetHDRMode(mode);
}

int CPQControl::FactoryGetHdrMode(void)
{
    return GetHDRMode();
}

int CPQControl::FactorySetOverscanParam(source_input_param_t source_input_param, vpp_display_mode_t dmode, tvin_cutwin_t cutwin_t)
{
    int ret = -1;
    if (mbCpqCfg_separate_db_enable) {
        ret = mpOverScandb->PQ_SetOverscanParams(source_input_param, dmode, cutwin_t);
    }

    if (ret != 0) {
        SYS_LOGE("%s failed.\n", __FUNCTION__);
    } else {
        SYS_LOGD("%s success.\n", __FUNCTION__);
    }

    return ret;
}

tvin_cutwin_t CPQControl::FactoryGetOverscanParam(source_input_param_t source_input_param, vpp_display_mode_t dmode)
{
    int ret = -1;
    tvin_cutwin_t cutwin_t;
    memset(&cutwin_t, 0, sizeof(cutwin_t));

    if (source_input_param.trans_fmt < TVIN_TFMT_2D || source_input_param.trans_fmt > TVIN_TFMT_3D_LDGD) {
        return cutwin_t;
    }
    if (mbCpqCfg_separate_db_enable) {
        ret = mpOverScandb->PQ_GetOverscanParams(source_input_param, dmode, &cutwin_t);
    }

    if (ret != 0) {
        SYS_LOGE("%s failed.\n", __FUNCTION__);
    } else {
        SYS_LOGD("%s success.\n", __FUNCTION__);
    }

    return cutwin_t;
}

int CPQControl::FactorySetGamma(int gamma_r_value, int gamma_g_value, int gamma_b_value)
{
    int ret = 0;
    tcon_gamma_table_t gamma_r, gamma_g, gamma_b;

    memset(gamma_r.data, (unsigned short)gamma_r_value, GAMMA_NUMBER);
    memset(gamma_g.data, (unsigned short)gamma_g_value, GAMMA_NUMBER);
    memset(gamma_b.data, (unsigned short)gamma_b_value, GAMMA_NUMBER);

    ret |= Cpq_SetGammaTbl_R((unsigned short *) gamma_r.data);
    ret |= Cpq_SetGammaTbl_G((unsigned short *) gamma_g.data);
    ret |= Cpq_SetGammaTbl_B((unsigned short *) gamma_b.data);

    return ret;
}

int CPQControl::FactorySSMRestore(void)
{
    resetAllUserSettingParam();
    return 0;
}

int CPQControl::Cpq_SetXVYCCMode(vpp_xvycc_mode_t xvycc_mode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_xvycc_enable) {
        SYS_LOGD("XVYCC module disabled!\n");
        return 0;
    }

    int ret = -1;
    am_regs_t regs, regs_1;
    memset(&regs, 0, sizeof(am_regs_t));
    memset(&regs_1, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetXVYCCParams((vpp_xvycc_mode_t) xvycc_mode, source_input_param, &regs, &regs_1) == 0) {
        ret = Cpq_LoadRegs(regs);
        ret |= Cpq_LoadRegs(regs_1);
    } else {
        SYS_LOGE("PQ_GetXVYCCParams failed!\n");
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::SetColorDemoMode(vpp_color_demomode_t demomode)
{
    SYS_LOGD("%s: mode is %d\n", __FUNCTION__, demomode);
    int ret = -1;
    cm_regmap_t regmap;
    unsigned long *temp_regmap;
    int i = 0;
    vpp_display_mode_t displaymode = VPP_DISPLAY_MODE_MODE43;

    switch (demomode) {
    case VPP_COLOR_DEMO_MODE_YOFF:
        temp_regmap = DemoColorYOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_COFF:
        temp_regmap = DemoColorCOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_GOFF:
        temp_regmap = DemoColorGOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_MOFF:
        temp_regmap = DemoColorMOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_ROFF:
        temp_regmap = DemoColorROffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_BOFF:
        temp_regmap = DemoColorBOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_RGBOFF:
        temp_regmap = DemoColorRGBOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_YMCOFF:
        temp_regmap = DemoColorYMCOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_ALLOFF:
        temp_regmap = DemoColorALLOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_ALLON:
    default:
        if (displaymode == VPP_DISPLAY_MODE_MODE43) {
            temp_regmap = DemoColorSplit4_3RegMap;
        }/* else {
            temp_regmap = DemoColorSplitRegMap;
        }*/

        break;
    }

    for (i = 0; i < CM_REG_NUM; i++) {
        regmap.reg[i] = temp_regmap[i];
    }

    ret = VPPDeviceIOCtl(AMSTREAM_IOC_CM_REGMAP, regmap);
    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::SetColorBaseMode(vpp_color_basemode_t basemode, int isSave)
{
    SYS_LOGI("%s: mode is %d\n", __FUNCTION__, basemode);
    int ret = Cpq_SetColorBaseMode(basemode, mCurrentSourceInputInfo);

    if (ret < 0) {
        SYS_LOGE("Cpq_SetColorBaseMode Failed!!!");
    } else {
        if (isSave == 1) {
            ret = SaveColorBaseMode(basemode);
        } else {
            SYS_LOGD("%s: No need save!\n", __FUNCTION__);
        }
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

vpp_color_basemode_t CPQControl::GetColorBaseMode(void)
{
    vpp_color_basemode_t data = VPP_COLOR_BASE_MODE_OFF;
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return data;
    }

    data = (vpp_color_basemode_t)pData.colorbase;

    if (data < VPP_COLOR_BASE_MODE_OFF || data >= VPP_COLOR_BASE_MODE_MAX) {
        data = VPP_COLOR_BASE_MODE_OPTIMIZE;
    }

    SYS_LOGD("%s: mode is %d\n", __FUNCTION__, data);
    return data;
}

int CPQControl::SaveColorBaseMode(vpp_color_basemode_t basemode)
{
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    pData.colorbase = (int)basemode;

    if (!SetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s SetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetColorBaseMode(vpp_color_basemode_t basemode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_cm2_enable) {
        SYS_LOGD("CM module disabled!\n");
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetCM2Params((vpp_color_management2_t)basemode, source_input_param, &regs) < 0) {
        SYS_LOGE("PQ_GetCM2Params failed!\n");
        return -1;
    }

    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    return 0;
}

//CMS
int CPQControl::SetColorCustomize(int Color, int Type, int value, int isSave)
{
    SYS_LOGD("%s color: %d, type: %d, value: %d, isSave: %d\n", __FUNCTION__, Color, Type, value, isSave);

    if (Type >= Type_Max) {
        SYS_LOGE("%s type is out of range\n",__FUNCTION__);
        return -1;
    }

    if (Color >= COLOR_MAX) {
        SYS_LOGE("%s Color is out of range\n",__FUNCTION__);
        return -1;
    }

    if (isSave) {
        SaveColorCustomize(Color, Type, value);
    }

    if (GetColorCustomizeEnable() == _OFF) {
        return Cpq_SetColorCustomizeEnable(_OFF);
    } else {
        Cpq_SetColorCustomizeEnable(_ON);
    }

    if (Cpq_SetColorCustomize(Color, Type, value) < 0) {
        SYS_LOGE("%s Cpq_SetColorCustomize fail\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success\n",__FUNCTION__);
    return 0;
}

int CPQControl::GetColorCustomize(int Color, int Type)
{
    if (Type >= Type_Max) {
        SYS_LOGE("%s type is out of range\n",__FUNCTION__);
        return -1;
    }

    if (Color >= COLOR_MAX) {
        SYS_LOGE("%s Color is out of range\n",__FUNCTION__);
        return -1;
    }

    int value = 0;
    TABLE_CMS pData;
    if (!GetColorCustomizeData(&pData)) {
        SYS_LOGE("%s GetColorCustomizeData fail\n",__FUNCTION__);
        return -1;
    }

    if (Type == Type_Saturation)
        value = pData.CmsColor[Color].Saturation;
    else if (Type == Type_Hue)
        value = pData.CmsColor[Color].Hue;
    else if (Type == Type_Luma)
        value = pData.CmsColor[Color].Luma;

    SYS_LOGD("%s value = %d\n",__FUNCTION__, value);
    return value;
}

int CPQControl::SaveColorCustomize(int Color, int Type, int value)
{
    if (Type >= Type_Max) {
        SYS_LOGE("%s type is out of range\n",__FUNCTION__);
        return -1;
    }

    if (Color >= COLOR_MAX) {
        SYS_LOGE("%s Color is out of range\n",__FUNCTION__);
        return -1;
    }

    TABLE_CMS pData;
    if (!GetColorCustomizeData(&pData)) {
        SYS_LOGE("%s GetColorCustomizeData fail\n",__FUNCTION__);
        return -1;
    }

    if (Type == Type_Saturation)
        pData.CmsColor[Color].Saturation  = value;
    else if (Type == Type_Hue)
        pData.CmsColor[Color].Hue  = value;
    else if (Type == Type_Luma)
        pData.CmsColor[Color].Luma  = value;

    if (!SetColorCustomizeData(&pData)) {
        SYS_LOGE("%s SetColorCustomizeData fail\n",__FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetColorCustomize(int Color, int Type, int value)
{
    cms_color_md_t pData;
    memset(&pData, 0, sizeof(cms_color_md_t));
    pData.color_type = CM_9_COLOR;
    pData.cm_9_color_md = (CMS_COLOR)Color;
    pData.cm_14_color_md = COLOR_14_MAX;
    pData.color_val = GetDriverValueMap((CMS_TYPE)Type, value);
    SYS_LOGD("%s cm_9_color_md = %d, cm_14_color_md = %d, color_value = %d.\n", __FUNCTION__, pData.cm_9_color_md, pData.cm_14_color_md, pData.color_val);

    int ret = 0;
    switch (Type) {
        case Type_Saturation:
            ret = VPPDeviceIOCtl(AMVECM_IOC_S_CMS_SAT, &pData);
            break;
        case Type_Hue:
            ret = VPPDeviceIOCtl(AMVECM_IOC_S_CMS_HUE_HS, &pData);
            break;
        case Type_Luma:
            ret = VPPDeviceIOCtl(AMVECM_IOC_S_CMS_LUMA, &pData);
            break;
        default:
            break;
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::GetDriverValueMap(CMS_TYPE type, int value)
{
    switch (type) {
        case Type_Saturation:
            if (value < 0) {
                value = (value * (0 - CMS_SAT_MIN)) / 50;
            } else {
                value = (value * CMS_SAT_MAX) / 50;
            }
            if (value < CMS_SAT_MIN) value = CMS_SAT_MIN;
            if (value > CMS_SAT_MAX) value = CMS_SAT_MAX;
            break;
        case Type_Hue:
            value = (((value + 50) * (CMS_HUE_MAX - CMS_HUE_MIN)) / 100) - CMS_HUE_MAX;
            if (value < CMS_HUE_MIN) value = CMS_HUE_MIN;
            if (value > CMS_HUE_MAX) value = CMS_HUE_MAX;
            break;
        case Type_Luma:
            value = (((value + 50) * (CMS_LUMA_MAX - CMS_LUMA_MIN)) / 100) - CMS_LUMA_MAX;
            if (value < CMS_LUMA_MIN) value = CMS_LUMA_MIN;
            if (value > CMS_LUMA_MAX) value = CMS_LUMA_MAX;
            break;
        default:
            break;
    }

    return value;
}

int CPQControl::SetColorCustomizeEnable(int enable)
{
    SaveColorCustomizeEnable(enable);

    if (Cpq_SetColorCustomizeEnable(enable) < 0) {
        SYS_LOGE("%s Cpq_SetColorTuneEnable fail\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success\n",__FUNCTION__);
    return 0;
}

int CPQControl::GetColorCustomizeEnable(void)
{
    int enable = 0;
    TABLE_CMS pData;
    if (!GetColorCustomizeData(&pData)) {
        SYS_LOGE("%s GetColorCustomizeData fail\n",__FUNCTION__);
        return enable;
    }

    enable = pData.CmsEnable;

    if (enable < 0 || enable > 1) {
        enable = 0;
    }

    SYS_LOGD("%s enable: %d\n",__FUNCTION__, enable);
    return enable;
}

int CPQControl::SaveColorCustomizeEnable(int enable)
{
    TABLE_CMS pData;
    if (!GetColorCustomizeData(&pData)) {
        SYS_LOGE("%s GetColorCustomizeData fail\n",__FUNCTION__);
        return -1;
    }

    pData.CmsEnable= enable;

    if (!SetColorCustomizeData(&pData)) {
        SYS_LOGE("%s SetColorCustomizeData fail\n",__FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetColorCustomizeEnable(int enable)
{
    SYS_LOGD("%s ColorCustomize enable: %d!\n", __FUNCTION__, enable);

    TABLE_CMS param;
    if (!GetColorCustomizeData(&param)) {
        SYS_LOGE("%s GetColorCustomizeData fail\n",__FUNCTION__);
        return -1;
    }

    if (enable == param.CmsEnable) {
        SYS_LOGE("%s same status\n",__FUNCTION__);
        return 0;
    }

    if (enable == 0) {
        memset(&param, 0, sizeof(TABLE_CMS));
    }

    int ret = 0;
    for (int i = COLOR_RED; i < COLOR_MAX; i++) {
        ret |= Cpq_SetColorCustomize((CMS_COLOR)i, Type_Saturation, param.CmsColor[i].Saturation);
        ret |= Cpq_SetColorCustomize((CMS_COLOR)i, Type_Hue, param.CmsColor[i].Hue);
        ret |= Cpq_SetColorCustomize((CMS_COLOR)i, Type_Luma, param.CmsColor[i].Luma);
    }

    return ret;
}

int CPQControl::Cpq_SetRGBOGO(const struct tcon_rgb_ogo_s *rgbogo)
{
    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_RGB_OGO, rgbogo);
    if (ret < 0) {
        SYS_LOGE("%s failed(%s)!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::Cpq_GetRGBOGO(const struct tcon_rgb_ogo_s *rgbogo)
{
    int ret = VPPDeviceIOCtl(AMVECM_IOC_G_RGB_OGO, rgbogo);
    if (ret < 0) {
        SYS_LOGE("%s failed(%s)!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::Cpq_SetGammaOnOff(int onoff)
{
    int ret = -1;

    if (onoff == 1) {
        SYS_LOGD("%s: enable gamma!\n", __FUNCTION__);
        ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_TABLE_EN);
    } else {
        SYS_LOGD("%s: disable gamma!\n", __FUNCTION__);
        ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_TABLE_DIS);
    }

    if (ret < 0) {
        SYS_LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::SetAad(void)
{
    if (!mbCpqCfg_aad_enable) {
        SYS_LOGD("AAD module disabled\n");
        return 0;
    }

    aad_param_t newaad;
    if (mPQdb->PQ_GetAADParams(mCurrentSourceInputInfo, &newaad) < 0) {
        SYS_LOGE("mPQdb->PQ_GetAADParams failed\n");
        return -1;
    }

    db_aad_param_t db_newaad;
    db_newaad.aad_param_cabc_aad_en                  = newaad.aad_param_cabc_aad_en;
    db_newaad.aad_param_aad_en                       = newaad.aad_param_aad_en;
    db_newaad.aad_param_tf_en                        = newaad.aad_param_tf_en;
    db_newaad.aad_param_force_gain_en                = newaad.aad_param_force_gain_en;
    db_newaad.aad_param_sensor_mode                  = newaad.aad_param_sensor_mode;
    db_newaad.aad_param_mode                         = newaad.aad_param_mode;
    db_newaad.aad_param_dist_mode                    = newaad.aad_param_dist_mode;
    db_newaad.aad_param_tf_alpha                     = newaad.aad_param_tf_alpha;
    db_newaad.aad_param_sensor_input[0]              = newaad.aad_param_sensor_input[0];
    db_newaad.aad_param_sensor_input[1]              = newaad.aad_param_sensor_input[1];
    db_newaad.aad_param_sensor_input[2]              = newaad.aad_param_sensor_input[2];
    db_newaad.db_LUT_Y_gain.length                   = newaad.aad_param_LUT_Y_gain_len;
    db_newaad.db_LUT_Y_gain.cabc_aad_param_ptr_len   = (long long)&(newaad.aad_param_LUT_Y_gain);
    db_newaad.db_LUT_RG_gain.length                  = newaad.aad_param_LUT_RG_gain_len;
    db_newaad.db_LUT_RG_gain.cabc_aad_param_ptr_len  = (long long)&(newaad.aad_param_LUT_RG_gain);
    db_newaad.db_LUT_BG_gain.length                  = newaad.aad_param_LUT_BG_gain_len;
    db_newaad.db_LUT_BG_gain.cabc_aad_param_ptr_len  = (long long)&(newaad.aad_param_LUT_BG_gain);
    db_newaad.db_gain_lut.length                     = newaad.aad_param_gain_lut_len;
    db_newaad.db_gain_lut.cabc_aad_param_ptr_len     = (long long)&(newaad.aad_param_gain_lut);
    db_newaad.db_xy_lut.length                       = newaad.aad_param_xy_lut_len;
    db_newaad.db_xy_lut.cabc_aad_param_ptr_len       = (long long)&(newaad.aad_param_xy_lut);

    if (VPPDeviceIOCtl(AMVECM_IOC_S_AAD_PARAM, &db_newaad) < 0) {
        SYS_LOGE("%s failed error(%s)\n",__FUNCTION__, strerror(errno));
        return -1;
    }

    SYS_LOGI("%s success\n",__FUNCTION__);
    return 0;
}

int CPQControl::SetCabc(void)
{
    if (!mbCpqCfg_cabc_enable) {
        SYS_LOGD("CABC module disabled\n");
        return 0;
    }

    cabc_param_t newcabc;
    if (mPQdb->PQ_GetCABCParams(mCurrentSourceInputInfo, &newcabc) < 0) {
        SYS_LOGE("mPQdb->PQ_GetCABCParams failed\n");
        return -1;
    }

    db_cabc_param_t db_newcabc;
    db_newcabc.cabc_param_cabc_en                     = newcabc.cabc_param_cabc_en;
    db_newcabc.cabc_param_hist_mode                   = newcabc.cabc_param_hist_mode;
    db_newcabc.cabc_param_tf_en                       = newcabc.cabc_param_tf_en;
    db_newcabc.cabc_param_sc_flag                     = newcabc.cabc_param_sc_flag;
    db_newcabc.cabc_param_bl_map_mode                 = newcabc.cabc_param_bl_map_mode;
    db_newcabc.cabc_param_bl_map_en                   = newcabc.cabc_param_bl_map_en;
    db_newcabc.cabc_param_temp_proc                   = newcabc.cabc_param_temp_proc;
    db_newcabc.cabc_param_max95_ratio                 = newcabc.cabc_param_max95_ratio;
    db_newcabc.cabc_param_hist_blend_alpha            = newcabc.cabc_param_hist_blend_alpha;
    db_newcabc.cabc_param_init_bl_min                 = newcabc.cabc_param_init_bl_min;
    db_newcabc.cabc_param_init_bl_max                 = newcabc.cabc_param_init_bl_max;
    db_newcabc.cabc_param_tf_alpha                    = newcabc.cabc_param_tf_alpha;
    db_newcabc.cabc_param_sc_hist_diff_thd            = newcabc.cabc_param_sc_hist_diff_thd;
    db_newcabc.cabc_param_sc_apl_diff_thd             = newcabc.cabc_param_sc_apl_diff_thd;
    db_newcabc.cabc_param_patch_bl_th                 = newcabc.cabc_param_patch_bl_th;
    db_newcabc.cabc_param_patch_on_alpha              = newcabc.cabc_param_patch_on_alpha;
    db_newcabc.cabc_param_patch_bl_off_th             = newcabc.cabc_param_patch_bl_off_th;
    db_newcabc.cabc_param_patch_off_alpha             = newcabc.cabc_param_patch_off_alpha;
    db_newcabc.db_o_bl_cv.length                      = newcabc.cabc_param_o_bl_cv_len;
    db_newcabc.db_o_bl_cv.cabc_aad_param_ptr_len      = (long long)&(newcabc.cabc_param_o_bl_cv);
    db_newcabc.db_maxbin_bl_cv.length                 = newcabc.cabc_param_maxbin_bl_cv_len;
    db_newcabc.db_maxbin_bl_cv.cabc_aad_param_ptr_len = (long long)&(newcabc.cabc_param_maxbin_bl_cv);

    if (VPPDeviceIOCtl(AMVECM_IOC_S_CABC_PARAM, &db_newcabc) < 0) {
        SYS_LOGE("%s failed error(%s)\n",__FUNCTION__, strerror(errno));
        return -1;
    }

    SYS_LOGI("%s success\n",__FUNCTION__);
    return 0;
}

int CPQControl::SetDnlpMode(int level)
{
    int ret = -1;
    ret = Cpq_SetDnlpMode((Dynamic_contrast_status_t)level, mCurrentSourceInputInfo);

    if (ret == 0) {
        ret = SaveDnlpMode((Dynamic_contrast_status_t)level);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGI("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetDnlpMode()
{
    int level = DYNAMIC_CONTRAST_MID;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
        return level;
    }

    level = para.DynamicContrast;

    SYS_LOGI("%s, source_input = %d, mode is %d\n",__FUNCTION__, mCurrentSourceInputInfo.source_input, level);
    return level;
}

int CPQControl::SaveDnlpMode(Dynamic_contrast_status_t level)
{
     PICTURE_MODE_DATA para;
     PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
     if (!GetPictureModeData(&para, pq_mode)) {
         SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
         return -1;
     }

     para.DynamicContrast = (int)level;

     if (!SetPictureModeData(&para, pq_mode)) {
         SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
         return -1;
     }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetDnlpMode(Dynamic_contrast_status_t level, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_dnlp_enable) {
        SYS_LOGD("DNLP module disabled!\n");
        return 0;
    }

    ve_dnlp_curve_param_t newdnlp;
    if (mPQdb->PQ_GetDNLPParams(mCurrentSourceInputInfo, level, &newdnlp) < 0) {
        SYS_LOGE("mPQdb->PQ_GetDNLPParams failed!\n");
        return -1;
    }

    if (VPPDeviceIOCtl(AMVECM_IOC_VE_NEW_DNLP, &newdnlp) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetDNLPStatus(ve_dnlp_state_t status)
{
    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_DNLP_STATE, &status);
    if (ret < 0) {
        SYS_LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::FactorySetDNLPCurveParams(source_input_param_t source_input_param, int level, int final_gain)
{
    int ret = -1;
    int cur_final_gain = -1;
    char tmp_buf[128];

    cur_final_gain = mPQdb->PQ_GetDNLPGains(source_input_param, (Dynamic_contrast_status_t)level);
    if (cur_final_gain == final_gain) {
        SYS_LOGI("FactorySetDNLPCurveParams, same value, no need to update!");
        return ret;
    } else {
        SYS_LOGI("%s final_gain = %d \n", __FUNCTION__, final_gain);
        sprintf(tmp_buf, "%s %s %d", "w", "final_gain", final_gain);
        pqWriteSys(AMVECM_PQ_DNLP_DEBUG, tmp_buf);
        ret |= mPQdb->PQ_SetDNLPGains(source_input_param, (Dynamic_contrast_status_t)level, final_gain);
    }

    return ret;
}

int CPQControl::FactoryGetDNLPCurveParams(source_input_param_t source_input_param, int level)
{
    return mPQdb->PQ_GetDNLPGains(source_input_param, (Dynamic_contrast_status_t)level);
}

int CPQControl::FactorySetBlackExtRegParams(source_input_param_t source_input_param, int val)
{
    int rt = -1;
    unsigned int reg_val = 0,tmp = 0;
    unsigned int start_val = 0;
    char tmp_buf[128];

    SYS_LOGI("%s, input BE value: %d!\n", __FUNCTION__, val);

    //read data from pq.db in condition of RegAddr = 0x1D80
    reg_val = FactoryGetBEValFromDB(source_input_param, VPP_BLACKEXT_CTRL);
    SYS_LOGD("%s, read reg_val from DB: reg_val: 0x%x!\n", __FUNCTION__, reg_val);

    // blackext_start(31~24 bit) get value
    tmp = reg_val;
    for (int i = 31; i >= 24; i--) {
        start_val |= ((tmp & (1<<i)) >> 24);
    }
    SYS_LOGD("%s, blackext_start val: 0x%x\n",__FUNCTION__, start_val);

    //Re assign for target reg_addr. (23~16bit)  reset value
    for (int i = 16; i <= 23;i++) {
        reg_val = (reg_val & ~(1<<i));
    }
    reg_val = reg_val | (val << 16);
    SYS_LOGD("%s, reg_val: 0x%x val: 0x%x\n",__FUNCTION__, reg_val, val);

    rt = FactorySetBERegDBVal(source_input_param, VPP_BLACKEXT_CTRL, reg_val);

    sprintf(tmp_buf, "%s", "blk_ext_en");
    pqWriteSys(AMVECM_PQ_USER_SET, tmp_buf);

    sprintf(tmp_buf, "%s %d", "blk_start",start_val);
    pqWriteSys(AMVECM_PQ_USER_SET, tmp_buf);

    sprintf(tmp_buf, "%s %d", "blk_slope",val);
    pqWriteSys(AMVECM_PQ_USER_SET, tmp_buf);

    return rt;
}

int CPQControl::FactoryGetBlackExtRegParams(source_input_param_t source_input_param)
{
    int rt = -1;
    unsigned int reg_val = 0;
    unsigned int start_val = 0, slope_val = 0;

    //read data from pq.db in condition of RegAddr = iaddr
    reg_val = FactoryGetBEValFromDB(source_input_param, VPP_BLACKEXT_CTRL);
    SYS_LOGD("%s, read reg_val from DB: reg_val: 0x%x!\n", __FUNCTION__,reg_val);

    //analysis val read from db, Bit 31~24 blackext_start, Bit 23~16 blackext_slope1;
    // blackext_start(31~24 bit) get value
    for (int i = 31; i >= 24; i--) {
        start_val |= ((reg_val & (1<<i)) >> 24);
    }
    SYS_LOGD("%s, start_val: 0x%x\n",__FUNCTION__, start_val);
    // blackext_slope1(23~16 bit) get value
    for (int i = 23; i >= 16; i--) {
        slope_val |= ((reg_val & (1<<i)) >> 16);
    }
    SYS_LOGD("%s, slope_val: 0x%x\n",__FUNCTION__, slope_val);
    rt = slope_val;

    return rt;
}

int CPQControl::FactoryGetBEValFromDB(source_input_param_t source_input_param, int addr)
{
    unsigned int ret = 0;
    am_regs_t regs;

    SYS_LOGD("%s, input BE addr: 0x%x!\n", __FUNCTION__,addr);
    mPQdb->PQ_GetBEParams(source_input_param, addr, &regs);

    SYS_LOGD("%s - get addr - 0x%x & value - 0x%x", __FUNCTION__,
          regs.am_reg[0].addr,regs.am_reg[0].val);

    ret = regs.am_reg[0].val;

    return ret;
}

int CPQControl::FactorySetBERegDBVal(source_input_param_t source_input_param, int addr, unsigned int reg_val)
{
    int ret = -1;
    am_regs_t regs;

    mPQdb->PQ_GetBEParams(source_input_param, addr, &regs);
    regs.am_reg[0].val = reg_val;
    ret = mPQdb->PQ_SetBEParams(source_input_param, addr, regs.am_reg[0].val);
    SYS_LOGD ("%s - get addr - 0x%x & value - 0x%x", __FUNCTION__,
          regs.am_reg[0].addr,regs.am_reg[0].val);

    return ret;
}

int CPQControl::FactorySetRGBCMYFcolorParams(source_input_param_t source_input_param, int color_type,int color_param,int val)
{
/*
    int data_Rank = -1;
    char tmp_buf[128];

    if (color_param == Type_Saturation) {
        if (val < -100 || val > 127)
            return -1;
    } else if (color_param == Type_Hue) {
        if (val < -127 || val > 127)
            return -1;
    } else if (color_param == Type_Luma) {
        if (val < -15 || val > 15)
            return -1;
    }

    switch (color_type) {
        case COLOR_RED:
            if (color_param == Type_Saturation) {
                data_Rank = CMS_sat_red;
            } else if (color_param == Type_Hue) {
                data_Rank = CMS_hue_red;
            } else if (color_param == Type_Luma) {
                data_Rank = CMS_luma_red;
            }
            break;
        case COLOR_GREEN:
            if (color_param == Type_Saturation) {
                data_Rank = CMS_sat_green;
            } else if (color_param == Type_Hue) {
                data_Rank = CMS_hue_green;
            } else if (color_param == Type_Luma) {
                data_Rank = CMS_luma_green;
            }
            break;
        case COLOR_BLUE:
            if (color_param == Type_Saturation) {
                data_Rank = CMS_sat_blue;
            } else if (color_param == Type_Hue) {
                data_Rank = CMS_hue_blue;
            } else if (color_param == Type_Luma) {
                data_Rank = CMS_luma_blue;
            }
            break;
        case COLOR_GRAY:
            if (color_param == COLOR_SATURATION) {
                data_Rank = CMS_sat_cyan;
            } else if (color_param == COLOR_HUE) {
                data_Rank = CMS_hue_cyan;
            } else if (color_param == COLOR_LUMA) {
                data_Rank = CMS_luma_cyan;
            }
            break;
        case COLOR_MAGENTA:
            if (color_param == COLOR_SATURATION) {
                data_Rank = CMS_sat_purple;
            } else if (color_param == COLOR_HUE) {
                data_Rank = CMS_hue_purple;
            } else if (color_param == COLOR_LUMA) {
                data_Rank = CMS_luma_purple;
            }
            break;
        case COLOR_YELLOW:
            if (color_param == COLOR_SATURATION) {
                data_Rank = CMS_sat_yellow;
            } else if (color_param == COLOR_HUE) {
                data_Rank = CMS_hue_yellow;
            } else if (color_param == COLOR_LUMA) {
                data_Rank = CMS_luma_yellow;
            }
            break;
        case COLOR_FLESHTONE:
            if (color_param == COLOR_SATURATION) {
                data_Rank = CMS_sat_skin;
            } else if (color_param == COLOR_HUE) {
                data_Rank = CMS_hue_skin;
            } else if (color_param == COLOR_LUMA) {
                data_Rank = CMS_luma_skin;
            }
            break;
        default:
            break;
    }

    if (color_param == COLOR_SATURATION) {
        sprintf(tmp_buf, "%s %d %d %d", "cm2_sat", color_type, val, 0);
        pqWriteSys(AMVECM_PQ_CM2_SAT, tmp_buf);
    } else if (color_param == COLOR_HUE) {
        sprintf(tmp_buf, "%s %d %d %d", "cm2_hue", color_type, val, 0);
        pqWriteSys(AMVECM_PQ_CM2_HUE_BY_HS, tmp_buf);
    } else if (color_param == COLOR_LUMA) {
        sprintf(tmp_buf, "%s %d %d %d", "cm2_luma", color_type, val, 0);
        pqWriteSys(AMVECM_PQ_CM2_LUMA, tmp_buf);
    }

    return mPQdb->PQ_SetRGBCMYFcolor(source_input_param, data_Rank, val);
    */
    return 0;
}

int CPQControl::FactoryGetRGBCMYFcolorParams(source_input_param_t source_input_param, int color_type,int color_param)
{
/*
    int data_Rank = -1;

    switch (color_type) {
        case COLOR_RED:
            if (color_param == COLOR_SATURATION) {
                data_Rank = CMS_sat_red;
            } else if (color_param == COLOR_HUE) {
                data_Rank = CMS_hue_red;
            } else if (color_param == COLOR_LUMA) {
                data_Rank = CMS_luma_red;
            }
            break;
        case COLOR_GREEN:
            if (color_param == COLOR_SATURATION) {
                data_Rank = CMS_sat_green;
            } else if (color_param == COLOR_HUE) {
                data_Rank = CMS_hue_green;
            } else if (color_param == COLOR_LUMA) {
                data_Rank = CMS_luma_green;
            }
            break;
        case COLOR_BLUE:
            if (color_param == COLOR_SATURATION) {
                data_Rank = CMS_sat_blue;
            } else if (color_param == COLOR_HUE) {
                data_Rank = CMS_hue_blue;
            } else if (color_param == COLOR_LUMA) {
                data_Rank = CMS_luma_blue;
            }
            break;
        case COLOR_GRAY:
            if (color_param == COLOR_SATURATION) {
                data_Rank = CMS_sat_cyan;
            } else if (color_param == COLOR_HUE) {
                data_Rank = CMS_hue_cyan;
            } else if (color_param == COLOR_LUMA) {
                data_Rank = CMS_luma_cyan;
            }
            break;
        case COLOR_MAGENTA:
            if (color_param == COLOR_SATURATION) {
                data_Rank = CMS_sat_purple;
            } else if (color_param == COLOR_HUE) {
                data_Rank = CMS_hue_purple;
            } else if (color_param == COLOR_LUMA) {
                data_Rank = CMS_luma_purple;
            }
            break;
        case COLOR_YELLOW:
            if (color_param == COLOR_SATURATION) {
                data_Rank = CMS_sat_yellow;
            } else if (color_param == COLOR_HUE) {
                data_Rank = CMS_hue_yellow;
            } else if (color_param == COLOR_LUMA) {
                data_Rank = CMS_luma_yellow;
            }
            break;
        case COLOR_FLESHTONE:
            if (color_param == COLOR_SATURATION) {
                data_Rank = CMS_sat_skin;
            } else if (color_param == COLOR_HUE) {
                data_Rank = CMS_hue_skin;
            } else if (color_param == COLOR_LUMA) {
                data_Rank = CMS_luma_skin;
            }
            break;
        default:
            break;
    }

    return mPQdb->PQ_GetRGBCMYFcolor(source_input_param, data_Rank);
    */
    return 0;
}

int CPQControl::FactorySetNoiseReductionParams(source_input_param_t source_input_param, vpp_noise_reduction_mode_t nr_mode, int addr, int val)
{
    return mPQdb->PQ_SetNoiseReductionParams(nr_mode, source_input_param, addr, val);
}

int CPQControl::FactoryGetNoiseReductionParams(source_input_param_t source_input_param, vpp_noise_reduction_mode_t nr_mode, int addr)
{
    return mPQdb->PQ_GetNoiseReductionParams(nr_mode, source_input_param, addr);
}

int CPQControl::SetCTIParamsCheckVal(int param_type, int val)
{
    switch (param_type) {
        case CVD_YC_DELAY: {
            if (val < 0 || val > 15) {
                return -1;
            }
            break;
        }
        case DECODE_CTI: {
            if (val < 0 || val > 3) {
                return -1;
            }
            break;
        }
        case SR0_CTI_GAIN0:
        case SR0_CTI_GAIN1:
        case SR0_CTI_GAIN2:
        case SR0_CTI_GAIN3:
        case SR1_CTI_GAIN0:
        case SR1_CTI_GAIN1:
        case SR1_CTI_GAIN2:
        case SR1_CTI_GAIN3:{
            if (val < 0 || val > 255) {
                return -1;
            }
            break;
        }
    }

    return 0;
}

int CPQControl::MatchCTIRegMask(int param_type)
{
    switch (param_type) {
        case CVD_YC_DELAY:
            return YC_DELAY_REG_MASK;
        case DECODE_CTI:
            return DECODE_CTI_REG_MASK;
        case SR0_CTI_GAIN0:
            return SR0_GAIN0_REG_MASK;
        case SR0_CTI_GAIN1:
            return SR0_GAIN1_REG_MASK;
        case SR0_CTI_GAIN2:
            return SR0_GAIN2_REG_MASK;
        case SR0_CTI_GAIN3:
            return SR0_GAIN3_REG_MASK;
        case SR1_CTI_GAIN0:
            return SR1_GAIN0_REG_MASK;
        case SR1_CTI_GAIN1:
            return SR1_GAIN1_REG_MASK;
        case SR1_CTI_GAIN2:
            return SR1_GAIN2_REG_MASK;
        case SR1_CTI_GAIN3:
            return SR1_GAIN3_REG_MASK;
    }
    return -1;
}

int CPQControl::MatchCTIRegAddr(int param_type)
{
    if (param_type == CVD_YC_DELAY) {
        return VPP_CTI_YC_DELAY;
    } else if (param_type == DECODE_CTI) {
        return VPP_DECODE_CTI;
    } else if (param_type >= SR0_CTI_GAIN0 && param_type <= SR0_CTI_GAIN3) {
        return VPP_CTI_SR0_GAIN;
    } else if (param_type >= SR1_CTI_GAIN0 && param_type <= SR1_CTI_GAIN3) {
        return VPP_CTI_SR1_GAIN;
    } else {
        SYS_LOGE("%s, error: param_type[%d] is not supported", __FUNCTION__, param_type);
        return -1;
    }
    return -1;
}

int CPQControl::FactorySetCTIParams(source_input_param_t source_input_param, int param_type, int val)
{
    int tmp_val;
    char tmp_buf[128];
    int addr, reg_mask;

    addr = MatchCTIRegAddr(param_type);
    reg_mask = MatchCTIRegMask(param_type);

    if (SetCTIParamsCheckVal(param_type, val) < 0) {
        SYS_LOGE("%s, error: val[%d] is out of range", __FUNCTION__, val);
        return -1;
    }

    tmp_val = mPQdb->PQ_GetSharpnessCTIParams(source_input_param, addr, param_type, reg_mask);
    SYS_LOGI("%s, get value is %d, try to set value: %d\n", __FUNCTION__, tmp_val, val);

    switch (param_type) {
        case CVD_YC_DELAY: {
            tmp_val &= ~0xf;
            tmp_val |= val;
            sprintf(tmp_buf, "%s %x %x", "wv", tmp_val, VPP_CTI_YC_DELAY);
            pqWriteSys(TVAFE_TVAFE0_REG, tmp_buf);
            break;
        }
        case DECODE_CTI: {
            tmp_val &= ~(0x3 << 6);
            tmp_val |= (val << 6);
            sprintf(tmp_buf, "%s %x %x", "wv", tmp_val, VPP_DECODE_CTI);
            pqWriteSys(TVAFE_TVAFE0_REG, tmp_buf);
            break;
        }
        case SR0_CTI_GAIN0: {
            tmp_val = val << 24;
            sprintf(tmp_buf, "%s %x %x %x %x", "bw", VPP_CTI_SR0_GAIN, val, 24, 8);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case SR0_CTI_GAIN1: {
            tmp_val = val << 16;
            sprintf(tmp_buf, "%s %x %x %x %x", "bw", VPP_CTI_SR0_GAIN, val, 16, 8);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case SR0_CTI_GAIN2: {
            tmp_val = val << 8;
            sprintf(tmp_buf, "%s %x %x %x %x", "bw", VPP_CTI_SR0_GAIN, val, 8, 8);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case SR0_CTI_GAIN3: {
            tmp_val = val;
            sprintf(tmp_buf, "%s %x %x %x %x", "bw", VPP_CTI_SR0_GAIN, val, 0, 8);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case SR1_CTI_GAIN0: {
            tmp_val = val << 24;
            sprintf(tmp_buf, "%s %x %x %x %x", "bw", VPP_CTI_SR1_GAIN, val, 24, 8);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case SR1_CTI_GAIN1: {
            tmp_val = val << 16;
            sprintf(tmp_buf, "%s %x %x %x %x", "bw", VPP_CTI_SR1_GAIN, val, 16, 8);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case SR1_CTI_GAIN2: {
            tmp_val = val << 8;
            sprintf(tmp_buf, "%s %x %x %x %x", "bw", VPP_CTI_SR1_GAIN, val, 8, 8);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case SR1_CTI_GAIN3: {
            tmp_val = val;
            sprintf(tmp_buf, "%s %x %x %x %x", "bw", VPP_CTI_SR1_GAIN, val, 0, 8);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        default:
            break;
    }

    mPQdb->PQ_SetSharpnessCTIParams(source_input_param, addr, tmp_val, param_type, reg_mask);

    return 0;
}

int CPQControl::FactoryGetCTIParams(source_input_param_t source_input_param, int param_type)
{
    unsigned int rval, reg_val;
    int addr, reg_mask;

    addr = MatchCTIRegAddr(param_type);
    reg_mask = MatchCTIRegMask(param_type);

    reg_val = mPQdb->PQ_GetSharpnessCTIParams(source_input_param, addr, param_type, reg_mask);

    switch (param_type) {
        case CVD_YC_DELAY: {
            SYS_LOGD("%s, type [%d], get val: %u\n", __FUNCTION__, param_type, reg_val);
            SYS_LOGD("%s, type [%d], get val: %d\n", __FUNCTION__, param_type, reg_val);
            rval = reg_val & 0xf;
            SYS_LOGD("%s, type [%d], get val: %d\n", __FUNCTION__, param_type, rval);
            break;
        }
        case DECODE_CTI: {
            rval = (reg_val >> 6) & 0x3;
            break;
        }
        case SR0_CTI_GAIN0: {
            rval = (reg_val >> 24) & 0xff;
            break;
        }
        case SR0_CTI_GAIN1: {
            rval = (reg_val >> 16) & 0xff;
            break;
        }
        case SR0_CTI_GAIN2: {
            rval = (reg_val >> 8) & 0xff;
            break;
        }
        case SR0_CTI_GAIN3: {
            rval = reg_val & 0xff;
            break;
        }
        case SR1_CTI_GAIN0: {
            rval = (reg_val >> 24) & 0xff;
            break;
        }
        case SR1_CTI_GAIN1: {
            rval = (reg_val >> 16) & 0xff;
            break;
        }
        case SR1_CTI_GAIN2: {
            rval = (reg_val >> 8) & 0xff;
            break;
        }
        case SR1_CTI_GAIN3: {
            rval = reg_val & 0xff;
            break;
        }
        default: {
            rval = 0;
            break;
        }
    }

    SYS_LOGD("%s, type [%d], get reg_val: %u, ret val: %u\n", __FUNCTION__, param_type, reg_val, rval);
    return rval;
}

int CPQControl::SetDecodeLumaParamsCheckVal(int param_type, int val)
{
    switch (param_type) {
        case VIDEO_DECODE_BRIGHTNESS: {
            if (val < 0 || val > 511) {
                return -1;
            }
            break;
        }
        case VIDEO_DECODE_CONTRAST: {
            if (val < 0 || val > 1023) {
                return -1;
            }
            break;
        }
        case VIDEO_DECODE_SATURATION: {
            if (val < 0 || val > 255) {
                return -1;
            }
            break;
        }
        default: {
            break;
        }
    }
    return 0;
}

int CPQControl::FactorySetDecodeLumaParams(source_input_param_t source_input_param, int param_type, int val)
{
    unsigned int tmp_val = 0;
    char tmp_buf[128] = {0};
    int addr = 0, reg_mask = 0;
    int reg_set_val = 0;

    if (SetDecodeLumaParamsCheckVal(param_type, val) < 0) {
        SYS_LOGE("%s, error: val[%d] is out of range", __FUNCTION__, val);
        return -1;
    }

    if (param_type == VIDEO_DECODE_BRIGHTNESS) {
        addr = DECODE_BRI_ADDR; // 0~8 bit
        reg_mask = DECODE_BRI_REG_MASK;
    } else if (param_type == VIDEO_DECODE_CONTRAST) {
        addr = DECODE_CON_ADDR; //16~25 bit
        reg_mask = DECODE_CON_REG_MASK;
    } else if (param_type == VIDEO_DECODE_SATURATION) {
        addr = DECODE_SAT_ADDR; // 0~8 bit
        reg_mask = DECODE_SAT_REG_MASK;
    } else {
        SYS_LOGE("%s, error: param_type[%d] is not supported", __FUNCTION__, param_type);
        return -1;
    }

    tmp_val = mPQdb->PQ_GetCVD2Param(source_input_param, addr, param_type, reg_mask);
    SYS_LOGI("%s, get value is %d, try to set value: %d\n", __FUNCTION__, tmp_val, val);

    switch (param_type) {
        case VIDEO_DECODE_BRIGHTNESS: {
            tmp_val &=  ~0x1ff;
            tmp_val |= val;

            reg_set_val = tmp_val;
            reg_set_val |= (0x8 << 12);
            reg_set_val |= (0x8 << 28);
            SYS_LOGD("%s: setting val is %d", __FUNCTION__, reg_set_val);
            sprintf(tmp_buf, "%s %x %x", "wv", reg_set_val, DECODE_BRI_ADDR);
            pqWriteSys(TVAFE_TVAFE0_REG, tmp_buf);
            break;
        }
        case VIDEO_DECODE_CONTRAST: {
            tmp_val &= ~0x3ff0000;
            tmp_val |= (val << 16);

            reg_set_val = tmp_val;
            reg_set_val |= (0x8 << 12);
            reg_set_val |= (0x8 << 28);
            sprintf(tmp_buf, "%s %x %x", "wv", reg_set_val, DECODE_CON_ADDR);
            pqWriteSys(TVAFE_TVAFE0_REG, tmp_buf);
            break;
        }
        case VIDEO_DECODE_SATURATION: {
            tmp_val &= ~0xff;
            tmp_val |= val;
            sprintf(tmp_buf, "%s %x %x", "wv", tmp_val, DECODE_SAT_ADDR);
            pqWriteSys(TVAFE_TVAFE0_REG, tmp_buf);
            break;
        }
    }

    mPQdb->PQ_SetCVD2Param(source_input_param, addr, tmp_val, param_type, reg_mask);

    return 0;
}

int CPQControl::FactoryGetDecodeLumaParams(source_input_param_t source_input_param, int param_type)
{
    unsigned int rval, reg_val;
    int addr, reg_mask;

    if (param_type == VIDEO_DECODE_BRIGHTNESS) {
        addr = 0x157;
        reg_mask = 0x1ff;
    }else if (param_type == VIDEO_DECODE_CONTRAST) {
        addr = 0x157;
        reg_mask = 0x3ff0000;
    }else if (param_type == VIDEO_DECODE_SATURATION) {
        addr = DECODE_SAT_ADDR;
        reg_mask = 0xff;
    } else {
        SYS_LOGE("%s, error: param_type[%d] is not supported", __FUNCTION__, param_type);
        return -1;
    }

    reg_val = mPQdb->PQ_GetCVD2Param(source_input_param, addr, param_type, reg_mask);

    switch (param_type) {
        case VIDEO_DECODE_BRIGHTNESS: {
            rval = reg_val & 0x1ff;
            break;
        }
        case VIDEO_DECODE_CONTRAST: {
            rval = (reg_val >> 16) & 0x3ff;
            break;
        }
        case VIDEO_DECODE_SATURATION: {
            rval = reg_val & 0xff;
            break;
        }
        /*default: {
            rval = 0;
            break;
        }*/
    }

    SYS_LOGI("%s, type [%d], get reg_val: %u, ret val: %u\n", __FUNCTION__, param_type, reg_val, rval);
    return rval;
}

int CPQControl::SetSharpnessParamsCheckVal(int param_type, int val)
{
    switch (param_type) {
        case H_GAIN_HIGH:
        case H_GAIN_LOW:
        case V_GAIN_HIGH:
        case V_GAIN_LOW:
        case D_GAIN_HIGH:
        case D_GAIN_LOW:
        case PKGAIN_VSLUMALUT7:
        case PKGAIN_VSLUMALUT6:
        case PKGAIN_VSLUMALUT5:
        case PKGAIN_VSLUMALUT4:
        case PKGAIN_VSLUMALUT3:
        case PKGAIN_VSLUMALUT2:
        case PKGAIN_VSLUMALUT1:
        case PKGAIN_VSLUMALUT0: {
            if (val < 0 || val > 15) {
                return -1;
            }
            break;
        }
        case HP_DIAG_CORE:
        case BP_DIAG_CORE: {
            if (val < 0 || val > 63) {
                return -1;
            }
            break;
        }
        default: {
            break;
        }
    }

    return 0;
}

int CPQControl::MatchSharpnessRegAddr(int param_type, int isHd)
{
    if (isHd) {
        if (param_type >= H_GAIN_HIGH && param_type <= D_GAIN_LOW) {
            return SHARPNESS_HD_GAIN;
        } else if (param_type == HP_DIAG_CORE) {
            return SHARPNESS_HD_HP_DIAG_CORE;
        } else if (param_type == BP_DIAG_CORE) {
            return SHARPNESS_HD_BP_DIAG_CORE;
        } else if (param_type >= PKGAIN_VSLUMALUT7 && param_type <=PKGAIN_VSLUMALUT0) {
            return SHARPNESS_HD_PKGAIN_VSLUMA;
        } else {
            SYS_LOGE("%s, error: param_type[%d] is not supported", __FUNCTION__, param_type);
            return -1;
        }
    }

    if (param_type >= H_GAIN_HIGH && param_type <= D_GAIN_LOW) {
        return SHARPNESS_SD_GAIN;
    } else if (param_type == HP_DIAG_CORE) {
        return SHARPNESS_SD_HP_DIAG_CORE;
    } else if (param_type == BP_DIAG_CORE) {
        return SHARPNESS_SD_BP_DIAG_CORE;
    } else if (param_type >= PKGAIN_VSLUMALUT7 && param_type <=PKGAIN_VSLUMALUT0) {
        return SHARPNESS_SD_PKGAIN_VSLUMA;
    } else {
        SYS_LOGE("%s, error: param_type[%d] is not supported", __FUNCTION__, param_type);
        return -1;
    }
    return -1;
}

unsigned int CPQControl::GetSharpnessRegVal(int addr)
{
    char tmp_buf[128] = {0};
    char rval[32] = {0};

    sprintf(tmp_buf, "%s %x", "r", addr);
    pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
    pqReadSys(AMVECM_PQ_REG_RW, rval, sizeof(rval));
    return atoi(rval);
}

int CPQControl::FactorySetSharpnessParams(source_input_param_t source_input_param, Sharpness_timing_e source_timing, int param_type, int val)
{
    int tmp_val;
    char tmp_buf[128];
    int addr;

    if (SetSharpnessParamsCheckVal(param_type, val) < 0) {
        SYS_LOGE("%s, error: val[%d] is out of range", __FUNCTION__, val);
        return -1;
    }

    addr = MatchSharpnessRegAddr(param_type, source_timing);
    tmp_val = mPQdb->PQ_GetSharpnessAdvancedParams(source_input_param, addr, source_timing);
    if (tmp_val == 0) {
        tmp_val = GetSharpnessRegVal(addr);
    }

    SYS_LOGD("%s, get value is %d, try to set value: %d\n", __FUNCTION__, tmp_val, val);

    switch (param_type) {
        case H_GAIN_HIGH: {
            SYS_LOGD("%s, type [%d], get val: %u\n", __FUNCTION__, param_type, tmp_val);
            tmp_val = tmp_val & (~(0xf << 28));
            SYS_LOGD("%s, type [%d], get val: %u\n", __FUNCTION__, param_type, tmp_val);
            tmp_val |= (val << 28);
            SYS_LOGD("%s, setting value : %u", __FUNCTION__, tmp_val);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_GAIN, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case H_GAIN_LOW: {
            SYS_LOGD("%s, type [%d], get val: %d\n", __FUNCTION__, param_type, tmp_val);
            tmp_val &= (~(0xf << 12));
            tmp_val |= (val << 12);
            SYS_LOGD("%s, setting value : %d", __FUNCTION__, tmp_val);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_GAIN, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case V_GAIN_HIGH: {
            tmp_val &= (~(0xf << 24));
            tmp_val |= (val << 24);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_GAIN, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case V_GAIN_LOW: {
            tmp_val &= ~(0xf << 8);
            tmp_val |= (val << 8);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_GAIN, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case D_GAIN_HIGH: {
            tmp_val &= ~(0xf << 20);
            tmp_val |= (val << 20);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_GAIN, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case D_GAIN_LOW: {
            tmp_val &= ~(0xf << 4);
            tmp_val |= (val << 4);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_GAIN, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case HP_DIAG_CORE: {
            tmp_val &= ~0x3f;
            tmp_val |= val;
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_HP_DIAG_CORE, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case BP_DIAG_CORE: {
            tmp_val &= ~0x3f;
            tmp_val |= val;
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_BP_DIAG_CORE, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT7: {
            tmp_val &= ~(0xf << 28);
            tmp_val |= (val << 28);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT6: {
            tmp_val &= ~(0xf << 24);
            tmp_val |= (val << 24);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT5: {
            tmp_val &= ~(0xf << 20);
            tmp_val |= (val << 20);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT4: {
            tmp_val &= ~(0xf << 16);
            tmp_val |= (val << 16);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT3: {
            tmp_val &= ~(0xf << 12);
            tmp_val |= (val << 12);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT2: {
            tmp_val &= ~(0xf << 8);
            tmp_val |= (val << 8);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT1: {
            tmp_val &= ~(0xf << 4);
            tmp_val |= (val << 4);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT0: {
            tmp_val &= ~0xf;
            tmp_val |= val;
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(AMVECM_PQ_REG_RW, tmp_buf);
            break;
        }
        default:
            break;
    }

    mPQdb->PQ_SetSharpnessAdvancedParams(source_input_param, addr, tmp_val, source_timing);

    return 0;
}

int CPQControl::FactoryGetSharpnessParams(source_input_param_t source_input_param, Sharpness_timing_e source_timing, int param_type)
{
    unsigned int rval, reg_val;
    int addr;

    addr = MatchSharpnessRegAddr(param_type, source_timing);
    reg_val = mPQdb->PQ_GetSharpnessAdvancedParams(source_input_param, addr, source_timing);
    if (reg_val == 0)
    {
        reg_val = GetSharpnessRegVal(addr);
    }

    switch (param_type) {
        case H_GAIN_HIGH: {
            SYS_LOGI("%s, type [%d], get val: %u\n", __FUNCTION__, param_type, reg_val);
            rval = (reg_val >> 28) & 0xf;
            SYS_LOGI("%s, type [%d], get val: %u\n", __FUNCTION__, param_type, rval);
            break;
        }
        case H_GAIN_LOW: {
            rval = (reg_val >> 12) & 0xf;
            break;
        }
        case V_GAIN_HIGH: {
            rval = (reg_val >> 24) & 0xf;
            break;
        }
        case V_GAIN_LOW: {
            rval = (reg_val >> 8) & 0xf;
            break;
        }
        case D_GAIN_HIGH: {
            rval = (reg_val >> 20) & 0xf;
            break;
        }
        case D_GAIN_LOW: {
            rval = (reg_val >> 4) & 0xf;
            break;
        }
        case HP_DIAG_CORE: {
            rval = reg_val & 0x3f;
            break;
        }
        case BP_DIAG_CORE: {
            rval = reg_val & 0x3f;
            break;
        }
        case PKGAIN_VSLUMALUT7: {
            rval = (reg_val >> 28) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT6: {
            rval = (reg_val >> 24) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT5: {
            rval = (reg_val >> 20) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT4: {
            rval = (reg_val >> 16) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT3: {
            rval = (reg_val >> 12) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT2: {
            rval = (reg_val >> 8) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT1: {
            rval = (reg_val >> 4) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT0: {
            rval = reg_val & 0xf;
            break;
        }
        default: {
            rval = 0;
            break;
        }
    }

    SYS_LOGI("%s, type [%d], get reg_val: %u, ret val: %u\n", __FUNCTION__, param_type, reg_val, rval);
    return rval;
}

int CPQControl::SetEyeProtectionMode(tv_source_input_t source_input __unused, int enable, int is_save __unused)
{
    SYS_LOGI("%s: mode:%d!\n", __FUNCTION__, enable);
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s: GetPictureStructDataGlobal fail!\n", __FUNCTION__);
        return -1;
    }

    pData.EyeProtection = enable;

    if (!SetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s: SetPictureStructDataGlobal fail!\n", __FUNCTION__);
        return -1;
    }

    return Cpq_SetColorTemperature(GetColorTemperature());
}

int CPQControl::GetEyeProtectionMode(tv_source_input_t source_input __unused)
{
    int mode = -1;
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s: GetPictureStructDataGlobal fail!\n", __FUNCTION__);
        return mode;
    }

    return pData.EyeProtection;
}

int CPQControl::SetFlagByCfg(void)
{
    pq_ctrl_t pqControlVal;
    memset(&pqControlVal, 0x0, sizeof(pq_ctrl_t));
    const char *config_value;

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_BIG_SMALL_DB_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_separate_db_enable = true;
    } else {
        mbCpqCfg_separate_db_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DI_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_di_enable = true;
    } else {
        mbCpqCfg_di_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_MCDI_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_mcdi_enable = true;
        pqWriteSys(DI_PARAMETERS_MCEN_MODE, "1");
    } else {
        mbCpqCfg_mcdi_enable = false;
        pqWriteSys(DI_PARAMETERS_MCEN_MODE, "0");
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DEBLOCK_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_deblock_enable = true;
        pqWriteSys(DI_PARAMETERS_DNR_EN, "13");//bit2~bit3
    } else {
        mbCpqCfg_deblock_enable = false;
        pqWriteSys(DI_PARAMETERS_DNR_EN, "1");
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DEMOSQUITO_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_demoSquito_enable = true;
        pqWriteSys(DI_PARAMETERS_DNR_DM_EN, "1");//bit0
    } else {
        mbCpqCfg_demoSquito_enable = false;
        pqWriteSys(DI_PARAMETERS_DNR_DM_EN, "0");
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_NOISEREDUCTION_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_nr_enable = true;
        pqWriteSys(DI_PARAMETERS_NR2_EN, "1");
    } else {
        mbCpqCfg_nr_enable = false;
        pqWriteSys(DI_PARAMETERS_NR2_EN, "0");
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_SHARPNESS0_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_sharpness0_enable = true;
        pqControlVal.sharpness0_en = 1;
    } else {
        mbCpqCfg_sharpness0_enable = false;
        pqControlVal.sharpness0_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_SHARPNESS1_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_sharpness1_enable = true;
        pqControlVal.sharpness1_en = 1;
    } else {
        mbCpqCfg_sharpness1_enable = false;
        pqControlVal.sharpness1_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_SHARPNESSPI_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_sharpnesspi_enable = true;
        //pqControlVal.sharpnesspi_en = 1;
    } else {
        mbCpqCfg_sharpnesspi_enable = false;
        //pqControlVal.sharpnesspi_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DNLP_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_dnlp_enable = true;
        pqControlVal.dnlp_en = 1;
        Cpq_SetDNLPStatus(VE_DNLP_STATE_ON);
    } else {
        mbCpqCfg_dnlp_enable = false;
        pqControlVal.dnlp_en = 0;
        Cpq_SetDNLPStatus(VE_DNLP_STATE_OFF);
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CM2_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_cm2_enable = true;
        pqControlVal.cm_en = 1;
    } else {
        mbCpqCfg_cm2_enable = false;
        pqControlVal.cm_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AMVECM_BASCI_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_amvecm_basic_enable = true;
        pqControlVal.vadj1_en = 1;
    } else {
        mbCpqCfg_amvecm_basic_enable = false;
        pqControlVal.vadj1_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AMVECM_BASCI_WITHOSD_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_amvecm_basic_withOSD_enable = true;
        pqControlVal.vadj2_en = 1;
    } else {
        mbCpqCfg_amvecm_basic_withOSD_enable = false;
        pqControlVal.vadj2_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CONTRAST_RGB_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_contrast_rgb_enable = true;
        pqControlVal.vd1_ctrst_en = 1;
    } else {
        mbCpqCfg_contrast_rgb_enable = false;
        pqControlVal.vd1_ctrst_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CONTRAST_RGB_WITHOSD_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_contrast_rgb_withOSD_enable = true;
        pqControlVal.post_ctrst_en = 1;
    } else {
        mbCpqCfg_contrast_rgb_withOSD_enable = false;
        pqControlVal.post_ctrst_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_WHITEBALANCE_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_whitebalance_enable = true;
        pqControlVal.wb_en = 1;
    } else {
        mbCpqCfg_whitebalance_enable = false;
        pqControlVal.wb_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_GAMMA_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_gamma_enable = true;
        pqControlVal.gamma_en = 1;
    } else {
        mbCpqCfg_gamma_enable = false;
        pqControlVal.gamma_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_LOCAL_CONTRAST_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_local_contrast_enable = true;
        pqControlVal.lc_en = 1;
    } else {
        mbCpqCfg_local_contrast_enable = false;
        pqControlVal.lc_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_BLACKEXTENSION_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_blackextension_enable = true;
        pqControlVal.black_ext_en = 1;
    } else {
        mbCpqCfg_blackextension_enable = false;
        pqControlVal.black_ext_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_XVYCC_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_xvycc_enable = true;
    } else {
        mbCpqCfg_xvycc_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DISPLAY_OVERSCAN_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_display_overscan_enable = true;
    } else {
        mbCpqCfg_display_overscan_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_HDMI, CFG_HDMI_OUT_WITH_FBC_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_hdmi_out_with_fbc_enable = true;
    } else {
        mbCpqCfg_hdmi_out_with_fbc_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_PQ_PARAM_CHECK_SOURCE_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_pq_param_check_source_enable = true;
    } else {
        mbCpqCfg_pq_param_check_source_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AI_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_ai_enable = true;
    } else {
        mbCpqCfg_ai_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_SMOOTHPLUS_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_smoothplus_enable = true;
    } else {
        mbCpqCfg_smoothplus_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_HDRTMO_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_hdrtmo_enable = true;
    } else {
        mbCpqCfg_hdrtmo_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_MEMC_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_memc_enable = true;
    } else {
        mbCpqCfg_memc_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AAD_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_aad_enable = true;
    } else {
        mbCpqCfg_aad_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CABC_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_cabc_enable = true;
    } else {
        mbCpqCfg_cabc_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_BLUESTRETCH_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_bluestretch_enable = true;
    } else {
        mbCpqCfg_bluestretch_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CHROMACORING_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_chroma_coring_enable = true;
    } else {
        mbCpqCfg_chroma_coring_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AISR_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_aisr_enable = true;
    } else {
        mbCpqCfg_aisr_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AICOLOR_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_aicolor_enable = true;
    } else {
        mbCpqCfg_aicolor_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DONGLE_LOW_POWER, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_dongle_low_power_enable = true;
    } else {
        mbCpqCfg_dongle_low_power_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_HDMI_COLOR_RANGE_MODE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_color_range_mode_enable = true;
    } else {
        mbCpqCfg_color_range_mode_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_COLOR_SPACE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_color_space_enable = true;
    } else {
        mbCpqCfg_color_space_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_GLOBAL_DIMMING, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_global_dimming_enable = true;
    } else {
        mbCpqCfg_global_dimming_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_SUPER_RESOLUTION, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_super_resolution_enable = true;
    } else {
        mbCpqCfg_super_resolution_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_FILM_MODE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_film_mode_enable = true;
    } else {
        mbCpqCfg_film_mode_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_OSD_SHARPNEDD, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_osd_sharpness_enable = true;
    } else {
        mbCpqCfg_osd_sharpness_enable = false;
    }

    //special ui display/hatch cfg start
    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_UI_PICTURE_MODE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_ui_picture_mode_enable = true;
    } else {
        mbCpqCfg_ui_picture_mode_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_UI_BACKLIGHT, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_ui_backlight_enable = true;
    } else {
        mbCpqCfg_ui_backlight_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_UI_SHARPNESS, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_ui_sharpness_enable = true;
    } else {
        mbCpqCfg_ui_sharpness_enable = false;
    }
    //special ui display/hatch cfg end

    vpp_pq_ctrl_t amvecmConfigVal;
    amvecmConfigVal.length = 14;//this is the count of pq_ctrl_s option
    amvecmConfigVal.ptr = (long long)&pqControlVal;
    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_PQ_CTRL, &amvecmConfigVal);
    if (ret < 0) {
        SYS_LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
    }

    Cpq_SetVadjEnableStatus(pqControlVal.vadj1_en, pqControlVal.vadj2_en);

    return 0;
}

int CPQControl::HasPqCaseFunc(pq_case_func_e type)
{
    bool func_en = false;

    switch (type) {
        default:                             func_en = false;                             break;
        case PQ_CASE_FUNC_PICTURE_MODE:      func_en = mbCpqCfg_ui_picture_mode_enable;   break;
        case PQ_CASE_FUNC_BACKLIGHT:         func_en = mbCpqCfg_ui_backlight_enable;      break;
        case PQ_CASE_FUNC_CONTRAST:          func_en = mbCpqCfg_amvecm_basic_enable;      break;
        case PQ_CASE_FUNC_BRIGHTNESS:        func_en = mbCpqCfg_amvecm_basic_enable;      break;
        case PQ_CASE_FUNC_SATURATION:        func_en = mbCpqCfg_amvecm_basic_enable;      break;
        case PQ_CASE_FUNC_HUE:               func_en = mbCpqCfg_amvecm_basic_enable;      break;
        case PQ_CASE_FUNC_SHARPNESS:         func_en = mbCpqCfg_ui_sharpness_enable;      break;
        case PQ_CASE_FUNC_ASPECT_RATIO:      func_en = mbCpqCfg_display_overscan_enable;  break;
        case PQ_CASE_FUNC_AI_PQ:             func_en = hasAipqFunc();                     break;
        case PQ_CASE_FUNC_AI_COLOR:          func_en = hasAiColorFunc();                  break;
        case PQ_CASE_FUNC_AI_SR:             func_en = hasAisrFunc();                     break;
        case PQ_CASE_FUNC_GAMMA:             func_en = mbCpqCfg_gamma_enable;             break;
        case PQ_CASE_FUNC_MANUAL_GAMMA:      func_en = mbCpqCfg_gamma_enable;             break;
        case PQ_CASE_FUNC_COLOR_TEMP:        func_en = mbCpqCfg_whitebalance_enable;      break;
        case PQ_CASE_FUNC_COLOR_MANAGEMENT:  func_en = mbCpqCfg_cm2_enable;               break;
        case PQ_CASE_FUNC_COLOR_CUSTOMIZE:   func_en = mbCpqCfg_cm2_enable;               break;
        case PQ_CASE_FUNC_COLOR_RANGE_MODE:  func_en = mbCpqCfg_color_range_mode_enable;  break;
        case PQ_CASE_FUNC_COLOR_SPACE:       func_en = mbCpqCfg_color_space_enable;       break;
        case PQ_CASE_FUNC_GLOBAL_DIMMING:    func_en = mbCpqCfg_global_dimming_enable;    break;
        case PQ_CASE_FUNC_LOCAL_DIMMING:     func_en = HasLocalDimming();                 break;
        case PQ_CASE_FUNC_BLACK_STRETCH:     func_en = mbCpqCfg_blackextension_enable;    break;
        case PQ_CASE_FUNC_DNLP:              func_en = mbCpqCfg_dnlp_enable;              break;
        case PQ_CASE_FUNC_LOCAL_CONTRAST:    func_en = mbCpqCfg_local_contrast_enable;    break;
        case PQ_CASE_FUNC_SR:                func_en = mbCpqCfg_super_resolution_enable;  break;
        case PQ_CASE_FUNC_DNR:               func_en = mbCpqCfg_nr_enable;                break;
        case PQ_CASE_FUNC_DEBLOCK:           func_en = mbCpqCfg_deblock_enable;           break;
        case PQ_CASE_FUNC_DEMOSQUITO:        func_en = mbCpqCfg_demoSquito_enable;        break;
        case PQ_CASE_FUNC_DECONTOUR:         func_en = mbCpqCfg_smoothplus_enable;        break;
        case PQ_CASE_FUNC_MEMC:              func_en = mbCpqCfg_memc_enable;              break;
        case PQ_CASE_FUNC_FILM_MODE:         func_en = mbCpqCfg_film_mode_enable;         break;
        case PQ_CASE_FUNC_OSD_SHARPNESS:     func_en = mbCpqCfg_osd_sharpness_enable;     break;
        case PQ_CASE_FUNC_RESET:             func_en = true;                              break;
    }

    SYS_LOGD("%s type:%d, func_en:%d\n", __FUNCTION__, type, func_en);

    return func_en;
}

int CPQControl::GetChipType(void)
{
    int ret = 0;
    int chip_type = 0; /*detail chip type please refer enum meson_cpuid_type_e*/

    ret = VPPDeviceIOCtl(AMVECM_IOC_G_CHIP_TYPE, &chip_type);
    if (ret < 0) {
       SYS_LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
       return -1;
    }

    SYS_LOGD("%s, chip_type:%d\n", __FUNCTION__, chip_type);
    return chip_type;
}

int CPQControl::SetPLLValues(source_input_param_t source_input_param)
{
    am_regs_t regs;
    int ret = 0;
    if (mPQdb->PQ_GetPLLParams (source_input_param, &regs ) == 0 ) {
        ret = AFEDeviceIOCtl(TVIN_IOC_LOAD_REG, &regs);
        if ( ret < 0 ) {
            SYS_LOGE ( "%s error(%s)!\n", __FUNCTION__, strerror(errno));
            return -1;
        }
    } else {
        SYS_LOGE ( "%s, PQ_GetPLLParams failed!\n", __FUNCTION__ );
        return -1;
    }

    return 0;
}

int CPQControl::SetCVD2Values(void)
{
    am_regs_t regs;
    int ret = mPQdb->PQ_GetCVD2Params ( mCurrentSourceInputInfo, &regs);
    if (ret < 0) {
        SYS_LOGE ( "%s, PQ_GetCVD2Params failed!\n", __FUNCTION__);
    } else {
        ret = AFEDeviceIOCtl(TVIN_IOC_LOAD_REG, &regs);
        if ( ret < 0 ) {
            SYS_LOGE ( "%s: ioctl failed!\n", __FUNCTION__);
        }
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SSMReadNTypes(int id, int data_len, int offset)
{
    int value = 0;
    int ret = 0;

    ret = mSSMAction->SSMReadNTypes(id, data_len, &value, offset);

    if (ret < 0) {
        SYS_LOGE("Cpq_SSMReadNTypes, error(%s).\n", strerror ( errno ) );
        return -1;
    } else {
        return value;
    }
}

int CPQControl::Cpq_SSMWriteNTypes(int id, int data_len, int data_buf, int offset)
{
    int ret = 0;
    ret = mSSMAction->SSMWriteNTypes(id, data_len, &data_buf, offset);

    if (ret < 0) {
        SYS_LOGE("Cpq_SSMWriteNTypes, error(%s).\n", strerror ( errno ) );
    }

    return ret;
}

int CPQControl::Cpq_GetSSMActualAddr(int id)
{
    return mSSMAction->GetSSMActualAddr(id);
}

int CPQControl::Cpq_GetSSMActualSize(int id)
{
    return mSSMAction->GetSSMActualSize(id);
}

int CPQControl::Cpq_SSMRecovery(void)
{
    return mSSMAction->SSMRecovery();
}

int CPQControl::Cpq_GetSSMStatus()
{
    return mSSMAction->GetSSMStatus();
}

hdr_type_t CPQControl::Cpq_GetSourceHDRType(source_input_param_t source_input_param)
{
    hdr_type_t newHdrType = HDR_TYPE_NONE;
    if ((source_input_param.source_input == SOURCE_MPEG)
        ||(source_input_param.source_input == SOURCE_DTV)) {
        if (!mbVideoIsPlaying) {
            newHdrType = HDR_TYPE_SDR;
        } else {
            char buf[32] = {0};
            int ret = pqReadSys(VIDEO_POLL_PRIMARY_SRC_FMT, buf, sizeof(buf));
            if (ret < 0) {
                newHdrType = HDR_TYPE_SDR;
                SYS_LOGE("%s error: %s\n", __FUNCTION__, strerror(errno));
            } else {
                if (0 == strcmp(buf, "src_fmt = SDR")) {
                    newHdrType = HDR_TYPE_SDR;
                } else if (0 == strcmp(buf, "src_fmt = HDR10")) {
                    newHdrType = HDR_TYPE_HDR10;
                } else if (0 == strcmp(buf, "src_fmt = HDR10+")) {
                    newHdrType = HDR_TYPE_HDR10PLUS;
                } else if (0 == strcmp(buf, "src_fmt = HDR10 prime")) {
                    newHdrType = HDR_TYPE_PRIMESL;
                } else if (0 == strcmp(buf, "src_fmt = HLG")) {
                    newHdrType = HDR_TYPE_HLG;
                } else if (0 == strcmp(buf, "src_fmt = Dolby Vison")) {
                    newHdrType = HDR_TYPE_DOVI;
                } else if (0 == strcmp(buf, "src_fmt = MVC")) {
                    newHdrType = HDR_TYPE_MVC;
                } else {
                    SYS_LOGE("%s: invalid hdr type:%s\n", __FUNCTION__, buf);
                    newHdrType = HDR_TYPE_SDR;
                }
            }
        }
    } else if ((source_input_param.source_input == SOURCE_HDMI1)
             || (source_input_param.source_input == SOURCE_HDMI2)
             || (source_input_param.source_input == SOURCE_HDMI3)
             || (source_input_param.source_input == SOURCE_HDMI4)) {
        int signalRange                  = (mHdmiHdrInfo >> 29) & 0x1;
        int signalColorPrimaries         = (mHdmiHdrInfo >> 16) & 0xff;
        int signalTransferCharacteristic = (mHdmiHdrInfo >> 8)  & 0xff;
        int dvFlag                       = (mHdmiHdrInfo >> 30) & 0x1;
        SYS_LOGD("%s: signalRange= 0x%x, signalColorPrimaries = 0x%x, signalTransferCharacteristic = 0x%x, dvFlag = 0x%x\n",
                __FUNCTION__, signalRange, signalColorPrimaries, signalTransferCharacteristic, dvFlag);
        if (((signalTransferCharacteristic == 0xe) || (signalTransferCharacteristic == 0x12))
            && (signalColorPrimaries == 0x9)) {
            newHdrType = HDR_TYPE_HLG;
        } else if ((signalTransferCharacteristic == 0x30) && (signalColorPrimaries == 0x9)) {
            newHdrType = HDR_TYPE_HDR10PLUS;
        } else if ((signalTransferCharacteristic == 0x10) || (signalColorPrimaries == 0x9)) {
            newHdrType = HDR_TYPE_HDR10;
        } else if (dvFlag == 0x1) {
            newHdrType = HDR_TYPE_DOVI;
        } else {
            newHdrType = HDR_TYPE_SDR;
        }
    } else if ((source_input_param.source_input == SOURCE_TV)
             || (source_input_param.source_input == SOURCE_AV1)
             || (source_input_param.source_input == SOURCE_AV2)) {
        if (source_input_param.sig_fmt != TVIN_SIG_FMT_NULL) {
            newHdrType = HDR_TYPE_SDR;
        } else {
           newHdrType = HDR_TYPE_NONE;
        }
    } else {
        newHdrType = HDR_TYPE_NONE;
    }

    SYS_LOGD("%s: newHdrType:%d, source_input:%d\n", __FUNCTION__, newHdrType, source_input_param.source_input);

    return newHdrType;
}

int CPQControl::SetCurrentSourceInputInfo(source_input_param_t source_input_param)
{
    AutoMutex _l( mLock );
    SYS_LOGD("%s: param_check_source_enable = %d\n", __FUNCTION__, mbCpqCfg_pq_param_check_source_enable);

    SYS_LOGD("%s:new source info: source=%d,sigFmt=%d(0x%x)\n", __FUNCTION__,
                                                                 source_input_param.source_input,
                                                                 source_input_param.sig_fmt,
                                                                 source_input_param.sig_fmt);

    //get hdr type
    hdr_type_t newHdrType = HDR_TYPE_NONE;
    newHdrType = Cpq_GetSourceHDRType(source_input_param);

    //notify hdr event to framework
    if (mCurrentHdrType != newHdrType) {
        mCurrentHdrType = newHdrType;
        if (mNotifyListener != NULL) {
            SYS_LOGD("%s: send hdr event, info is %d\n", __FUNCTION__, mCurrentHdrType);
            mNotifyListener->onHdrInfoChange(mCurrentHdrType);
        } else {
            SYS_LOGE("%s: mNotifyListener is NULL\n", __FUNCTION__);
        }
    }

    if ((newHdrType == HDR_TYPE_SDR) || (newHdrType == HDR_TYPE_NONE)) {
        mPQdb->mHdrStatus = false;
    } else {
        mPQdb->mHdrStatus = true;
    }
    SYS_LOGD("%s: mCurrentHdrType is %d, hdrStatus is %d!\n", __FUNCTION__, mCurrentHdrType, mPQdb->mHdrStatus);

    //check pq src timming
    pq_src_param_t PqSrcTim;
    PqSrcTim.pq_source_input = CheckPQSource(source_input_param.source_input);
    PqSrcTim.pq_sig_fmt = CheckPQTimming(newHdrType);
    SYS_LOGD("%s:PqSrcTim.pq_source_input is %d  PqSrcTim.pq_sig_fmt is %d\n", __FUNCTION__, PqSrcTim.pq_source_input, PqSrcTim.pq_sig_fmt);

    CheckOutPutMode(source_input_param.source_input);

    //check env hdr policy (always hdr or adaptive hdr)
    getHdrPolicy();

    //check current framerate
    GetCurrentFrameRate(source_input_param.source_input);

    if ((mCurrentSourceInputInfo.source_input != source_input_param.source_input) ||
         (mCurrentSourceInputInfo.sig_fmt != source_input_param.sig_fmt) ||
         (mCurrentSourceInputInfo.trans_fmt != source_input_param.trans_fmt) ||
         (mCurrentHdrStatus != mPQdb->mHdrStatus) ||
         (mCurrentOutputType != mPQdb->mOutPutType) ||
         (CurSource != PqSrcTim.pq_source_input) ||
         (CurTimming != PqSrcTim.pq_sig_fmt) ||
         (mCurrentNodeNumber != mPQdb->node_number)) {
        mCurrentSourceInputInfo.source_input = source_input_param.source_input;
        mCurrentSourceInputInfo.sig_fmt = source_input_param.sig_fmt;
        mCurrentSourceInputInfo.trans_fmt = source_input_param.trans_fmt;
        mCurrentHdrStatus = mPQdb->mHdrStatus;
        mCurrentOutputType = mPQdb->mOutPutType;
        CurSource = PqSrcTim.pq_source_input;
        CurTimming = PqSrcTim.pq_sig_fmt;
        mCurrentNodeNumber = mPQdb->node_number;

        SYS_LOGD("%s:CurSource is %d  CurTimming is %d\n", __FUNCTION__, CurSource, CurTimming);

        if (mbCpqCfg_pq_param_check_source_enable) {
            mSourceInputForSaveParam = mCurrentSourceInputInfo.source_input;
        } else {
            mSourceInputForSaveParam = SOURCE_MPEG;
        }

        if (mPQdb->mDbMatchType == MATCH_TYPE_MBOX_T3X) {
            if (mCurrentSourceInputInfo.sig_fmt == TVIN_SIG_FMT_HDMI_3840_2160_00HZ) {
                if (mPQdb->node_number != 2) {
                    SYS_LOGD("%s: to cover same sequence issue, fix node num to 2 for 4k input\n", __FUNCTION__);
                    mPQdb->node_number = 2;
                }
            }
        }

        if (mCurrentSourceInputInfo.sig_fmt != TVIN_SIG_FMT_NULL) {
            LoadPQSettings();
        } else {
            vpp_display_mode_t display_mode = (vpp_display_mode_t)GetDisplayMode();
            SetDisplayMode(display_mode, 1);
        }
    } else {
        SYS_LOGD("%s: same signal, no need set!\n", __FUNCTION__);
    }
    return 0;
}

source_input_param_t CPQControl::GetCurrentSourceInputInfo()
{
    AutoMutex _l( mLock );
    return mCurrentSourceInputInfo;
}

int CPQControl::GetRGBPattern() {
    char value[33] = {0};
    pqReadSys(VIDEO_RGB_SCREEN, value, (sizeof(value)-1));
    value[32] = '\0';
    return strtol(value, NULL, 10);
}

int CPQControl::SetRGBPattern(int r, int g, int b) {
    int value = ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
    char str[32] = {0};
    sprintf(str, "%d", value);
    int ret = pqWriteSys(VIDEO_RGB_SCREEN, str);
    return ret;
}

int CPQControl::FactorySetDDRSSC(int step) {
    if (step < 0 || step > 5) {
        SYS_LOGE ("%s, step = %d is too long", __FUNCTION__, step);
        return -1;
    }

    return mSSMAction->SSMSaveDDRSSC(step);
}

int CPQControl::FactoryGetDDRSSC() {
    unsigned char data = 0;
    mSSMAction->SSMReadDDRSSC(&data);
    return data;
}

int CPQControl::SetLVDSSSC(int step) {

    SYS_LOGI("%s: %d\n", __FUNCTION__, step);

    char buf[32] = {0};
    sprintf(buf, "%d", step);
    int ret = pqWriteSys(LCD_SS, buf);
    return ret;
}

int CPQControl::FactorySetLVDSSSC(int step)
{
    if (step > 4)
        step = 4;

    aml_lcd_ss_ctl_t pData;
    memset(&pData, 0, sizeof(aml_lcd_ss_ctl_t));

    pData.level = step;

    int data[3] = {0, 0, 0};// level frep mode
    data[0] = pData.level;
    data[1] = pData.freq;
    data[2] = pData.mode;

    mSSMAction ->SSMSaveLVDSSSC(data);

    if (AML_HAL_LCD_SetSS((HAL_lcd_ss_ctl_t *)&pData) != API_OK) {
        SYS_LOGE("%s: fail\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetLVDSSSC()
{
    int data[3] = {0, 0, 0};

    mSSMAction ->SSMReadLVDSSSC(data);

    int level = data[0];

    return level;
}

int CPQControl::SetLCDPowerCtrl(int state)
{
    unsigned int onoff = state;

    if (AML_HAL_LCD_SetPowerCtrl(onoff) != API_OK) {
        SYS_LOGE("%s: fail\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::SetLCDMuteCtrl(int state)
{
    unsigned int onoff = state;

    if (AML_HAL_LCD_SetMuteCtrl(onoff) != API_OK) {
        SYS_LOGE("%s: fail\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::SetGrayPattern(int value) {
    if (value < 0) {
        value = 0;
    } else if (value > 255) {
        value = 255;
    }
    value = value << 16 | 0x8080;

    SYS_LOGI("%s: %d\n", __FUNCTION__, value);

    char val[64] = {0};
    sprintf(val, "%d", value);
    return pqWriteSys(VIDEO_TEST_SCREEN, val);
}

int CPQControl::GetGrayPattern() {
    int value = 0;
    char temp[9];
    memset(temp, 0, sizeof(temp));
    int ret = pqReadSys(VIDEO_TEST_SCREEN, temp, (sizeof(temp)-1));
    temp[8] = '\0';
    value = strtol(temp, NULL, 16);

    if (value < 0) {
        return 0;
    } else {
        value = value >> 16;
        if (value > 255) {
            value = 255;
        }
        return value;
    }
}

int CPQControl::SetHDRMode(int mode)
{
    int ret = -1;
    if ((CurSource == PQ_SRC_MPEG) || ((CurSource >= PQ_SRC_HDMI1) && CurSource <= PQ_SRC_HDMI4)) {
        ret = VPPDeviceIOCtl(AMVECM_IOC_S_CSCTYPE, &mode);
        if (ret < 0) {
            SYS_LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
        }
    } else {
        SYS_LOGE("%s: Current source no hdr status!\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::GetHDRMode()
{
    ve_csc_type_t mode = VPP_MATRIX_NULL;
    if ((CurSource == PQ_SRC_MPEG) || ((CurSource >= PQ_SRC_HDMI1) && CurSource <= PQ_SRC_HDMI4)) {
        int ret = VPPDeviceIOCtl(AMVECM_IOC_G_CSCTYPE, &mode);
        if (ret < 0) {
            SYS_LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
            mode = VPP_MATRIX_NULL;
        } else {
            SYS_LOGI("%s: mode is %d\n", __FUNCTION__, mode);
        }
    } else {
        SYS_LOGI("%s: Current source no hdr status!\n", __FUNCTION__);
    }

    return mode;
}

int CPQControl::GetSourceHDRType()
{
    SYS_LOGI("%s: type is %d\n", __FUNCTION__, mCurrentHdrType);
    return mCurrentHdrType;
}

void CPQControl::setHdrInfoListener(const sp<PqNotify>& listener) {
    mNotifyListener = listener;
}

void CPQControl::GetChipVersionInfo(char* chip_version) {
    database_attribute_t dbAttribute;
    mPQdb->PQ_GetDataBaseAttribute(&dbAttribute);
    if ((dbAttribute.ChipVersion.c_str() == NULL) || (dbAttribute.ChipVersion.length() == 0)) {
        SYS_LOGI("%s: ChipVersion is null\n", __FUNCTION__);
        std::strcpy(chip_version, " ");
    } else {
        std::string TempString = std::string(dbAttribute.ChipVersion.c_str());
        char* tempstr = new char[TempString.length() + 1];
        std::strcpy(tempstr, TempString.c_str());
        chip_version = strtok(tempstr, "_");

        SYS_LOGI("%s: versionStr is %s\n", __FUNCTION__, chip_version);
        delete []tempstr;
    }
}

tvpq_databaseinfo_t CPQControl::GetDBVersionInfo(db_name_t name) {
    bool val = false;
    String8 tmpToolVersion, tmpProjectVersion, tmpGenerateTime;
    tvpq_databaseinfo_t pqdatabaseinfo_t;
    memset(&pqdatabaseinfo_t, 0, sizeof(pqdatabaseinfo_t));
    switch (name) {
        case DB_NAME_PQ:
            val = mPQdb->PQ_GetPqVersion(tmpToolVersion, tmpProjectVersion, tmpGenerateTime);
            break;
        case DB_NAME_OVERSCAN:
            val = mpOverScandb->GetOverScanDbVersion(tmpToolVersion, tmpProjectVersion, tmpGenerateTime);
            break;
        default:
            val = mPQdb->PQ_GetPqVersion(tmpToolVersion, tmpProjectVersion, tmpGenerateTime);
            break;
    }

    if (val) {
        if (strlen(tmpToolVersion.c_str()) < sizeof(pqdatabaseinfo_t.ToolVersion)/sizeof(char)) {
            strcpy(pqdatabaseinfo_t.ToolVersion, tmpToolVersion.c_str());
        }
        if (strlen(tmpProjectVersion.c_str()) < sizeof(pqdatabaseinfo_t.ProjectVersion)/sizeof(char)) {
            strcpy(pqdatabaseinfo_t.ProjectVersion, tmpProjectVersion.c_str());
        }
        if (strlen(tmpGenerateTime.c_str()) < sizeof(pqdatabaseinfo_t.GenerateTime)/sizeof(char)) {
            strcpy(pqdatabaseinfo_t.GenerateTime, tmpGenerateTime.c_str());
        }
    }

    return pqdatabaseinfo_t;
}

int CPQControl::SetCurrentHdrInfo (int hdrInfo)
{
    int ret = 0;
    SYS_LOGI("%s: mHdmiHdrInfo:%d new hdrInfo:%d\n", __FUNCTION__, mHdmiHdrInfo, hdrInfo);

    if (mHdmiHdrInfo != (unsigned int)hdrInfo) {
        mHdmiHdrInfo = (unsigned int)hdrInfo;
        //get hdr type
        hdr_type_t newHdrType = HDR_TYPE_NONE;
        newHdrType            = Cpq_GetSourceHDRType(mCurrentSourceInputInfo);

        //notify hdr event to framework
        if (mCurrentHdrType != newHdrType) {
            mCurrentHdrType = newHdrType;
            if (mNotifyListener != NULL) {
                SYS_LOGI("%s: send hdr event, info is %d\n", __FUNCTION__, mCurrentHdrType);
                mNotifyListener->onHdrInfoChange(mCurrentHdrType);
            } else {
                SYS_LOGE("%s: mNotifyListener is NULL\n", __FUNCTION__);
            }
        }
    } else {
        SYS_LOGI("%s: same HDR info\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::SetCurrentAspectRatioInfo(tvin_aspect_ratio_e aspectRatioInfo)
{
    int ret = 0;
    if (mCurrentAfdInfo != aspectRatioInfo) {
        mCurrentAfdInfo = aspectRatioInfo;
        SYS_LOGD("%s mCurrentAfdInfo:%d\n", __FUNCTION__, mCurrentAfdInfo);
        vpp_display_mode_t display_mode = (vpp_display_mode_t)GetDisplayMode();
        ret = SetDisplayMode(display_mode, 1);
    } else {
        SYS_LOGI("%s: same AFD info\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::SetDtvKitSourceEnable(bool isEnable)
{
    SYS_LOGI("%s: isEnable:%d\n", __FUNCTION__, isEnable);

    mbDtvKitEnable = isEnable;
    return 0;
}

//AI
bool CPQControl::hasAipqFunc()
{
    int ret = -1;
    SYS_LOGI("%s, hasAipqFunc\n", __FUNCTION__);
    if (mbCpqCfg_ai_enable && isFileExist(pqSysWrite->getSysNode(AIPQ_PARAMETERS_UVM_OPEN))) {
        ret = true;
    } else {
        ret = false;
    }

    SYS_LOGI("%s, has aipq or not:%d\n", __FUNCTION__, ret);
    return ret;
}

int CPQControl::SetAipqEnable(bool isEnable)
{
    SYS_LOGI("%s, SetAipqEnable isEnable:%d\n", __FUNCTION__, isEnable);
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    pData.aipq_enable = isEnable ? 1 : 0;

    if (!SetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s SetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    enableAipq(isEnable);
    return 0;
}

int CPQControl::GetAipqEnable()
{
    int data = 0;
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return data;
    }

    data = pData.aipq_enable;

    if (data < 0 || data > 1) {
        data = 0;
    }

    SYS_LOGI("%s, data:%d\n", __FUNCTION__, data);
    return data;
}

void CPQControl::enableAipq(bool isEnable)
{
    SYS_LOGI("%s, enableAipq\n", __FUNCTION__);
    pqWriteSys(AIPQ_PARAMETERS_UVM_OPEN, isEnable ? "1" : "0");
}

int CPQControl::SetAipqMode(aipq_mode_e mode, int is_save)
{
    SYS_LOGI("%s mode = %d is_save = %d\n", __FUNCTION__, mode, is_save);
    int ret = -1;
    ret = Cpq_SetAipqMode(mode, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveAipqMode((int)mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed\n", __FUNCTION__);
    } else {
        SYS_LOGI("%s success\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::GetAipqMode()
{
    int data = 0;
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return data;
    }

    data = pData.aipq_mode;

    SYS_LOGI("%s, data = %d\n", __FUNCTION__, data);
    return data;
}

int CPQControl::SaveAipqMode(int mode)
{
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    pData.aipq_mode = mode;

    if (!SetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s SetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetAipqMode(aipq_mode_e mode, source_input_param_t source_input_param)
{
    if (isGameMode()) {
        mode = AIPQ_MODE_OFF;
        SYS_LOGE("%s, isGameMode Set AI PQ mode OFF!!!\n", __FUNCTION__);
    }

    SYS_LOGI("%s mode = %d\n", __FUNCTION__, mode);
    if (!mbCpqCfg_ai_enable) {
        SYS_LOGE("%s: ai is disable\n", __FUNCTION__);
        return 0;
    }

    int ai_param_buf_size = 127; //contain last char '\0'
    ai_pic_table_t aiRegs;
    memset(&aiRegs, 0, sizeof(ai_pic_table_t));
    aiRegs.table_ptr = malloc(ai_param_buf_size);
    if (mPQdb->PQ_GetAIParams(mode, mCurrentSourceInputInfo, &aiRegs) < 0) {
        SYS_LOGE("%s: get AI pq params failed\n", __FUNCTION__);
        return -1;
    }

    //SYS_LOGI("%s: width: %d, height: %d\n", __FUNCTION__, aiRegs.width, aiRegs.height);
    //SYS_LOGD("%s: table_ptr): %s\n", __FUNCTION__, (char *)aiRegs.table_ptr);

    if (VPPDeviceIOCtl(AMVECM_IOC_S_AIPQ_TABLE, &aiRegs) < 0) {
        SYS_LOGE("%s: iocontrol failed\n", __FUNCTION__);
        free(aiRegs.table_ptr);
        return -1;
    }

    free(aiRegs.table_ptr);
    SetAipqEnable((mode > AIPQ_MODE_OFF) ? true : false);
    SYS_LOGI("%s success\n", __FUNCTION__);
    return 0;
}

bool CPQControl::hasAisrFunc()
{
    if (isFileExist(pqSysWrite->getSysNode(AISR_PARAMETERS_UVM_OPEN_NN))) {
        SYS_LOGI("%s, has aisr\n", __FUNCTION__);
        return true;
    }

    SYS_LOGI("%s, has not aisr\n", __FUNCTION__);
    return false;
}

int CPQControl::SetAiSrEnable(bool isEnable)
{
    SYS_LOGI("%s isEnable = %d\n", __FUNCTION__, isEnable);
    int ret = -1;
    ret = Cpq_SetAiSrEnable(isEnable);

    if (ret < 0) {
        SYS_LOGE("%s Cpq_SetAiSrEnable fail\n", __FUNCTION__);
        return ret;
    } else {
        ret = SaveAiSrEnable(isEnable);
        property_set(PROP_MEDIA_AISR, isEnable > 0 ? "true" : "false");
    }

    if (ret < 0) {
        SYS_LOGE("%s failed\n", __FUNCTION__);
    } else {
        SYS_LOGI("%s success\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::GetAiSrEnable()
{
    int data = 0;
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return data;
    }

    data = pData.aisr_enable;

    if (data < 0 || data > 1) {
        data = 0;
    }

    return data;
}

int CPQControl::SaveAiSrEnable(bool enable)
{
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    pData.aisr_enable = enable ? 1 : 0;

    if (!SetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetAiSrEnable(bool enable)
{
    if (!mbCpqCfg_aisr_enable) {
        SYS_LOGD("%s disabled\n", __FUNCTION__);
        return 0;
    }

    if (pqWriteSys(VIDEO_AISR_ENABLE, enable ? "1" : "0") < 0) {
        SYS_LOGE("%s failed!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::SetAiSrMode(aisr_mode_e mode, int is_save)
{
    SYS_LOGI("%s mode: %d is_save: %d\n", __FUNCTION__, mode, is_save);
    int ret = -1;
    ret = Cpq_SetAiSrMode(mode, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveAiSrMode((int)mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed\n", __FUNCTION__);
    } else {
        SYS_LOGI("%s success\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::GetAiSrMode()
{
    int data = 0;
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return data;
    }

    data = pData.aisr_mode;

    if (GetChipType() == 0x38 && data > 1) { //MESON_CPU_MAJOR_ID_T3 = 0x38
        /*T3 aisr is two level, so just use 0 and 1, but UI bin default value maybe 2 or 3*/
        data = 1;
    }

    SYS_LOGI("%s, data = %d\n", __FUNCTION__, data);
    return data;
}

int CPQControl::SaveAiSrMode(int mode)
{
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    pData.aisr_mode = mode;

    if (!SetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s SetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetAiSrMode(aisr_mode_e mode, source_input_param_t source_input_param)
{
    if (!hasAisrFunc()) {
        SYS_LOGD("%s not support\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_aisr_enable) {
        SYS_LOGD("%s: AiSr disabled!\n", __FUNCTION__);
        return 0;
    }

    if (isGameMode()) {
        mode = AISR_MODE_OFF;
        SYS_LOGE("%s, isGameMode Set AI SR mode OFF!!!\n", __FUNCTION__);
    }

    SYS_LOGI("%s mode = %d\n", __FUNCTION__, mode);

    if (GetChipType() == 0x38) { //MESON_CPU_MAJOR_ID_T3 = 0x38
        SYS_LOGI("%s two level aisr project, no need load reg table\n", __FUNCTION__);
        goto SET_ENABLE;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetAiSrParams(mode, source_input_param, &regs) < 0) {
        SYS_LOGE("%s PQ_GetAiSrParams failed!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

SET_ENABLE:
    SetAiSrEnable((mode > AISR_MODE_OFF) ? true : false);

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

bool CPQControl::hasAiColorFunc()
{
    int ret = -1;
    SYS_LOGI("%s, hasAiColorFunc\n", __FUNCTION__);
    if (mbCpqCfg_aicolor_enable && isFileExist(pqSysWrite->getSysNode(AICOLOR_PARAMETERS_UVM_OPEN))) {
        ret = true;
    } else {
        ret = false;
    }

    SYS_LOGI("%s, has aicolor or not:%d\n", __FUNCTION__, ret);
    return ret;
}

int CPQControl::SetAiColor(int value, int is_save)
{
    SYS_LOGI("%s value = %d\n", __FUNCTION__, value);
    int ret = -1;

    ret = Cpq_SetAiColor(value);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveAiColor(value);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed\n", __FUNCTION__);
    } else {
        SYS_LOGI("%s success\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::GetAiColor(void)
{
    int data = 0;
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return data;
    }

    data = pData.ai_color;

    SYS_LOGI("%s, data = %d\n", __FUNCTION__, data);
    return data;
}

int CPQControl::SaveAiColor(int value)
{
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    pData.ai_color = value;

    if (!SetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s SetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetAiColor(int value)
{
    SYS_LOGI("%s value = %d\n", __FUNCTION__, value);

    if (!mbCpqCfg_aicolor_enable) {
        SYS_LOGD("%s: AiColor disabled!\n", __FUNCTION__);
        return 0;
    }

    int ret = -1;
    ret = VPPDeviceIOCtl(AMVECM_IOC_AI_COLOR_EN, &value);
    pqWriteSys(AICOLOR_PARAMETERS_UVM_OPEN,  value ? "1" : "0");

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGI("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::HasAiFace(void)
{
    char buf[32] = {0};

    if (pqReadSys(VIDEO_AIFACE_ENABLE, buf, sizeof(buf)) > 0) {
        SYS_LOGI("%s has aiface\n", __FUNCTION__);
        return 0;
    } else {
        SYS_LOGE("%s read VIDEO_AIFACE_ENABLE failed\n", __FUNCTION__);
        return -1;
    }
}

int CPQControl::SetAiFaceEnable(bool isEnable)
{
    SYS_LOGI("%s isEnable:%d\n", __FUNCTION__, isEnable);
    int ret = -1;

    ret = pqWriteSys(VIDEO_AIFACE_ENABLE, isEnable ? "1" : "0");

    if (ret < 0) {
        SYS_LOGE("%s failed\n", __FUNCTION__);
    } else {
        SYS_LOGI("%s success\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::GetAiFaceEnable(void)
{
    int enable = 0;
    char buf[32] = {0};

    if (pqReadSys(VIDEO_AIFACE_ENABLE, buf, sizeof(buf)) > 0) {
        enable = atoi(buf);
    } else {
        SYS_LOGE("%s read VIDEO_AIFACE_ENABLE failed!\n", __FUNCTION__);
    }

    SYS_LOGI("%s enable:%d\n", __FUNCTION__, enable);

    return enable;
}

//DLG
int CPQControl::SetDLGEnable(int enable, int is_save)
{
    int ret = 0;
    SYS_LOGI("%s, source:%d, enable:%d\n", __FUNCTION__, mSourceInputForSaveParam, enable);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveDLGEnable(enable);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed\n",__FUNCTION__);
    } else {
        SYS_LOGI("%s success\n",__FUNCTION__);
    }
    return ret;
}

int CPQControl::GetDLGEnable()
{
    int data = 0;
    mSSMAction->SSMReadDLGEnable(&data);
    SYS_LOGI(" %s, data = %d\n", __FUNCTION__, data);

    if (data < 0 || data > 1) {
        data = 0;
    }
    return data;
}

int CPQControl::SaveDLGEnable(int enable)
{
    int ret = mSSMAction->SSMSaveDLGEnable(enable);

    if (ret < 0) {
        SYS_LOGE("%s failed\n",__FUNCTION__);
    } else {
        SYS_LOGI("%s success\n",__FUNCTION__);
    }

    return ret;
}

//color space
int CPQControl::SetColorGamutMode(vpp_colorgamut_mode_t value, int is_save)
{
    int ret =0;
    SYS_LOGI("%s, source: %d, value: %d\n", __FUNCTION__, mSourceInputForSaveParam, value);
    ret = Cpq_SetColorGamutMode(value, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveColorGamutMode(value);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed\n",__FUNCTION__);
    } else {
        SYS_LOGI("%s success\n",__FUNCTION__);
    }
    return 0;
}

int CPQControl::GetColorGamutMode(void)
{
    int data = VPP_COLORGAMUT_MODE_AUTO;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d data: %d fail\n",__FUNCTION__, CurSource, CurTimming, data);
        return -1;
    }

    data = para.ColorGamut;

    SYS_LOGI("%s:source: %d, timming :%d, value: %d\n", __FUNCTION__, CurSource, CurTimming, data);
    if (data < VPP_COLORGAMUT_MODE_SRC || data > VPP_COLORGAMUT_MODE_NATIVE) {
        data = VPP_COLORGAMUT_MODE_AUTO;
    }

    return data;
}

int CPQControl::SaveColorGamutMode(vpp_colorgamut_mode_t value)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    para.ColorGamut = (int)value;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    SYS_LOGI("%s success\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetColorGamutMode(vpp_colorgamut_mode_t value, source_input_param_t source_input_param)
{
    char val[64] = {0};
    sprintf(val, "%d", value);
    //need driver support
    return 0;
}

//SmoothPlus mode
int CPQControl::SetSmoothPlusMode(int smoothplus_mode, int is_save)
{
    SYS_LOGI("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, smoothplus_mode);
    int ret = Cpq_SetSmoothPlusMode((vpp_smooth_plus_mode_t)smoothplus_mode, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveSmoothPlusMode((vpp_smooth_plus_mode_t)smoothplus_mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGI("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetSmoothPlusMode(void)
{
    int mode = VPP_SMOOTH_PLUS_MODE_MID;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return mode;
    }

    mode = para.Decontour;

    if (mode < VPP_SMOOTH_PLUS_MODE_OFF || mode > VPP_SMOOTH_PLUS_MODE_AUTO) {
        mode = VPP_SMOOTH_PLUS_MODE_MID;
    }

    SYS_LOGI("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, mode);
    return mode;
}

int CPQControl::SaveSmoothPlusMode(int smoothplus_mode)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d smoothplus_mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, smoothplus_mode);
        return -1;
    }

    para.Decontour = smoothplus_mode;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d smoothplus_mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, smoothplus_mode);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetSmoothPlusMode(vpp_smooth_plus_mode_t smoothplus_mode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_smoothplus_enable) {
        SYS_LOGI("Smooth Plus disabled\n");
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetSmoothPlusParams(smoothplus_mode, source_input_param, &regs) < 0) {
        SYS_LOGE("PQ_GetSmoothPlusParams failed!\n");
        return -1;
    }

    am_pq_param_t di_regs;
    memset(&di_regs, 0x0,sizeof(am_pq_param_t));
    di_regs.table_name = TABLE_NAME_SMOOTHPLUS;
    di_regs.table_len = regs.length;
    am_reg_t tmp_buf[regs.length];
    for (unsigned int i = 0; i < regs.length; i++) {
          tmp_buf[i].addr = regs.am_reg[i].addr;
          tmp_buf[i].mask = regs.am_reg[i].mask;
          tmp_buf[i].type = regs.am_reg[i].type;
          tmp_buf[i].val  = regs.am_reg[i].val;
    }
    di_regs.table_ptr = (long long)tmp_buf;

    if (DI_LoadRegs(di_regs) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

bool CPQControl::hasSmoothPlusFunc(void)
{
    return mbCpqCfg_smoothplus_enable;
}

int CPQControl::SetHDRTMData(int *reGain)
{
    int ret = -1;
    if (reGain == NULL) {
        SYS_LOGE("%s: reGain is NULL.\n", __FUNCTION__);
    } else {
        int i = 0;
        vpp_hdr_tone_mapping_t hdrToneMapping;
        memset(&hdrToneMapping, 0, sizeof(vpp_hdr_tone_mapping_t));
        hdrToneMapping.lut_type = LUT_TYPE_HLG;
        hdrToneMapping.lutlength = 149;
        hdrToneMapping.tm_lut = reGain;

        SYS_LOGI("hdrToneMapping.lut_type = %d\n", hdrToneMapping.lut_type);
        SYS_LOGI("hdrToneMapping.lutlength = %d\n", hdrToneMapping.lutlength);
        //SYS_LOGV("hdrToneMapping.tm_lut = %s\n", hdrToneMapping.tm_lut);

        ret = VPPDeviceIOCtl(AMVECM_IOC_S_HDR_TM, &hdrToneMapping);
    }

    if (ret < 0) {
        SYS_LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    } else {
        SYS_LOGI("%s success!\n", __FUNCTION__);
    }

    return 0;
}

//HDR TMO
int CPQControl::SetHDRTMOMode(hdr_tmo_t mode, int is_save)
{
    SYS_LOGI("%s, source: %d, mode = %d\n", __FUNCTION__, mSourceInputForSaveParam, mode);
    int ret = Cpq_SetHDRTMOMode((int)mode);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveHDRTMOMode(mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGI("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetHDRTMOMode(void)
{
    int mode = HDR_TMO_DYNAMIC;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return mode;
    }

    mode = para.HdrTmo;

    if (mode < HDR_TMO_OFF || mode > HDR_TMO_STATIC) {
        mode = HDR_TMO_DYNAMIC;
    }

    return mode;
}

int CPQControl::SaveHDRTMOMode(hdr_tmo_t mode)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return -1;
    }

    para.HdrTmo = (int)mode;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d mode: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetHDRTMOMode(int mode)
{
    if (!mbCpqCfg_hdrtmo_enable) {
        SYS_LOGD("%s HDRTMO disable!\n", __FUNCTION__);
        return 0;
    }

    if (mode < 0) {
        SYS_LOGD("%s Skip Hdr Tmo!\n", __FUNCTION__);
        return 0;
    }

    //patch start: for chips that no "HdrTmo" in PM5 XML, but use HDR_TMO_DYNAMIC as default
    if (mPQdb->mDbMatchType != MATCH_TYPE_MBOX_T3X) {
        mode = HDR_TMO_DYNAMIC;
    }
    SYS_LOGI("%s, mode = %d\n", __FUNCTION__, mode);
    //patch end

    hdr_tmo_sw_s hdrtmo_param;
    if (mPQdb->PQ_GetHDRTMOParams(mCurrentSourceInputInfo, (hdr_tmo_t)mode, &hdrtmo_param) < 0) {
        SYS_LOGE("mPQdb->PQ_GetHDRTMOParams failed!\n");
        return -1;
    }

    if (VPPDeviceIOCtl(AMVECM_IOC_S_HDR_TMO, &hdrtmo_param) < 0) {
        SYS_LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
        return -1;
    }

    SYS_LOGD("%s Success!\n", __FUNCTION__);
    return 0;
}

int CPQControl::SetBlackStretch(int level, int is_save)
{
    SYS_LOGI("%s, level = %d\n", __FUNCTION__, level);
    int ret = -1;
    ret = Cpq_SetBlackStretch(level, mCurrentSourceInputInfo);

    if (ret == 0 && is_save == 1) {
        SaveBlackStretch(level);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGI("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetBlackStretch(void)
{
    int level = VPP_PQ_LV_OFF;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d level: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
        return level;
    }

    level = para.BlackStretch;

    if (level < VPP_PQ_LV_OFF || level >= VPP_PQ_LV_MAX) {
        level = VPP_PQ_LV_OFF;
    }

    SYS_LOGI("%s:source: %d, timming: %d, value: %d\n", __FUNCTION__, CurSource, CurTimming, level);
    return level;
}

int CPQControl::SaveBlackStretch(int level)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d level: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
        return -1;
    }

    para.BlackStretch = level;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d level: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
        return -1;
    }

    SYS_LOGI("%s %d success!\n",__FUNCTION__, level);
    return 0;
}

int CPQControl::Cpq_SetBlackStretch(int level,source_input_param_t source_input_param)
{
    if (!mbCpqCfg_blackextension_enable) {
        SYS_LOGD("%s: BlackStretch disabled!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetBlackStretchParams(level,source_input_param, &regs) < 0) {
        SYS_LOGE("%s: PQ_GetBlackStretchParams failed!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::SetBlueStretch(int level, int is_save)
{
    SYS_LOGI("%s, level = %d\n", __FUNCTION__, level);
    int ret = -1;
    ret = Cpq_SetBlueStretch(level, mCurrentSourceInputInfo);

    if (ret == 0 && is_save == 1) {
        SaveBlueStretch(level);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGI("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetBlueStretch(void)
{
    int level = VPP_PQ_LV_OFF;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d level: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
        return level;
    }

    level = para.BlueStretch;

    if (level < VPP_PQ_LV_OFF || level >= VPP_PQ_LV_MAX) {
        level = VPP_PQ_LV_OFF;
    }

    return level;
}

int CPQControl::SaveBlueStretch(int level)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d level: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
        return -1;
    }

    para.BlueStretch = level;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d level: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetBlueStretch(int level, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_bluestretch_enable) {
        SYS_LOGD("%s: BlueStretch disabled!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetBlueStretchParams(level,source_input_param, &regs) < 0) {
        SYS_LOGE("%s: PQ_GetBlueStretchParams failed!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::SetChromaCoring(int level, int is_save)
{
    SYS_LOGI("%s, level = %d\n", __FUNCTION__, level);
    int ret = -1;
    ret = Cpq_SetChromaCoring(level, mCurrentSourceInputInfo);

    if (ret == 0 && is_save == 1) {
        SaveChromaCoring(level);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGI("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetChromaCoring(void)
{
    int level = VPP_PQ_LV_OFF;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d level: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
        return level;
    }

    level = para.ChromaCoring;

    if (level < VPP_PQ_LV_OFF || level >= VPP_PQ_LV_MAX) {
        level = VPP_PQ_LV_OFF;
    }

    return level;
}

int CPQControl::SaveChromaCoring(int level)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d level: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
        return -1;
    }

    para.ChromaCoring = level;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d level: %d fail\n",__FUNCTION__, CurSource, CurTimming, level);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetChromaCoring(int level,source_input_param_t source_input_param)
{
    if (!mbCpqCfg_chroma_coring_enable) {
        SYS_LOGD("%s: ChromaCoring disabled!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetChromaCoringParams(level,source_input_param, &regs) < 0) {
        SYS_LOGE("%s: PQ_GetChromaCoringParams failed!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadRegs(regs) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

bool CPQControl::HasLocalDimming(void)
{
    if (AML_HAL_LD_IsExist() != API_OK) {
        return false;
    }

    return true;
}

int CPQControl::SetLocalDimming(int level, int is_save)
{
    SYS_LOGD("%s, level = %d\n", __FUNCTION__, level);

    if (Cpq_SetLocalDimming((vpp_pq_level_t)level) < 0) {
        SYS_LOGE("%s, Cpq_LocalDimming level= %d fail\n", __FUNCTION__, level);
        return -1;
    }

    if (is_save == 1) {
        if (SaveLocalDimming(level) < 0) {
            SYS_LOGE("%s failed\n",__FUNCTION__);
            return -1;
        }
    }

    SYS_LOGD("%s success\n",__FUNCTION__);
    return 0;
}

int CPQControl::GetLocalDimming(void)
{
    int level = VPP_PQ_LV_OFF;
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return level;
    }

    level = pData.LocalDimming;

    if (level < VPP_PQ_LV_OFF || level >= VPP_PQ_LV_MAX) {
        level = VPP_PQ_LV_OFF;
    }

    SYS_LOGI("%s, source: %d, level = %d\n", __FUNCTION__, mSourceInputForSaveParam, level);
    return level;
}

int CPQControl::SaveLocalDimming(int level)
{
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    pData.LocalDimming = level;

    if (!SetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetLocalDimming(vpp_pq_level_t level)
{
    if (!HasLocalDimming()) {
        SYS_LOGD("%s: LocalDimming disabled!\n", __FUNCTION__);
        return 0;
    }

    if (AML_HAL_LD_SetLevelIdx((int)level) != API_OK) {
        SYS_LOGE("%s, AML_HAL_LD_SetLevelIdx level= %d fail\n", __FUNCTION__, level);
        return -1;
    }

    return 0;
}

int PQDemoMemcState = PQ_DEMO_STATE_OFF;
int PQDemoAisrState = PQ_DEMO_STATE_OFF;
int PQDemoAisrWin   = PQ_DEMO_AISR_WIN_ON;

int CPQControl::SetPQModuleDemoState(pq_module_demo_t modules, pq_module_demo_state_t state)
{
    SYS_LOGD("%s, modules:%d, state:%d\n",__FUNCTION__, modules, state);
    int ret = -1;
    switch (modules) {
        case PQ_DEMO_MEMC://left memc on, right memc off
            if (hasMemcFunc()) {
                ret = pqWriteSys(PQ_MODULE_MEMC_DEMO_WIN, (state == 1) ? "demo_win 1" : "demo_win 0");
                PQDemoMemcState = state;
            } else {
                SYS_LOGE("%s MEMC Module disabled\n",__FUNCTION__);
                ret = -1;
            }
            break;
        case PQ_DEMO_AISR:
            if (mbCpqCfg_aisr_enable) {
                ret = pqWriteSys(PQ_MODULE_AISR_DEMO_EN, (state == 1) ? "1" : "0");
                PQDemoAisrState = state;

                    if (state == PQ_DEMO_STATE_ON && mCurrentOutputType == OUTPUT_TYPE_LVDS) {//tv
                        ret = pqWriteSys(PQ_MODULE_AISR_DEMO_AXIS, "0 0 1919 2159");//default 4k
                    } else if (state == PQ_DEMO_STATE_ON && mCurrentOutputType != OUTPUT_TYPE_LVDS) {//ott
                        if (mOutPutFrameHeightType == UHD_HEIGHT_4320) {
                            ret = pqWriteSys(PQ_MODULE_AISR_DEMO_AXIS, "0 0 3839 4319");//8k output
                        } else if (mOutPutFrameHeightType == UHD_HEIGHT_2160) {
                            ret = pqWriteSys(PQ_MODULE_AISR_DEMO_AXIS, "0 0 1919 2159");//4k output
                        } else if (mOutPutFrameHeightType <= FHD_HEIGHT_1080) {
                            ret = pqWriteSys(PQ_MODULE_AISR_DEMO_AXIS, "0 0 960 1079");//1080 output
                        }
                    } else {
                        SYS_LOGD("%s AISR Module Demo disabled\n",__FUNCTION__);
                    }

            } else {
                SYS_LOGE("%s AISR Module disabled\n",__FUNCTION__);
                ret = -1;
            }
            break;
        default:
            SYS_LOGE("%s This Module ：%d is missing \n",__FUNCTION__, modules);
            ret = -1;
            break;
    }

    if (ret < 0) {
        SYS_LOGE("%s failed\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success\n",__FUNCTION__);
    }
    return ret;
}

int CPQControl::GetPQModuleDemoState(int modules)
{
    int state = 0;

    switch (modules) {
        case PQ_DEMO_MEMC:
            state = PQDemoMemcState;
            break;
        case PQ_DEMO_AISR:
            state = PQDemoAisrState;
            break;
        default:
            SYS_LOGE("%s This Module ：%d is missing \n",__FUNCTION__, modules);
            break;
    }

    if (state < PQ_DEMO_STATE_OFF || state >= PQ_DEMO_STATE_MAX) {
        SYS_LOGE("%s state: %d out of range\n", __FUNCTION__, state);
        state = PQ_DEMO_STATE_OFF;
    }

    return state;
}

int CPQControl::SetPQModuleDemoAisrWin(pq_module_demo_aisr_win_t aisr_win)
{
    SYS_LOGD("%s, aisr_win:%d\n",__FUNCTION__, aisr_win);

    int ret = -1;

    ret = pqWriteSys(PQ_MODULE_AISR_DEMO_WIN, (aisr_win == 1) ? "1" : "0");
    PQDemoAisrWin = aisr_win;

    if (ret < 0) {
        SYS_LOGE("%s failed\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success\n",__FUNCTION__);
    }
    return ret;
}

int CPQControl::GetPQModuleDemoAisrWin(void)
{
    int aisr_win = 0;
    aisr_win = PQDemoAisrWin;

    if (aisr_win < PQ_DEMO_AISR_WIN_OFF || aisr_win >= PQ_DEMO_AISR_WIN_MAX) {
        SYS_LOGE("%s aisr_win: %d out of range\n", __FUNCTION__, aisr_win);
        aisr_win = PQ_DEMO_AISR_WIN_ON;
    }

    return aisr_win;
}

void CPQControl::resetAllUserSettingParam()
{
    ResetNonlinearDataAll();
    ResetColorTemperatureDataAll();
    ResetColorCustomizeDataAll();
    ResetPictureStructDataBySrcAll();
    ResetPictureStructDataGlobal();
    ResetPictureModeAll();
    ResetPictureModeDataAll();

    return;
}

void CPQControl::resetSSMData(void)
{
    int config_val = 0;
    const char *buf = NULL;

    config_val = mPQConfigFile->GetInt(CFG_SECTION_HDMI, CFG_EDID_VERSION_DEF, 0);
    mSSMAction->SSMEdidRestoreDefault(config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_HDMI, CFG_HDCP_SWITCHER_DEF, 0);
    mSSMAction->SSMHdcpSwitcherRestoreDefault(0);

    buf = mPQConfigFile->GetString(CFG_SECTION_HDMI, CFG_COLOR_RANGE_MODE_DEF, "default");
    if (strcmp(buf, "full") == 0)
        mSSMAction->SSMSColorRangeModeRestoreDefault(1);
    else if (strcmp(buf, "limit") == 0)
        mSSMAction->SSMSColorRangeModeRestoreDefault(2);
    else
        mSSMAction->SSMSColorRangeModeRestoreDefault(0);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_COLORDEMOMODE_DEF, VPP_COLOR_DEMO_MODE_ALLON);
    mSSMAction->SSMSaveColorDemoMode(config_val);

    return;
}

void CPQControl::pqTransformStringToInt(const char *buf, int *val)
{
    if (buf != NULL) {
        //SYS_LOGD("%s: %s\n", __FUNCTION__, buf);
        char temp_buf[256];
        char *p = NULL;
        int i = 0;
        strncpy(temp_buf, buf, strlen(buf)+1);
        p = strtok(temp_buf, ",");
        while (NULL != p) {
           val[i++] = atoi(p);
           p = strtok(NULL,  ",");
        }
    } else {
        SYS_LOGE("%s:Invalid param!\n", __FUNCTION__);
    }
    return;
}

bool CPQControl::isFileExist(const char *file_name)
{
    struct stat tmp_st;
    int ret = -1;

    ret = stat(file_name, &tmp_st);
    if (ret != 0 ) {
       SYS_LOGE("%s don't exist!\n",file_name);
       return false;
    } else {
       return true;
    }
}

int CPQControl::Cpq_GetInputVideoFrameHeight(tv_source_input_t source_input)
{
    int inputFrameHeight = 0;

    if ((source_input == SOURCE_MPEG)
        || (source_input == SOURCE_DTV)) {//decoder
        char inputModeBuf[32] = {0};
        if (pqReadSys(VIDEO_FRAME_HEIGHT, inputModeBuf, sizeof(inputModeBuf)) > 0) {
            inputFrameHeight = atoi(inputModeBuf);
        } else {
            SYS_LOGE("Read VIDEO_FRAME_HEIGHT failed!\n");
        }
    } else {//vdin
#ifdef SUPPORT_TVSERVICE
        const sp<TvServerHidlClient> &TvService = getTvService();
        if (TvService == NULL) {
            SYS_LOGE("%s: get tvservice failed!\n", __FUNCTION__);
        } else {
            FormatInfo info = TvService->getHdmiFormatInfo();
            inputFrameHeight = info.height;
        }
#else
        SYS_LOGI("%s: don't support tvservice!\n", __FUNCTION__);
#endif
    }

    if (inputFrameHeight <= 0) {
        SYS_LOGE("%s: inputFrameHeight is invalid, return default value!\n", __FUNCTION__);
        inputFrameHeight = 1080;
    } else if (inputFrameHeight > 0 && inputFrameHeight < 480) {
        SYS_LOGD("%s: such as Youtube/etc 360/240/... video!\n", __FUNCTION__);
        inputFrameHeight = 480;
    }

    SYS_LOGI("%s: inputFrameHeight is %d!\n", __FUNCTION__, inputFrameHeight);
    return inputFrameHeight;
}

//table about db TVOUT_CVBS with input/output resolution
int Table_TvoutWithIOResolution[TABLE_TYPE_MAX][RESOLUTION_MAX][RESOLUTION_MAX] = {
                /*480                      576                       720                      1080                       2160                         4320*/
    { //SDR
        /*480*/ {OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_480_720, OUTPUT_TYPE_HDMI_480_1080, OUTPUT_TYPE_HDMI_480_2160,  OUTPUT_TYPE_HDMI_480_2160},
        /*576*/ {OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_576_720, OUTPUT_TYPE_HDMI_576_1080, OUTPUT_TYPE_HDMI_576_2160,  OUTPUT_TYPE_HDMI_576_2160},
        /*720*/ {OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_720_1080, OUTPUT_TYPE_HDMI_720_2160,  OUTPUT_TYPE_HDMI_720_2160},
        /*1080*/{OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE,  OUTPUT_TYPE_HDMI_1080_2160, OUTPUT_TYPE_HDMI_1080_2160},
        /*2160*/{OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE,  OUTPUT_TYPE_HDMI_4K,        OUTPUT_TYPE_HDMI_4K},
        /*4320*/{OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE,  OUTPUT_TYPE_HDMI_NOSCALE,   OUTPUT_TYPE_HDMI_NOSCALE}
    },
    { //HDR
        /*480*/ {OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_480_720_HDR, OUTPUT_TYPE_HDMI_480_1080_HDR, OUTPUT_TYPE_HDMI_480_2160_HDR,  OUTPUT_TYPE_HDMI_480_2160_HDR},
        /*576*/ {OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_576_720_HDR, OUTPUT_TYPE_HDMI_576_1080_HDR, OUTPUT_TYPE_HDMI_576_2160_HDR,  OUTPUT_TYPE_HDMI_576_2160_HDR},
        /*720*/ {OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_720_1080_HDR, OUTPUT_TYPE_HDMI_720_2160_HDR,  OUTPUT_TYPE_HDMI_720_2160_HDR},
        /*1080*/{OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR,  OUTPUT_TYPE_HDMI_1080_2160_HDR,OUTPUT_TYPE_HDMI_1080_2160_HDR},
        /*2160*/{OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR,  OUTPUT_TYPE_HDMI_4K_HDR,       OUTPUT_TYPE_HDMI_4K_HDR},
        /*4320*/{OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR, OUTPUT_TYPE_HDMI_NOSCALE_HDR,  OUTPUT_TYPE_HDMI_NOSCALE_HDR,  OUTPUT_TYPE_HDMI_NOSCALE_HDR}
    },
    { //4K 120HZ
        /*480*/ {OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_480_4K120,  OUTPUT_TYPE_HDMI_NOSCALE},
        /*576*/ {OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_576_4K120,  OUTPUT_TYPE_HDMI_NOSCALE},
        /*720*/ {OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_720_4K120,  OUTPUT_TYPE_HDMI_NOSCALE},
        /*1080*/{OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_1080_4K120, OUTPUT_TYPE_HDMI_NOSCALE},
        /*2160*/{OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_4K_4K120,   OUTPUT_TYPE_HDMI_NOSCALE},
        /*4320*/{OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE,    OUTPUT_TYPE_HDMI_NOSCALE}
    },
    { //4K 120HZ HDR
        /*480*/ {OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_480_4K120_HDR,  OUTPUT_TYPE_HDMI_NOSCALE},
        /*576*/ {OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_576_4K120_HDR,  OUTPUT_TYPE_HDMI_NOSCALE},
        /*720*/ {OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_720_4K120_HDR,  OUTPUT_TYPE_HDMI_NOSCALE},
        /*1080*/{OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_1080_4K120_HDR, OUTPUT_TYPE_HDMI_NOSCALE},
        /*2160*/{OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_4K_4K120_HDR,   OUTPUT_TYPE_HDMI_NOSCALE},
        /*4320*/{OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE, OUTPUT_TYPE_HDMI_NOSCALE,        OUTPUT_TYPE_HDMI_NOSCALE}
    }
};

//table about resolution height thread value
int Table_ResolutionHeightThread[RESOLUTION_MAX][2] = {
    //type            value
    {SD_HEIGHT_480,   480},
    {SD_HEIGHT_576,   576},
    {HD_HEIGHT_720,   720},
    {FHD_HEIGHT_1080, 1080},
    {UHD_HEIGHT_2160, 2160},
    {UHD_HEIGHT_4320, 4320}
};

output_type_t CPQControl::MapDbTvoutWithIOResolution(int inputFrameHeight, int outputFrameHeight)
{
    output_type_t OutPutType = OUTPUT_TYPE_LVDS;
    SYS_LOGD("%s inputFrameHeight %d outputFrameHeight %d\n", __FUNCTION__, inputFrameHeight, outputFrameHeight);

    if (mPQdb->mDbMatchType == MATCH_TYPE_MBOX_S5 ||
        mPQdb->mDbMatchType == MATCH_TYPE_MBOX_S7D ||
        mPQdb->mDbMatchType == MATCH_TYPE_MBOX_S6) {
        int index_in = 0, index_out = 0, table_type = 0;

        for (int i = 0; i < RESOLUTION_MAX; i++) { //pick up input index
            if (inputFrameHeight < Table_ResolutionHeightThread[i][1]) {
                index_in = i - 1;
                break;
            }
            if (inputFrameHeight >= Table_ResolutionHeightThread[UHD_HEIGHT_4320][1]) { //input 8k
                index_in = 5;
            }
        }

        for (int j = 0; j < RESOLUTION_MAX; j++) { //pick up output index
            if (outputFrameHeight < Table_ResolutionHeightThread[j][1]) {
                index_out = j - 1;
                break;
            }
            if (outputFrameHeight >= Table_ResolutionHeightThread[UHD_HEIGHT_4320][1]) { //8k output
                index_out = 5;
            }
        }

        if (mPQdb->mHdrStatus == true) {
            if (mDisplayMode4k120 == true || mDisplayMode4k100 ==  true) {
                table_type = 3;
            } else {
                table_type = 1;
            }
        } else {
            if (mDisplayMode4k120 == true || mDisplayMode4k100 == true) {
                table_type = 2;
            } else {
                table_type = 0;
            }
        }

        mOutPutFrameHeightType = (resolution_height_type_t)index_out;

        SYS_LOGD("%s table_type %d index_in %d index_out %d mOutPutFrameHeightType %d\n", __FUNCTION__, table_type, index_in, index_out, mOutPutFrameHeightType);
        OutPutType = (output_type_t)Table_TvoutWithIOResolution[table_type][index_in][index_out];
    } else { //old project logic
        if (inputFrameHeight > 1088) {//inputsource is 4k
            OutPutType = OUTPUT_TYPE_HDMI_4K;
        } else {
            if (inputFrameHeight >= outputFrameHeight) {//input height >= output height
                OutPutType = OUTPUT_TYPE_HDMI_NOSCALE;
            } else {//input height < output height
                if (inputFrameHeight > 720 && inputFrameHeight <= 1088) {//inputsource is 1080
                    OutPutType = OUTPUT_TYPE_HDMI_NOSCALE;
                } else if (inputFrameHeight > 576 && inputFrameHeight <= 720) {//inputsource is 720
                    if (outputFrameHeight == 4096) {
                        OutPutType = OUTPUT_TYPE_HDMI_HD_4096;
                    } else if (outputFrameHeight >= inputFrameHeight * 2) {
                        OutPutType = OUTPUT_TYPE_HDMI_HD_UPSCALE;
                    } else {
                        OutPutType = OUTPUT_TYPE_HDMI_NOSCALE;
                    }
                } else {//inputsource is 480
                    if (outputFrameHeight == 4096) {
                        OutPutType = OUTPUT_TYPE_HDMI_SD_4096;
                    } else if ((outputFrameHeight * 8) >= (inputFrameHeight * 15)) {
                        OutPutType = OUTPUT_TYPE_HDMI_SD_UPSCALE;
                    } else {
                        OutPutType = OUTPUT_TYPE_HDMI_NOSCALE;
                    }
                }
            }
        }
    }

    SYS_LOGD("%s OutPutType %d\n", __FUNCTION__, OutPutType);
    return OutPutType;
}

output_type_t CPQControl::CheckOutPutMode(tv_source_input_t source_input)
{
    output_type_t OutPutType = OUTPUT_TYPE_LVDS;
    if (!isFileExist(HDMI_OUTPUT_CHECK_PATH)) {//LVDS output
        OutPutType = OUTPUT_TYPE_LVDS;
    } else {
        int outputFrameHeight = 1080;
        char outputModeBuf[32] = {0};
        if ((pqReadSys(DISPLAY_MODE, outputModeBuf, sizeof(outputModeBuf)) < 0) || (strlen(outputModeBuf) == 0)) {
            SYS_LOGD("Read DISPLAY_MODE failed!\n");
        } else {
            SYS_LOGD( "%s: current output mode is %s!\n", __FUNCTION__, outputModeBuf);
            if (strstr(outputModeBuf, "null")) {
                return OUTPUT_TYPE_MAX;
            } else if (strstr(outputModeBuf, "480cvbs")) {//NTSC output
                OutPutType = OUTPUT_TYPE_NTSC;
            } else if(strstr(outputModeBuf, "576cvbs")) {//PAL output
                OutPutType = OUTPUT_TYPE_PAL;
            } else {//HDMI output
                char tempBuf[32] = {0};
                int outputModeStrSize = strlen(outputModeBuf);
                strncpy(tempBuf, outputModeBuf, (outputModeStrSize-4));//delete "xxhz"
                SYS_LOGD( "%s: size is %d, str is : %s!\n", __FUNCTION__, outputModeStrSize, tempBuf);
                if (strstr(tempBuf, "smpte")) {
                    outputFrameHeight = 4096;
                } else {
                    memset(tempBuf,0, sizeof(tempBuf));
                    strncpy(tempBuf, outputModeBuf, (outputModeStrSize - 5));//delete "pxxhz"
                    outputFrameHeight = atoi(tempBuf);
                }
                SYS_LOGD("%s: outputFrameHeight: %d!\n", __FUNCTION__, outputFrameHeight);

                if (strstr(outputModeBuf, "120hz")) {
                    mDisplayMode4k120 = true;
                } else if (strstr(outputModeBuf, "100hz")) {
                    mDisplayMode4k100 = true;
                } else {
                    mDisplayMode4k120 = false;
                    mDisplayMode4k100= false;
                }
                SYS_LOGD("%s: mDisplayMode4k120:%d mDisplayMode4k100:%d!\n", __FUNCTION__, mDisplayMode4k120, mDisplayMode4k100);

                //check outputmode
                if ((source_input == SOURCE_MPEG)
                    || (source_input == SOURCE_DTV)
                    || (source_input == SOURCE_HDMI1)
                    || (source_input == SOURCE_HDMI2)
                    || (source_input == SOURCE_HDMI3)
                    || (source_input == SOURCE_HDMI4)) {//hdmi/dtv/mpeg input
                    int inputFrameHeight = Cpq_GetInputVideoFrameHeight(source_input);
                    OutPutType = MapDbTvoutWithIOResolution(inputFrameHeight, outputFrameHeight);
                } else {//atv/av input
                    if (outputFrameHeight >= 720) {
                        OutPutType = OUTPUT_TYPE_HDMI_SD_UPSCALE;
                    } else {
                        OutPutType = OUTPUT_TYPE_HDMI_NOSCALE;
                    }
                }
            }
        }
    }

    SYS_LOGD("%s: output mode is %d!\n", __FUNCTION__, OutPutType);
    mPQdb->mOutPutType = OutPutType;
    return OutPutType;
}

bool CPQControl::isCVBSParamValid(void)
{
    bool ret = mPQdb->CheckCVBSParamValidStatus();
    if (ret) {
        SYS_LOGI("cvbs param exist!\n");
    } else {
        SYS_LOGI("cvbs param don't exist!\n");
    }
    return ret;
}

bool CPQControl::isPqDatabaseMachChip()
{
    bool matchStatus = false;
    meson_cpu_ver_e chipVersion = MESON_CPU_VERSION_NULL;
    database_attribute_t dbAttribute;
    mPQdb->PQ_GetDataBaseAttribute(&dbAttribute);
    if ((dbAttribute.ChipVersion.c_str() == NULL) || (dbAttribute.ChipVersion.length() == 0)) {
        SYS_LOGI("%s: ChipVersion is null!\n", __FUNCTION__);
        chipVersion = MESON_CPU_VERSION_NULL;
    } else {
        std::string TempStr = std::string(dbAttribute.ChipVersion.c_str());
        int flagPosition = TempStr.find("_");
        std::string versionStr = TempStr.substr(flagPosition+1, 1);
        SYS_LOGI("%s: versionStr is %s!\n", __FUNCTION__, versionStr.c_str());
        if (versionStr == "A") {
            chipVersion = MESON_CPU_VERSION_A;
        } else if (versionStr ==  "B") {
            chipVersion = MESON_CPU_VERSION_B;
        } else if (versionStr == "C") {
            chipVersion = MESON_CPU_VERSION_C;
        } else {
            chipVersion = MESON_CPU_VERSION_NULL;
        }
    }

    if (chipVersion == MESON_CPU_VERSION_NULL) {
        SYS_LOGI("%s: database don't have chipversion!\n", __FUNCTION__);
        matchStatus = true;
    } else {
        int ret = VPPDeviceIOCtl(AMVECM_IOC_S_MESON_CPU_VER, &chipVersion);
        if (ret < 0) {
            SYS_LOGE("%s: database don't match chip!\n", __FUNCTION__);
            matchStatus = false;
        } else {
            SYS_LOGI("%s: database is match chip!\n", __FUNCTION__);
            matchStatus = true;
        }
    }

    return matchStatus;
}

int CPQControl::Cpq_SetVadjEnableStatus(int isvadj1Enable, int isvadj2Enable)
{
    SYS_LOGD("%s: isvadj1Enable = %d, isvadj2Enable = %d.\n", __FUNCTION__, isvadj1Enable, isvadj2Enable);
    int ret = -1;
    if ((!mbCpqCfg_amvecm_basic_enable) && (!mbCpqCfg_amvecm_basic_withOSD_enable)) {
        ret = 0;
        SYS_LOGD("%s: all vadj module disabled.\n", __FUNCTION__);
    } else {
        am_pic_mode_t params;
        memset(&params, 0, sizeof(params));
        params.flag |= (0x1 << 6);
        params.vadj1_en = isvadj1Enable;
        params.vadj2_en = isvadj2Enable;
        ret = VPPDeviceIOCtl(AMVECM_IOC_S_PIC_MODE, &params);
        if (ret < 0) {
            SYS_LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
        }
    }

    return ret;
}

bool CPQControl::isBootvideoStopped()
{
    int readLength = 0;
    char* end = NULL;
    char buf[PROPERTY_VALUE_MAX] = {0};
    bool ret = true;

    readLength = property_get(BOOTVIDEO_ENABLE_PROP, buf, "3050");
    SYS_LOGD("%s: bootvideo enable value is %s!\n", __FUNCTION__, buf);
    if (readLength > 0) {
        int bootVideoEnable = (strtol(buf, &end, 0)) / 1000;
        if (bootVideoEnable == 3) {
            memset(buf, 0, sizeof(buf));
            readLength = property_get(BOOTVIDEO_EXIT_PROP, buf, "0");
            if (readLength > 0) {
                if (strcmp(buf, "1") == 0) {
                    ret = false;
                } else {
                    ret = true;
                }
            } else {
                SYS_LOGE("%s: getprop %s error!\n", __FUNCTION__, BOOTVIDEO_EXIT_PROP);
                ret = true;
            }
        } else {
            SYS_LOGD("%s: bootvideo don't enable!\n", __FUNCTION__);
            ret = true;
        }
    } else {
        SYS_LOGE("%s: getprop %s error!\n", __FUNCTION__, BOOTVIDEO_ENABLE_PROP);
        ret = true;
    }

    return ret;
}

int CPQControl::Set_PictureMode(PICTURE_MODE pq_mode, pq_mode_switch_type_t switch_type)
{
    int ret = 0;
    SetPcGameMode((vpp_picture_mode_t)pq_mode, switch_type);

    ret = SetPQPictureMode(pq_mode);

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::SetPQPictureMode(PICTURE_MODE pq_mode)
{
    int ret = 0;

    SetFacColorParams(mCurrentSourceInputInfo);

    PICTURE_MODE_DATA PictureMode;
    if (!GetPictureModeData(&PictureMode, pq_mode)) {
        SYS_LOGE("%s GetPictureModeData failed!\n",__FUNCTION__);
        return -1;
    }

    SYS_LOGI("%s:\n"
        "PictureMode:      %3d.\n"
        "Brightness:       %3d, Contrast:         %3d, Saturation:       %3d, Hue:             %3d,\n"
        "Sharpness:        %3d, Backlight:        %3d, Nr:               %3d, DynamicContrast: %3d,\n"
        "DynamicBacklight  %3d, ColorGamut:       %3d, ColorTemperature: %3d, LocalContrast:   %3d,\n"
        "BlackStretch:     %3d, BlueStretch:      %3d, MpegNr:           %3d, ChromaCoring:    %3d,\n"
        "Memc              %3d, SmoothPlus:       %3d, SuperResolution   %3d, Gamma            %3d,\n"
        "hdr_tone_mapping: %3d, DvMode:           %3d, DvDarkDetail:     %3d, DvLightSensor:   %3d,\n"
        "AmDolbyPrcision:  %3d, Deblock:          %3d, DeMoSquito:       %3d\n",
        __FUNCTION__,
        (int)PictureMode.mode,
        PictureMode.Brightness, PictureMode.Contrast, PictureMode.Saturation, PictureMode.Hue,
        PictureMode.Sharpness, PictureMode.Backlight, PictureMode.Nr, PictureMode.DynamicContrast,
        PictureMode.DynamicBacklight, PictureMode.ColorGamut, PictureMode.ColorTemperature, PictureMode.LocalContrast,
        PictureMode.BlackStretch, PictureMode.BlueStretch, PictureMode.MpegNr, PictureMode.ChromaCoring,
        PictureMode.Memc, PictureMode.Decontour, PictureMode.SuperResolution, PictureMode.GammaMidLuminance,
        PictureMode.HdrTmo, PictureMode.DvMode, PictureMode.DvDarkDetail, PictureMode.DvLightSensor,
        PictureMode.AmDolbyPrcision, PictureMode.Deblock, PictureMode.DeMoSquito);

    if (ret == 0) {
        ret |= Cpq_SetBrightness(PictureMode.Brightness, mCurrentSourceInputInfo);
        ret |= Cpq_SetContrast(PictureMode.Contrast, mCurrentSourceInputInfo);
        ret |= Cpq_SetSaturation(PictureMode.Saturation, mCurrentSourceInputInfo);
        ret |= Cpq_SetHue(PictureMode.Hue, mCurrentSourceInputInfo);
        ret |= Cpq_SetSharpness(PictureMode.Sharpness, mCurrentSourceInputInfo);
        ret |= Cpq_SetNoiseReductionMode((vpp_noise_reduction_mode_t)PictureMode.Nr, mCurrentSourceInputInfo);
        ret |= Cpq_SetDnlpMode((Dynamic_contrast_status_t)PictureMode.DynamicContrast, mCurrentSourceInputInfo);
        ret |= Cpq_SetLocalContrastMode((local_contrast_mode_t)PictureMode.LocalContrast);
        ret |= Cpq_SetColorGamutMode((vpp_colorgamut_mode_t)PictureMode.ColorGamut, mCurrentSourceInputInfo);
        ret |= Cpq_SetBlackStretch(PictureMode.BlackStretch, mCurrentSourceInputInfo);
        ret |= Cpq_SetBlueStretch(PictureMode.BlueStretch, mCurrentSourceInputInfo);
        ret |= Cpq_SetChromaCoring(PictureMode.ChromaCoring, mCurrentSourceInputInfo);
        ret |= Cpq_SetDeblockMode((di_deblock_mode_t)PictureMode.Deblock, mCurrentSourceInputInfo);
        ret |= Cpq_SetDemoSquitoMode((di_demosquito_mode_t)PictureMode.DeMoSquito, mCurrentSourceInputInfo);
        ret |= Cpq_SetSmoothPlusMode((vpp_smooth_plus_mode_t)PictureMode.Decontour, mCurrentSourceInputInfo);
    ret |= Cpq_SetMemcMode((MEMC_MODE)PictureMode.Memc, mCurrentSourceInputInfo);
        ret |= Cpq_SetSuperResolution(PictureMode.SuperResolution, mCurrentSourceInputInfo);

        // colortemp
        ret |= Cpq_SetColorTemperature(PictureMode.ColorTemperature);
        ret |= Cpq_LoadGamma((vpp_gamma_mode_t)PictureMode.GammaMidLuminance, (vpp_color_temperature_mode_t)PictureMode.ColorTemperature);

        // for hdr
        ret |= Cpq_SetHDRTMOMode(PictureMode.HdrTmo);

        // for amdolby
        ret |= Cpq_SetAmDolbyPQMode(PictureMode.DvMode);
        ret |= Cpq_SetDolbyDarkDetail(PictureMode.DvDarkDetail);
        ret |= Cpq_SetAMDolbyLightSensor(PictureMode.DvLightSensor);
        ret |= Cpq_SetAmDolbyPecisionDetail(PictureMode.AmDolbyPrcision);
    }

    if (ret < 0) {
        SYS_LOGE("%s Some modules failed to set in Picture Mode, please check!\n",__FUNCTION__);
    }

    SYS_LOGD("%s Done!\n",__FUNCTION__);
    return ret;
}

void CPQControl::SetPcGameMode(vpp_picture_mode_t pq_mode, pq_mode_switch_type_t switch_type)
{
    if (switch_type >= PQ_MODE_SWITCH_TYPE_MAX) {
        return;
    }

    if ((CurSource >= PQ_SRC_HDMI1) && (CurSource <= PQ_SRC_HDMI4)) {//HDMI source;
        if (mLastPictureMode == VPP_PICTURE_MODE_GAME) {
            if (pq_mode == VPP_PICTURE_MODE_GAME) {
                setPQModeByTvService(MODE_ON, MODE_OFF, switch_type);//game mode on and monitor mode off;
            } else if (pq_mode == VPP_PICTURE_MODE_MONITOR) {
                setPQModeByTvService(MODE_OFF, MODE_ON, switch_type);//game mode off and monitor mode on;
            } else {
                setPQModeByTvService(MODE_OFF, MODE_OFF, switch_type);//game mode off and monitor mode off;
            }
        } else if (mLastPictureMode == VPP_PICTURE_MODE_MONITOR) {
            if (pq_mode == VPP_PICTURE_MODE_MONITOR) {
                setPQModeByTvService(MODE_OFF, MODE_ON, switch_type);//game mode off and monitor mode on;
            } else if (pq_mode == VPP_PICTURE_MODE_GAME) {
                setPQModeByTvService(MODE_ON, MODE_OFF, switch_type);//game mode on and monitor mode off;
            } else {
                setPQModeByTvService(MODE_OFF, MODE_OFF, switch_type);//game mode off and monitor mode off;
            }
        } else {
            if (pq_mode == VPP_PICTURE_MODE_GAME) {
                setPQModeByTvService(MODE_ON, MODE_OFF, switch_type);//game mode on and monitor mode off;
            } else if (pq_mode == VPP_PICTURE_MODE_MONITOR) {
                setPQModeByTvService(MODE_OFF, MODE_ON, switch_type);//game mode off and monitor mode on;
            } else {
                setPQModeByTvService(MODE_OFF, MODE_OFF, PQ_MODE_SWITCH_TYPE_INIT);//game mode off and monitor mode off;
            }
        }
    } else {//other source;
        if (mInitialized) {
            setPQModeByTvService(MODE_OFF, MODE_OFF, PQ_MODE_SWITCH_TYPE_INIT);
        }
    }

    return;
}

pq_source_input_t CPQControl::CheckPQSource(tv_source_input_t source)
{
    pq_source_input_t src = PQ_SRC_DEFAULT;
    switch (source) {
    case SOURCE_TV:
        src = PQ_SRC_TV;
        break;
    case SOURCE_AV1:
        src = PQ_SRC_AV1;
        break;
    case SOURCE_AV2:
        src = PQ_SRC_AV2;
        break;
    case SOURCE_YPBPR1:
        src = PQ_SRC_YPBPR1;
        break;
    case SOURCE_YPBPR2:
        src = PQ_SRC_YPBPR2;
        break;
    case SOURCE_HDMI1:
        src = PQ_SRC_HDMI1;
        break;
    case SOURCE_HDMI2:
        src = PQ_SRC_HDMI2;
        break;
    case SOURCE_HDMI3:
        src = PQ_SRC_HDMI3;
        break;
    case SOURCE_HDMI4:
        src = PQ_SRC_HDMI4;
        break;
    case SOURCE_VGA:
        src = PQ_SRC_VGA;
        break;
    case SOURCE_MPEG:
        src = PQ_SRC_MPEG;
        break;
    case SOURCE_DTV:
        src = PQ_SRC_DTV;
        break;
    case SOURCE_SVIDEO:
        src = PQ_SRC_SVIDEO;
        break;
    case SOURCE_IPTV:
        src = PQ_SRC_IPTV;
        break;
    case SOURCE_DUMMY:
        src = PQ_SRC_DUMMY;
        break;
    case SOURCE_SPDIF:
        src = PQ_SRC_SPDIF;
        break;
    case SOURCE_ADTV:
        src = PQ_SRC_ADTV;
        break;
    default:
        src = PQ_SRC_DEFAULT;
        break;
    }

    return src;
}

pq_sig_fmt_t CPQControl::CheckPQTimming(hdr_type_t hdr_type)
{
    pq_sig_fmt_t timming = PQ_SIGFMT_DEFAULT;
    switch (hdr_type) {
        case HDR_TYPE_HDR10:
            timming = PQ_SIGFMT_HDR;
            break;
        case HDR_TYPE_HDR10PLUS:
            timming = PQ_SIGFMT_HDRP;
            break;
        case HDR_TYPE_DOVI:
            timming = PQ_SIGFMT_DV;
            break;
        case HDR_TYPE_HLG:
            timming = PQ_SIGFMT_HLG;
            break;
        case HDR_TYPE_SDR:
            timming = PQ_SIGFMT_SDR;
            break;
        case HDR_TYPE_NONE:
        case HDR_TYPE_PRIMESL:
        case HDR_TYPE_MVC:
        default:
            timming = PQ_SIGFMT_DEFAULT;
            break;
    }

    return timming;
}

void CPQControl::InitTconGamma(void)
{
    int ret = -1;
    gm_tbl_t tconGmTbl;
    memset(&tconGmTbl, 0, sizeof(gm_tbl_t));

    for (int i = 0; i < 10; i++) {
        ret  = mPQdb->PQ_GetTconGammaTable(i, &tconGmTbl);

        if (ret < 0) {
            SYS_LOGE("%s, PQ_GetTconGammaTable %d file...\n", __FUNCTION__, i);
            return;
        }
    }

    ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_SET, &tconGmTbl);

    if (ret < 0) {
        SYS_LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
    }

    return;
}

void CPQControl::InitPGammaBin()
{
    char propbuf[PROPERTY_VALUE_MAX] = {0};
    bool autogen = false;

    if (property_get(PROP_PGAMMA_AUTO_GEN, propbuf, "0") > 0) {
        SYS_LOGE("Prop [%s]=%s\n", PROP_PGAMMA_AUTO_GEN, propbuf);
        if (!strcasecmp(propbuf, "true") || !strcmp(propbuf, "1"))
            autogen = true;
    } else {
        SYS_LOGE("getprop [%s] fail\n", PROP_PGAMMA_AUTO_GEN);
    }

    if (autogen) {
        CTconPGamma *pgammaDev = CTconPGamma::GetInstance();
        if (pgammaDev && !pgammaDev->Init(NULL)) {
            pgammaDev->PrintInfo(-1);
            if (pgammaDev->GenerateBin((char *)"default") < 0)
                SYS_LOGE("Gen pgamma bin failed, exit...\n");
            else
                SYS_LOGD("Gen pgamma bin Ok\n");
        }

        if (pgammaDev)
        pgammaDev->UnInit();
    }
}

void CPQControl::InitTconlessBin(void)
{
    int ret = -1;
    unsigned int i;
    unsigned int max_cnt = 0;

    if (AML_HAL_LCD_GetTconBinMaxCnt(&max_cnt) != API_OK) {
        SYS_LOGE("%s AML_HAL_LCD_GetTconBinMaxCnt FAIL max_cnt = %d \n", __FUNCTION__, max_cnt);
        return;
    }

    for (i = 0; i < max_cnt; i++) {
        if (LoadTconlessBin(i) < 0) {
            SYS_LOGE("%s table %d is not exist  \n", __FUNCTION__, i);
        } else {
            SYS_LOGD("%s LOAD tcon bin index: %d  \n", __FUNCTION__, i);
        }
    }

    // handle pgamma bin
    InitPGammaBin();
}

int CPQControl::LoadTconlessBin(unsigned int index)
{
    int ret = 0;
    int fd = -1;
    int sizeHeader = 0;
    int sizeData = 0;
    unsigned char * dataBuff = NULL;
    unsigned char buff[MAX_TABLE_SIZE/sizeof(unsigned char)] = {0};
    int count_retry = 20;

    am_pq_bin_param_t param;
    aml_path_t path;
    memset(&param, 0, sizeof(am_pq_bin_param_t));
    memset(&path, 0, sizeof(aml_path_t));

    //set index to driver
    if (AML_HAL_LCD_SetTconDataIndex(index) != API_OK) {
        SYS_LOGE("%s AML_HAL_LCD_SetTconDataIndex fail \n", __FUNCTION__);
        return -1;
    }

    // get bin path
    if (AML_HAL_LCD_GetTconBinPath((HAL_aml_path_s *)&path) != API_OK) {
        SYS_LOGE("%s AML_HAL_LCD_GetTconBinPath fail \n", __FUNCTION__);
        return -1;
    } else {
        SYS_LOGD("%s INDEX:{ %d } ====> GetBinPath = %s!\n",__FUNCTION__, index, path.string);
    }

    if (strstr(path.string, "demura")) {
        char propbuf[PROPERTY_VALUE_MAX] = {0};
        bool autogen = false;
        if (property_get(PROP_DEMURA_AUTO_GEN, propbuf, "0") > 0) {
            SYS_LOGD("Prop [%s]=%s\n", PROP_DEMURA_AUTO_GEN, propbuf);
            if (!strcasecmp(propbuf, "true") || !strcmp(propbuf, "1"))
                autogen = true;
        } else {
            SYS_LOGE("getprop [%s] fail\n", PROP_DEMURA_AUTO_GEN);
        }
        if (autogen) {
            SYS_LOGD("Detect demura binary, try to gen %s...\n", path.string);
            CTconDemura *demuraDev = CTconDemura::GetInstance();
            if (demuraDev && !demuraDev->Init(NULL)) {
                demuraDev->PrintInfo(-1);
                if (demuraDev->GenerateBin(path.string) < 0)
                    SYS_LOGE("Gen %s failed, exit...\n", path.string);
                else
                    SYS_LOGD("Gen %s Ok\n", path.string);
            }
            if (demuraDev)
                demuraDev->UnInit();
        }
    }

    // read bin data
    if (isFileExist(path.string) == false) {
        return -1;
    }
    if ((fd = open(path.string, O_RDONLY)) < 0) {
        SYS_LOGE("Open %s error(%s)!\n", path.string, strerror(errno));
        return -1;
    }
    sizeData = read(fd, buff, sizeof(buff));
    if (sizeData <= 0) {
        SYS_LOGE("%s ERROR !!!data size[%d]\n", __FUNCTION__,sizeData);
        ret = -1;
        goto exit;
    } else {
        dataBuff = (unsigned char *)malloc(sizeData);
        if (dataBuff != NULL) {
            memset(dataBuff, 0x0, sizeData);
        } else {
            SYS_LOGE("%s malloc memory fail \n", __FUNCTION__);
            ret = -1;
            goto exit;
        }
        memcpy((void *)dataBuff, buff, sizeData);
        param.table_index= index;
        param.table_len= sizeData;
        param.table_ptr = (long long) dataBuff;
        ret = 0;
    }

    //set bin data to driver
    while (count_retry) {
        if (AML_HAL_LCD_SetTconBinData((HAL_am_pq_bin_param_s *)&param) !=  API_OK) {
            SYS_LOGE("%s, error(%s), errno(%d)\n", __FUNCTION__, strerror(errno), errno);
            if (errno == EBUSY) {
                SYS_LOGE("%s, %s, retry...\n", __FUNCTION__, strerror(errno));
                count_retry--;
                continue;
            }
        }
        break;
    }

    close(fd);
    free(dataBuff);
    return ret;

    exit:
        close(fd);
        return ret;
}

// for dv IQ APO
AMDOLBY_IQ_APO_STRUCT amdolby_apo[AMDOLBY_APOO_TYPE_MAX] = {
/*Module                 sharp sr    memc  nr*/
/*Type 0          */     {0,   _OFF, _OFF, _MID},
/*Type 1          */     {0,   _OFF, _OFF, _OFF},
/*Type 2          */     {0,   _OFF, _OFF, _OFF},
/*Type 3          */     {25,  _LOW, _HIGH,_LOW},
/*Type 4          */     {50,  _MID, _OFF, _MID},
};
int CPQControl::RefreshDvApoPictureMode(int Type)
{
    if (mCurrentHdrType != HDR_TYPE_DOVI) {
        return 0;
    }

    if (Type < AMDV_APOO_TYPE_0 || Type > AMDV_APOO_TYPE_4) {
        SYS_LOGE("%s:Type %d out of range\n", __FUNCTION__, Type);
        return -1;
    }

    Cpq_SetMemcMode((MEMC_MODE)amdolby_apo[Type].Memc, mCurrentSourceInputInfo);
    Cpq_SetSharpness(amdolby_apo[Type].Sharp, mCurrentSourceInputInfo);
    Cpq_SetSuperResolution(amdolby_apo[Type].Sr, mCurrentSourceInputInfo);
    Cpq_SetNoiseReductionMode((vpp_noise_reduction_mode_t)amdolby_apo[Type].Nr, mCurrentSourceInputInfo);

    SYS_LOGD("%s:Refresh Type: %d. MEMC  -> %d\n", __FUNCTION__, Type, amdolby_apo[Type].Memc);
    SYS_LOGD("%s:Refresh Type: %d. Sharp -> %d\n", __FUNCTION__, Type, amdolby_apo[Type].Sharp);
    SYS_LOGD("%s:Refresh Type: %d. Sr    -> %d\n", __FUNCTION__, Type, amdolby_apo[Type].Sr);
    SYS_LOGD("%s:Refresh Type: %d. Nr    -> %d\n", __FUNCTION__, Type, amdolby_apo[Type].Nr);
    return 0;
}

int CPQControl::SetDolbyDarkDetail(int mode, int is_save)
{
    int ret =0;
    SYS_LOGD("%s, mode = %d\n", __FUNCTION__, mode);
    ret = Cpq_SetDolbyDarkDetail(mode);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveDolbyDarkDetail(mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }
    return 0;
}

int CPQControl::GetDolbyDarkDetail(void)
{
    int mode = -1;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d level: %d fail\n",__FUNCTION__, CurSource, CurTimming, mode);
        return mode;
    }

    mode = para.DvDarkDetail;

    SYS_LOGD("%s, source: %d, timming: %d, mode = %d\n", __FUNCTION__, CurSource, CurTimming, mode);
    return mode;
}

int CPQControl::SaveDolbyDarkDetail(int value)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: GetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    para.DvDarkDetail = value;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s: SetPictureModeData source: %d, timming: %d value: %d fail\n",__FUNCTION__, CurSource, CurTimming, value);
        return -1;
    }

    SYS_LOGI("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::Cpq_SetDolbyDarkDetail(int mode)
{
    if (mode < 0) {
        SYS_LOGD("%s skip DarkDetail!\n",__FUNCTION__);
        return 0;
    }

    int ret = -1;
    ret = mDolbyVision->SetDolbyPQDarkDetail(mode);

    if (ret < 0)
        SYS_LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetAmDolbyPQMode(int mode)
{
    if (mode < 0) {
        SYS_LOGD("%s skip AmDolbyPQMode!\n",__FUNCTION__);
        return 0;
    }

    int ret = -1;
    ret = mDolbyVision->SetDolbyPQMode((dolby_pq_mode_t)mode);

    if (ret < 0)
        SYS_LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::SetAMDolbyLightSensor(int mode, int is_save)
{
    int ret =0;
    SYS_LOGD("%s, mode = %d\n", __FUNCTION__, mode);
    ret = Cpq_SetAMDolbyLightSensor(mode);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveAMDolbyLightSensor(mode);
    }

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetAMDolbyLightSensor(void)
{
    int mode = -1;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s GetPictureModeData failed!\n",__FUNCTION__);
        return -1;
    }

    mode = para.DvLightSensor;

    SYS_LOGD("%s, source: %d, timming: %d, mode = %d\n", __FUNCTION__, CurSource, CurTimming, mode);
    return mode;
}

int CPQControl::SaveAMDolbyLightSensor(int value)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s GetPictureModeData failed!\n",__FUNCTION__);
        return -1;
    }

    para.DvLightSensor = value;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetAMDolbyLightSensor(int mode)
{
    if (mode < 0) {
        SYS_LOGD("%s skip LightSensor!\n",__FUNCTION__);
        return 0;
    }

    light_sensor_s data;
    data.flag = mode;
    data.t_frontLux = 0;

    int ret = -1;
    ret = mDolbyVision->SetDolbyPQLightSensor(&data);

    if (ret < 0)
        SYS_LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::SetAmDolbyPecisionDetail(int mode, int is_save)
{
    SYS_LOGD("%s, mode = %d\n", __FUNCTION__, mode);

    if (is_save)
        SaveAmDolbyPecisionDetail(mode);

    int ret = Cpq_SetAmDolbyPecisionDetail(mode);

    if (ret < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
    } else {
        SYS_LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetAmDolbyPecisionDetail(void)
{
    int mode = _NULL;
    if (mDolbyVision->GetAmDolbyPecisionDetailSupport() == 0) {
        SYS_LOGE("%s not support Pecision Detail!\n",__FUNCTION__);
        return mode ;
    }

    PICTURE_MODE_DATA para;
    if (!GetPictureModeData(&para, (PICTURE_MODE)GetPQMode())) {
        SYS_LOGE("%s GetPictureModeData failed!\n",__FUNCTION__);
        return mode;
    }

    mode = para.AmDolbyPrcision;

    if (mode < _NULL || mode > _ON) {
        mode = _NULL;
    }

    SYS_LOGD("%s, source: %d, timming: %d, mode = %d\n", __FUNCTION__, CurSource, CurTimming, mode);
    return mode;
}

int CPQControl::SaveAmDolbyPecisionDetail(int mode)
{
    PICTURE_MODE_DATA para;
    PICTURE_MODE pq_mode = (PICTURE_MODE)GetPQMode();
    if (!GetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s GetPictureModeData failed!\n",__FUNCTION__);
        return -1;
    }

    para.AmDolbyPrcision = mode;

    if (!SetPictureModeData(&para, pq_mode)) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetAmDolbyPecisionDetail(int mode)
{
    if (mode < 0) {
        SYS_LOGD("%s skip Pecision Detail!\n",__FUNCTION__);
        return 0;
    }

    if (mDolbyVision->SetAmDolbyPecisionDetail(mode) < 0) {
        SYS_LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::SetFilmMakerMode(int onoff)
{
    SYS_LOGI("%s, onoff: %d\n", __FUNCTION__, onoff);
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    pData.FilmMakerEnable = onoff;

    if (!SetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s SetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    Cpq_SetFilmMakerMode(onoff);

    return 0;
}

int CPQControl::GetFilmMakerMode(void)
{
    int enable  = 0;
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return enable;
    }

    enable = pData.FilmMakerEnable;

    SYS_LOGI("%s, status: %d\n", __FUNCTION__, enable);
    return enable;
}

int CPQControl::Cpq_SetFilmMakerMode(int onoff)
{
    SYS_LOGI("%s, onoff: %d\n", __FUNCTION__, onoff);
    if (!mbFilmmakerModeFlag) {
        SYS_LOGI("%s, mbFilmmakerModeFlag is disable\n", __FUNCTION__);
        return 0;
    }

    if (onoff == 1) {
        SetPQPictureMode(PICTURE_MODE_FILMMAKER);
    } else {
        SetPQPictureMode(mLastPictureMode);
    }

    return 0;
}

int CPQControl::SetFilmMakerFlag(int enable)
{
    if (GetFilmMakerMode() == 0) {
        SYS_LOGI("%s, FilmMaker is disable: %d\n", __FUNCTION__, enable);
        return 0;
    }

    SYS_LOGI("%s, enable: %d\n", __FUNCTION__, enable);
    if (enable == 1) {
        if (mbFilmmakerModeFlag == false) {
            mbFilmmakerModeFlag = true;
            SetPQPictureMode(PICTURE_MODE_FILMMAKER);
        } else {
            SYS_LOGI("%s, same status: %d\n", __FUNCTION__, enable);
        }
    } else {
        if (mbFilmmakerModeFlag == true) {
            mbFilmmakerModeFlag = false;
            SetPQPictureMode(mLastPictureMode);
        } else {
            SYS_LOGI("%s, same status: %d\n", __FUNCTION__, enable);
        }
    }

    return 0;
}

int CPQControl::SetSDR2HDR(int onoff)
{
    SYS_LOGD("%s, onoff = %d\n", __FUNCTION__, onoff);

    SaveSDR2HDR(onoff);

    if (Cpq_SetSDR2HDR(onoff) < 0) {
        SYS_LOGD("%s fail!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::GetSDR2HDR(void)
{
    int onoff = 0;
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return onoff;
    }

    onoff = pData.Sdr2Hdr;

    if (onoff < 0 || onoff > 1) {
        onoff = 0;
    }

    SYS_LOGD("%s, mode = %d\n", __FUNCTION__, onoff);
    return onoff;
}

int CPQControl::SaveSDR2HDR(int onoff)
{
    PICTURE_SETTING_GLOBAL pData;
    if (!GetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s GetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    pData.Sdr2Hdr = onoff;

    if (!SetPictureStructDataGlobal(&pData)) {
        SYS_LOGE("%s SetPictureStructDataGlobal failed!\n",__FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetSDR2HDR(int onoff)
{
    if (CurTimming != PQ_SIGFMT_SDR)
        onoff = 0;

    if (VPPDeviceIOCtl(AMVECM_IOC_S_SDR2HDR_CTRL, &onoff) < 0) {
        SYS_LOGE("%s AMVECM_IOC_S_SDR2HDR_CTRL failed!\n",__FUNCTION__);
        return -1;
    }

    return 0;
}

#ifdef DIFFERENTIAL_COMPRESS_PQ_DB
char* CPQControl::CalculateFileSha1(const char* filePath)
{
    SHA_CTX c;
    unsigned char md[SHA_DIGEST_LENGTH];
    int fd;
    int size;
    unsigned char buf[BUFSIZE];
    static char strMd[SHA_DIGEST_LENGTH*2+1];

    if ((fd = open(filePath, O_RDONLY)) < 0) {
        SYS_LOGE("Open %s error(%s)!\n", filePath, strerror(errno));
        return NULL;
    }
    SHA1_Init(&c);
    for (;;)
    {
        size = read(fd, buf, BUFSIZE);
        if (size <= 0) break;
        SHA1_Update(&c, buf, (unsigned long)size);
    }
    SHA1_Final(md, &c);
    close(fd);

    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        sprintf(strMd + i*2, "%02x", md[i]);
    strMd[SHA_DIGEST_LENGTH*2] = '\0';
    return strMd;
}

int CPQControl::GenerateTargetPQ()
{
    char basePQPath[128] = {0};
    char diffPQPath[128] = {0};
    char targetPQSha1[128] = {0};
    char basePQSha1[128] = {0};

    const char *config_value = NULL;
    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ_DIFF, CFG_BASE_PQ_PATH, NULL);
    if (!config_value) {
        SYS_LOGE("Can not get basePQPath!\n");
        return -1;
    }
    strcpy(basePQPath, config_value);

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ_DIFF, CFG_DIFF_PQ_PATH, NULL);
    if (!config_value) {
        SYS_LOGE("Can not get diffPQPath!\n");
        return -1;
    }
    strcpy(diffPQPath, config_value);

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ_DIFF, CFG_TARGET_PQ_SHA1, NULL);
    if (!config_value) {
        SYS_LOGE("Can not get targetPQSha1!\n");
        return -1;
    }
    strcpy(targetPQSha1, config_value);

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ_DIFF, CFG_BASE_PQ_SHA1, NULL);
    if (!config_value) {
        SYS_LOGE("Can not get basePQSha1!\n");
        return -1;
    }
    strcpy(basePQSha1, config_value);

    if (!isFileExist(basePQPath) && (!strcmp(basePQSha1, CalculateFileSha1(basePQPath)))) {
        SYS_LOGE("Base pq.db is not exist or sha1 value [%s] is error!\n", CalculateFileSha1(basePQPath));
        return -1;
    }

    if (!isFileExist(diffPQPath)) {
        SYS_LOGE("%s is not exist!\n", CalculateFileSha1(diffPQPath));
        return -1;
    }

    if (pqbspatch(basePQPath, PARAM_PQ_DB_PATH, diffPQPath) != 0) {
        SYS_LOGE("Doing bspatch pq is failed\n");
        return -1;
    }

    if (strcmp(targetPQSha1, CalculateFileSha1(PARAM_PQ_DB_PATH))) {
        SYS_LOGE("Target pq.db sha1 value [%s] is not equal targetPQSha1[%s]!\n", CalculateFileSha1(PARAM_PQ_DB_PATH), targetPQSha1);
        return -1;
    }
    return 0;
}
#endif

bool CPQControl::getBootEnv(const char *name, char *value)
{
    bool result = false;
    char env_name_buffer[100] = {'\0'};

    memset(env_name_buffer, '\0', sizeof(env_name_buffer));
    if (strstr(name, "ubootenv.var.") == NULL) {
        SYS_LOGE("%s uboot_env_name does not include \"ubootenv.var.\" prefix, now add it!", __FUNCTION__);
        sprintf(env_name_buffer, "ubootenv.var.%s", name);
    } else {
        sprintf(env_name_buffer, "%s", name);
    }
/*
    if (pqUbootenv != NULL) {
        const char* p_value = pqUbootenv->getValue(env_name_buffer);
        if (p_value) {
            strcpy(value, p_value);
            SYS_LOGD("%s read [%s]=%s", __FUNCTION__, name, value);
            result = true;
        } else {
            SYS_LOGE("%s get %s failed!\n ", __FUNCTION__, name);
        }
    } else {
        SYS_LOGE("%s get [%s] fail", __FUNCTION__, name);
    }
*/
    return result;
}

int CPQControl::getHdrPolicy(void)
{
    int ret = -1;
    char hdr_policy[9] = {0};

    if (mPQdb->mDbMatchType != MATCH_TYPE_MBOX_S5 &&
        mPQdb->mDbMatchType != MATCH_TYPE_MBOX_S7D) {
        return 0;
    }

    memset(hdr_policy, 0, sizeof(hdr_policy));
    ret = pqReadSys(PQ_DISPLAY_HDR_POLICY, hdr_policy, (sizeof(hdr_policy)-1));
    if (ret > 0) {
        hdr_policy[ret] = 0;
    } else {
        memset(hdr_policy, 0, sizeof(hdr_policy));
    }
    SYS_LOGD("%s ret %d hdr_policy %s\n", __FUNCTION__, ret, hdr_policy);

    if (strcmp(hdr_policy, "1") == 0) { //adaptive Hdr
        SYS_LOGD("%s adaptive Hdr\n", __FUNCTION__);
        mPQdb->node_number = 1;
    } else if (strcmp(hdr_policy, "0") == 0) { //always hdr
        SYS_LOGD("%s always Hdr\n", __FUNCTION__);
        mPQdb->node_number = 2;
    } else {
        SYS_LOGE("%s hdr policy setting out of range\n", __FUNCTION__);
    }

    return ((ret == true) ? 1 : 0);
}

int CPQControl::GetVideoVdProcState(void)
{
    int readRet = -1;
    int slice_num = 0;
    int length = 13; //ex: "slice_num:[1]" 13 char
    int i = 0;
    char vd_proc_state[100] = {0};
    char slice_infor[20] = {0};
    char find_string[10] = "slice_num";
    char *findRet = NULL;

    readRet =  pqReadSys(VIDEO_VD_PROC_STATE, vd_proc_state, sizeof(vd_proc_state));
    if (readRet > 0) {
        findRet = strstr(vd_proc_state, find_string);
        if (findRet) {
            strncpy(slice_infor, findRet, length);
            SYS_LOGD("%s slice_infor:%s\n", __FUNCTION__, slice_infor);

            while (slice_infor[i] != '\0') {
                if (isdigit(slice_infor[i])) {
                    slice_num = slice_infor[i] - '0';
                    SYS_LOGD("%s slice_infor[%d]:%c\n", __FUNCTION__, i, slice_infor[i]);
                    break;
                }

                i++;
            }
        } else {
            SYS_LOGE("%s no slice_num during vd_proc_state\n", __FUNCTION__);
        }
    } else {
        SYS_LOGE("%s read /sys/class/video/video_vd_proc_state failed\n", __FUNCTION__);
    }

    SYS_LOGD("%s slice_num %d\n", __FUNCTION__, slice_num);

    return slice_num;
}

int CPQControl::GetCurrentFrameRate(tv_source_input_t src_input)
{
    int ret = -1;
    tvin_info_s vdinSignalInfo;

    memset(&vdinSignalInfo, 0, sizeof(tvin_info_s));

    if (src_input >= SOURCE_HDMI1 && src_input <= SOURCE_HDMI4) {
        ret = VDINDeviceIOCtl(TVIN_IOC_G_SIG_INFO, &vdinSignalInfo);
        if (ret < 0) {
            SYS_LOGD("%s: Get Tvin_GetSignalInfo fail!\n", __FUNCTION__);
        } else {
            SYS_LOGD("%s: framerate:%d\n", __FUNCTION__, vdinSignalInfo.fps);
            mFrameRate = vdinSignalInfo.fps;
        }
    } else {
        mFrameRate = 60;
    }

    return mFrameRate;
}

int CPQControl::SetSrTable_WorkArroundByEvent(void)
{
    int ret = -1;
    int sliceNum = 0;
    int sharpness = 0;

    sliceNum = GetVideoVdProcState();
    SYS_LOGD("%s sliceNum %d\n", __FUNCTION__, sliceNum);

    if (sliceNum == 1) {
        mPQdb->node_number = 1;
    } else if (sliceNum == 2) {
        mPQdb->node_number = 2;
    } else {
        SYS_LOGD("%s sliceNum is out range, do nothing\n", __FUNCTION__);
        return ret;
    }
    SYS_LOGD("%s node_number %d\n", __FUNCTION__, mPQdb->node_number);

    //reload sr table
    ret = Cpq_SetSharpness0FixedParam(GetSuperResolution(),mCurrentSourceInputInfo);
    ret |= Cpq_SetSharpness0VariableParam(mCurrentSourceInputInfo);

    ret |= Cpq_SetSharpness1FixedParam(GetSuperResolution(), mCurrentSourceInputInfo);
    ret |= Cpq_SetSharpness1VariableParam(mCurrentSourceInputInfo);

    //reset sharpness
    sharpness = GetSharpness();
    ret |= Cpq_SetSharpness(sharpness, mCurrentSourceInputInfo);
    SYS_LOGD("%s sharpness:%d ret %d\n", __FUNCTION__, sharpness, ret);

    return ret;
}

int CPQControl::SetAmDolbyIQType(int type)
{
    if (mCurrentHdrType != HDR_TYPE_DOVI) {
        SYS_LOGD("%s: Not Dv Mode, SKIP!\n", __FUNCTION__, type, IsDvApoTypeGame);
        return 0;
    }

    if (type == AMDV_APOO_TYPE_2 && (mCurrentPictureMode != PICTURE_MODE_AMDOLBY_IQ && mCurrentPictureMode != PICTURE_MODE_GAME)) {// dv tye == 2 and not iq mode or game mode
        SYS_LOGD("%s: DV IQ Type Event = %d\n", __FUNCTION__, type);
        IsDvApoTypeGame = 1;
        SetPQPictureMode(PICTURE_MODE_GAME);
    } else {
        if (IsDvApoTypeGame) {
            IsDvApoTypeGame = 0;
            SetPQPictureMode((PICTURE_MODE)GetPQMode());
        }
    }

    if (mCurrentPictureMode == PICTURE_MODE_AMDOLBY_DARK && type != AMDV_APOO_TYPE_2) {
        SYS_LOGD("%s: PICTURE_MODE_AMDOLBY_DARK mode  skip Apo\n", __FUNCTION__, type, IsDvApoTypeGame);
    } else {
        RefreshDvApoPictureMode(type);
    }

    SYS_LOGD("%s: mCurrentPictureMode = %d, DV IQ Type = %d, IsDvApoTypeGame = %d\n", __FUNCTION__, mCurrentPictureMode, type, IsDvApoTypeGame);
    return 0;
}

int CPQControl::GetNonlinearOsdRemapVal(nonline_params_type_t type, int Value)
{
    NonlinearModeType pData;
    if (!GetNonlinearData(&pData)) {
        SYS_LOGE("%s: GetNonlinearData fail\n", __FUNCTION__);
        return -1;
    }

    int temp = 0;
    unsigned char startPoint = 0, endPoint = 0;
    unsigned char point0 = 0;
    unsigned char point25 = 0;
    unsigned char point50 = 0;
    unsigned char point75 = 0;
    unsigned char point100 = 0;

    switch (type) {
        case _BRIGHTNESS:
            point0   = pData.Brightness_0;
            point25  = pData.Brightness_25;
            point50  = pData.Brightness_50;
            point75  = pData.Brightness_75;
            point100 = pData.Brightness_100;
            break;
        case _CONTRAST:
            point0   = pData.Contrast_0;
            point25  = pData.Contrast_25;
            point50  = pData.Contrast_50;
            point75  = pData.Contrast_75;
            point100 = pData.Contrast_100;
            break;
        case _SATURATION:
            point0   = pData.Saturation_0;
            point25  = pData.Saturation_25;
            point50  = pData.Saturation_50;
            point75  = pData.Saturation_75;
            point100 = pData.Saturation_100;
            break;
        case _HUE:
            point0   = pData.Hue_0;
            point25  = pData.Hue_25;
            point50  = pData.Hue_50;
            point75  = pData.Hue_75;
            point100 = pData.Hue_100;
            break;
        case _SHARPNESS:
            point0   = pData.Sharpness_0;
            point25  = pData.Sharpness_25;
            point50  = pData.Sharpness_50;
            point75  = pData.Sharpness_75;
            point100 = pData.Sharpness_100;
            break;
        case _BACKLIGHT:
            point0   = pData.Backlight_0;
            point25  = pData.Backlight_25;
            point50  = pData.Backlight_50;
            point75  = pData.Backlight_75;
            point100 = pData.Backlight_100;
            break;
    default:
            break;
    }

    if (Value < 25) {
        startPoint = point0;
        endPoint = point25;
        temp = Value;
    } else if ((Value >= 25) && (Value < 50)) {
        startPoint = point25;
        endPoint = point50;
        temp = Value - 25;
    } else if ((Value >= 50) && (Value < 75)) {
        startPoint = point50;
        endPoint = point75;
        temp = Value - 50;
    } else if (Value >= 75) {
        startPoint = point75;
        endPoint = point100;
        temp = Value - 75;
    }

    if (endPoint >= startPoint) {
        temp = (endPoint - startPoint) * temp / 25;
        temp +=  startPoint;
    } else if ((endPoint < startPoint)) {
        temp = (startPoint-endPoint) * temp / 25;
        temp = startPoint - temp;
    }

    return temp;
}

bool CPQControl::IsDongleLowPowerPqOff(void)
{
    char propbuf[PROPERTY_VALUE_MAX] = {0};
    bool pq_off = false;

    if (!mbCpqCfg_dongle_low_power_enable)
        return false;

    if (property_get(PROP_DONGLE_LOW_POWER_PQ_OFF, propbuf, "on") > 0) {
        SYS_LOGD("%s: Prop [%s]=%s\n", __FUNCTION__, PROP_DONGLE_LOW_POWER_PQ_OFF, propbuf);
        if (strcasecmp(propbuf, "off") == 0)
            pq_off = true;
    } else {
        SYS_LOGE("%s: getprop [%s] fail\n", __FUNCTION__, PROP_DONGLE_LOW_POWER_PQ_OFF);
    }

    return pq_off;
}

//DATABASE
bool CPQControl::SetPictureMode(PICTURE_MODE_DEFAULT *params)
{
    if (mDataBase == NULL) {
        return false;
    }

    return mDataBase->SetPictureMode(params, CurSource, CurTimming);
}

bool CPQControl::GetPictureMode(PICTURE_MODE_DEFAULT *params)
{
    if (mDataBase == NULL) {
        return false;
    }

    if (mDataBase->GetPictureMode(params, CurSource, CurTimming)) {
        return true;
    }

    if (mDataBase->GetPictureMode(params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT)) {
        return true;
    }

    return false;
}

bool CPQControl::SetLastPictureMode(PICTURE_MODE_DEFAULT *params)
{
    if (mDataBase == NULL) {
        return false;
    }

    return mDataBase->SetLastPictureMode(params, CurSource, CurTimming);
}

bool CPQControl::GetLastPictureMode(PICTURE_MODE_DEFAULT *params)
{
    if (mDataBase == NULL) {
        return false;
    }

    if (mDataBase->GetLastPictureMode(params, CurSource, CurTimming)) {
        return true;
    }

    if (mDataBase->GetLastPictureMode(params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT)) {
        return true;
    }

    return false;
}

bool CPQControl::ResetPictureMode(void)
{
    if (mDataBase == NULL ) {
        return false;
    }

    PICTURE_MODE_DEFAULT params;
    if (!mDataBase->GetDefaultPictureMode(&params, CurSource, CurTimming)) {
        if (!mDataBase->GetDefaultPictureMode(&params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT)) {
            SYS_LOGE("[%s] GetDefaultPictureModeData src:%d, timing:%d failed", __FUNCTION__, CurSource, CurTimming);
            return false;
        }
    }

    if (!mDataBase->SetPictureMode(&params, CurSource, CurTimming)) {
        SYS_LOGE("[%s] mDataBase->SetPictureMode src:%d, timing:%d  failed", __FUNCTION__, CurSource, CurTimming);
    }

    return true;
}

bool CPQControl::ResetPictureModeAll(void)
{
    SYS_LOGD("%s: start\n", __FUNCTION__);
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    int ret = -1;
    PICTURE_MODE_DEFAULT params;
    for (int i = PQ_SRC_DEFAULT; i < PQ_SRC_MAX; i++) {
        for (int j = PQ_SIGFMT_DEFAULT; j < PQ_SIGFMT_MAX; j++) {
            ret = 0;
            if (mDataBase->GetPictureMode(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j)) {
                if (!mDataBase->GetDefaultPictureMode(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j)) {
                    if (!mDataBase->GetDefaultPictureMode(&params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT)) {
                        SYS_LOGE("[%s] GetDefaultPictureModeData src:%d, timing:%d failed", __FUNCTION__, i, j);
                        ret =  -1;
                    }
                }

                if (ret == 0) {
                    if (!mDataBase->SetPictureMode(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j)) {
                        SYS_LOGE("[%s] mDataBase->SetPictureMode src:%d, timing:%d  failed", __FUNCTION__, i, j);
                    }
                }
            }
        }
    }

    SYS_LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::SetPictureModeData(PICTURE_MODE_DATA *params, PICTURE_MODE PictureMode)
{
    if (mDataBase == NULL ) {
        return false;
    }

    return mDataBase->SetPictureModeData(params, CurSource, CurTimming, PictureMode);
}

bool CPQControl::GetPictureModeData(PICTURE_MODE_DATA *params, PICTURE_MODE PictureMode)
{
    if (mDataBase == NULL) {
        return false;
    }

    if (mDataBase->GetPictureModeData(params, CurSource, CurTimming, PictureMode)) {
        return true;
    }

    if (mDataBase->GetPictureModeData(params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, PictureMode)) {
        return true;
    }

    return false;
}

bool CPQControl::SetPictureModeCustomData(PICTURE_MODE_DATA *params, PICTURE_MODE PictureMode, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (mDataBase == NULL ) {
        return false;
    }

    return mDataBase->SetPictureModeData(params, src, timing, PictureMode);
}

bool CPQControl::GetPictureModeCustomData(PICTURE_MODE_DATA *params, PICTURE_MODE PictureMode, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (mDataBase == NULL) {
        return false;
    }

    if (mDataBase->GetPictureModeData(params, src, timing, PictureMode)) {
        return true;
    }

    if (mDataBase->GetPictureModeData(params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, PictureMode)) {
        return true;
    }

    return false;
}

bool CPQControl::ResetPictureModeData(void)
{
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    PICTURE_MODE Mode = (PICTURE_MODE)GetPQMode();
    PICTURE_MODE_DATA params;
    if (!mDataBase->GetDefaultPictureModeData(&params, CurSource, CurTimming, Mode)) {
        if (!mDataBase->GetDefaultPictureModeData(&params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, Mode)) {
            SYS_LOGE("[%s] GetDefaultPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, CurSource, CurTimming, Mode);
            return false;
        }
    }

    return mDataBase->SetPictureModeData(&params, CurSource, CurTimming, Mode);
}

bool CPQControl::ResetPictureModeDataAll(void)
{
    SYS_LOGD("%s: start\n", __FUNCTION__);
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    int ret = -1;
    PICTURE_MODE_DATA params;
    for (int i = PQ_SRC_DEFAULT; i < PQ_SRC_MAX; i++) {
        for (int j = PQ_SIGFMT_DEFAULT; j < PQ_SIGFMT_MAX; j++) {
            for (int k = PICTURE_MODE_STANDARD; k < PICTURE_MODE_MAX; k++) {
                ret = 0;
                if (mDataBase->GetPictureModeData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j, (PICTURE_MODE)k)) {
                    if (!mDataBase->GetDefaultPictureModeData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j, (PICTURE_MODE)k)) {
                        if (!mDataBase->GetDefaultPictureModeData(&params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, (PICTURE_MODE)k)) {
                            SYS_LOGE("[%s] mDataBase->GetDefaultPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, i, j, k);
                            ret = -1;
                        }
                    }

                    if (ret == 0) {
                        if (!mDataBase->SetPictureModeData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j, (PICTURE_MODE)k)) {
                             SYS_LOGE("[%s] mDataBase->SetPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, i, j, k);
                        }
                    }
                }
            }
        }
    }

    SYS_LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::SetNonlinearData(NonlinearModeType *params)
{
    if (mDataBase == NULL ) {
        return false;
    }

    return mDataBase->SetNonlinearData(params, CurSource, CurTimming);
}

bool CPQControl::GetNonlinearData(NonlinearModeType *params)
{
    if (mDataBase == NULL ) {
        return false;
    }

    if (mDataBase->GetNonlinearData(params, CurSource, CurTimming)) {
        return true;
    }

    if (mDataBase->GetNonlinearData(params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT)) {
        return true;
    }

    return false;
}

bool CPQControl::SetNonlinearCustomData(NonlinearModeType *params, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (mDataBase == NULL ) {
        return false;
    }

    return mDataBase->SetNonlinearData(params, src, timing);
}

bool CPQControl::GetNonlinearCustomData(NonlinearModeType *params, pq_source_input_t src, pq_sig_fmt_t timing)
{
    if (mDataBase == NULL ) {
        return false;
    }

    if (mDataBase->GetNonlinearData(params, src, timing)) {
        return true;
    }

    if (mDataBase->GetNonlinearData(params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT)) {
        return true;
    }

    return false;
}

bool CPQControl::ResetNonlinearData(void)
{
    if (mDataBase == NULL ) {
        return false;
    }

    NonlinearModeType params;
    if (!mDataBase->GetDefaultNonlinearData(&params, CurSource, CurTimming)) {
        if (mDataBase->GetDefaultNonlinearData(&params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT)) {
            SYS_LOGE("[%s] GetDefaultNonlinearData src:%d, timing:%d failed", __FUNCTION__, CurSource, CurTimming);
            return false;
        }
    }

    return mDataBase->SetNonlinearData(&params, CurSource, CurTimming);
}

bool CPQControl::ResetNonlinearDataAll(void)
{
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    int ret = -1;
    NonlinearModeType params;
    for (int i = PQ_SRC_DEFAULT; i < PQ_SRC_MAX; i++) {
        for (int j = PQ_SIGFMT_DEFAULT; j < PQ_SIGFMT_MAX; j++) {
            ret = 0;
            if (mDataBase->GetNonlinearData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j)) {
                if (!mDataBase->GetDefaultNonlinearData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j)) {
                    if (!mDataBase->GetDefaultNonlinearData(&params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT)) {
                        SYS_LOGE("[%s] mDataBase->GetDefaultNonlinearData src:%d, timing:%d failed", __FUNCTION__, i, j);
                        ret = -1;
                    }
                }

                if (ret == 0) {
                    if (!mDataBase->SetNonlinearData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j)) {
                        SYS_LOGE("[%s] mDataBase->SetNonlinearData src:%d, timing:%d failed", __FUNCTION__, i, j);
                    }
                }
            }
        }
    }

    return true;
}

bool CPQControl::SetColorTemperatureData(COLORTEMP_DATA *params, int level)
{
    if (mDataBase == NULL ) {
        return false;
    }

    if (mDataBase->SetColorTemperatureData(params, CurSource, CurTimming, level) == true) {
        return true;
    }

    return false;
}

bool CPQControl::GetColorTemperatureData(COLORTEMP_DATA *params, int level)
{
    if (mDataBase == NULL ) {
        return false;
    }

    if (mDataBase->GetColorTemperatureData(params, CurSource, CurTimming, level)) {
        return true;
    }

    if (mDataBase->GetColorTemperatureData(params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, level)) {
        return true;
    }

    return false;
}

bool CPQControl::ResetColorTemperatureData(void)
{
    if (mDataBase == NULL ) {
        return false;
    }

    SYS_LOGD("[%s] start", __FUNCTION__);
    int ColorTemp = GetColorTemperature();
    COLORTEMP_DATA params;
    if (!mDataBase->GetDefaultColorTemperatureData(&params, CurSource, CurTimming, ColorTemp)) {
        if (!mDataBase->GetDefaultColorTemperatureData(&params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, ColorTemp)) {
            SYS_LOGE("[%s] GetDefaultColorTemperatureData src:%d, timing:%d failed", __FUNCTION__, CurSource, CurTimming);
            return false;
        }
    }

    return mDataBase->SetColorTemperatureData(&params, CurSource, CurTimming, ColorTemp);
}

bool CPQControl::ResetColorTemperatureDataAll(void)
{
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    int ret = -1;
    COLORTEMP_DATA params;
    for (int i = PQ_SRC_DEFAULT; i < PQ_SRC_MAX; i++) {
        for (int j = PQ_SIGFMT_DEFAULT; j < PQ_SIGFMT_MAX; j++) {
            for (int k = COLOR_TMP_MODE_STANDARD; k < COLOR_TMP_MODE_MAX; k++) {
                ret = 0;
                if (mDataBase->GetColorTemperatureData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j, k)) {
                    if (!mDataBase->GetDefaultColorTemperatureData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j, k)) {
                        if (!mDataBase->GetDefaultColorTemperatureData(&params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, k)) {
                            SYS_LOGE("[%s] mDataBase->GetDefaultColorTemperatureData src:%d, timing:%d colortemp:%d failed", __FUNCTION__, i, j, k);
                            ret = -1;
                        }
                    }

                    if (ret == 0) {
                        if (!mDataBase->SetColorTemperatureData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j, (vpp_picture_mode_t)k)) {
                             SYS_LOGE("[%s] mDataBase->SetColorTemperatureData src:%d, timing:%d  mode %d failed", __FUNCTION__, i, j, k);
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool CPQControl::ResetMultipointGammaData(void)
{
    if (mDataBase == NULL ) {
        return false;
    }

    SYS_LOGD("[%s] start", __FUNCTION__);
    int ColorTemp = GetColorTemperature();
    COLORTEMP_DATA params;
    if (!GetColorTemperatureData(&params, ColorTemp)) {
        SYS_LOGE("[%s] GetColorTemperatureData failed", __FUNCTION__);
        return false;
    }

    COLORTEMP_DATA params1;
    if (!mDataBase->GetDefaultColorTemperatureData(&params1, CurSource, CurTimming, ColorTemp)) {
        if (!mDataBase->GetDefaultColorTemperatureData(&params1, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, ColorTemp)) {
            SYS_LOGE("[%s] GetDefaultColorTemperatureData src:%d, timing:%d failed", __FUNCTION__, CurSource, CurTimming);
            return false;
        }
    }

    memcpy(&params.GammaOffset, &params1.GammaOffset, sizeof(GAMMA_OFFSET_DATA));

    return mDataBase->SetColorTemperatureData(&params, CurSource, CurTimming, ColorTemp);
}

bool CPQControl::ResetMultipointGammaDataAll(void)
{
    if (mDataBase == NULL ) {
        return false;
    }

    int ret = -1;
    COLORTEMP_DATA params;
    COLORTEMP_DATA params1;
    for (int i = PQ_SRC_DEFAULT; i < PQ_SRC_MAX; i++) {
        for (int j = PQ_SIGFMT_DEFAULT; j < PQ_SIGFMT_MAX; j++) {
            for (int k = COLOR_TMP_MODE_STANDARD; k < COLOR_TMP_MODE_MAX; k++) {
                ret = 0;
                if (mDataBase->GetColorTemperatureData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j, k)) {
                    if (!mDataBase->GetDefaultColorTemperatureData(&params1, (pq_source_input_t)i, (pq_sig_fmt_t)j, k)) {
                        if (!mDataBase->GetDefaultColorTemperatureData(&params1, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, k)) {
                            SYS_LOGE("[%s] mDataBase->GetDefaultColorTemperatureData src:%d, timing:%d colortemp:%d failed", __FUNCTION__, i, j, k);
                            ret = -1;
                        }
                    }

                    if (ret == 0) {
                        memcpy(&params.GammaOffset, &params1.GammaOffset, sizeof(GAMMA_OFFSET_DATA));
                        if (!mDataBase->SetColorTemperatureData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j, (vpp_picture_mode_t)k)) {
                             SYS_LOGE("[%s] mDataBase->SetColorTemperatureData src:%d, timing:%d  mode %d failed", __FUNCTION__, i, j, k);
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool CPQControl::SetColorCustomizeData(TABLE_CMS *params)
{
    if (mDataBase == NULL ) {
        return false;
    }

    return mDataBase->SetColorCustomizeData(params, CurSource, CurTimming);
}

bool CPQControl::GetColorCustomizeData(TABLE_CMS *params)
{
    if (mDataBase == NULL ) {
        return false;
    }

    if (mDataBase->GetColorCustomizeData(params, CurSource, CurTimming)) {
        return true;
    }

    if (mDataBase->GetColorCustomizeData(params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT)) {
        return true;
    }

    return false;
}

bool CPQControl::ResetColorCustomizeData(void)
{
    if (mDataBase == NULL ) {
        return false;
    }

    TABLE_CMS params;
    if (!mDataBase->GetDefaultColorCustomizeData(&params, CurSource, CurTimming)) {
        if (mDataBase->GetDefaultColorCustomizeData(&params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT)) {
            SYS_LOGE("[%s] GetDefaultColorCustomizeData src:%d, timing:%d failed", __FUNCTION__, CurSource, CurTimming);
            return false;
        }
    }

    return mDataBase->SetColorCustomizeData(&params, CurSource, CurTimming);
}

bool CPQControl::ResetColorCustomizeDataAll(void)
{
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    int ret = -1;
    TABLE_CMS params;
    for (int i = PQ_SRC_DEFAULT; i < PQ_SRC_MAX; i++) {
        for (int j = PQ_SIGFMT_DEFAULT; j < PQ_SIGFMT_MAX; j++) {
            ret = 0;
            if (mDataBase->GetColorCustomizeData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j)) {
                if (!mDataBase->GetDefaultColorCustomizeData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j)) {
                    if (!mDataBase->GetDefaultColorCustomizeData(&params, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT)) {
                        SYS_LOGE("[%s] mDataBase->GetDefaultColorCustomizeData src:%d, timing:%d failed", __FUNCTION__, i, j);
                        ret = -1;
                    }
                }

                if (ret == 0) {
                    if (!mDataBase->SetColorCustomizeData(&params, (pq_source_input_t)i, (pq_sig_fmt_t)j)) {
                        SYS_LOGE("[%s] mDataBase->SetColorCustomizeData src:%d, timing:%d failed", __FUNCTION__, i, j);
                    }
                }
            }
        }
    }

    return true;
}

bool CPQControl::SetPictureStructDataBySrc(PICTURE_SETTING_BY_SRC *params)
{
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    return mDataBase->SetPictureStructDataBySrc(params, CurSource);
}

bool CPQControl::GetPictureStructDataBySrc(PICTURE_SETTING_BY_SRC *params)
{
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    if (mDataBase->GetPictureStructDataBySrc(params, CurSource)) {
        return true;
    }

    if (mDataBase->GetPictureStructDataBySrc(params,PQ_SRC_DEFAULT)) {
        return true;
    }

    SYS_LOGE("[%s] GetPictureStructDataBySrc src:%d failed", __FUNCTION__, CurSource);
    return false;
}

bool CPQControl::ResetPictureStructDataBySrc(void)
{
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    SYS_LOGD("[%s] start", __FUNCTION__);
    PICTURE_SETTING_BY_SRC params;
    if (!mDataBase->GetDefaultPictureStructDataBySrc(&params, CurSource)) {
        if (!mDataBase->GetDefaultPictureStructDataBySrc(&params, PQ_SRC_DEFAULT)) {
        SYS_LOGE("[%s] GetDefaultColorTemperatureData src:%d, timing:%d failed", __FUNCTION__, CurSource, CurTimming);
        return false;
        }
    }

    return mDataBase->SetPictureStructDataBySrc(&params, CurSource);
}

bool CPQControl::ResetPictureStructDataBySrcAll(void)
{
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    int ret = -1;
    PICTURE_SETTING_BY_SRC params;
    for (int i = PQ_SRC_DEFAULT; i < PQ_SRC_MAX; i++) {
        ret = 0;
        if (mDataBase->GetPictureStructDataBySrc(&params, (pq_source_input_t)i)) {
            if (!mDataBase->GetDefaultPictureStructDataBySrc(&params, (pq_source_input_t)i)) {
                if (!mDataBase->GetDefaultPictureStructDataBySrc(&params, PQ_SRC_DEFAULT)) {
                    SYS_LOGE("[%s] GetDefaultPictureStructDataBySrc src:%d failed", __FUNCTION__, i);
                    ret = -1;
                }
            }

            if (ret == 0) {
                if (!mDataBase->SetPictureStructDataBySrc(&params, (pq_source_input_t)i)) {
                    SYS_LOGE("[%s] mDataBase->SetPictureStructDataBySrc src:%d failed", __FUNCTION__, i);
                    return false;
                }
            }
        }
    }

    return true;
}

//PictureStructGlobal
bool CPQControl::SetPictureStructDataGlobal(PICTURE_SETTING_GLOBAL *params)
{
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    return mDataBase->SetPictureStructDataGlobal(params);
}

bool CPQControl::GetPictureStructDataGlobal(PICTURE_SETTING_GLOBAL *params)
{
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    return mDataBase->GetPictureStructDataGlobal(params);
}

bool CPQControl::ResetPictureStructDataGlobal(void)
{
    if (mDataBase == NULL ) {
        SYS_LOGE("[%s] mDataBase is NULL", __FUNCTION__);
        return false;
    }

    SYS_LOGD("[%s] start", __FUNCTION__);
    PICTURE_SETTING_GLOBAL params;
    if (!mDataBase->GetDefaultPictureStructDataGlobal(&params)) {
        SYS_LOGE("[%s] GetDefaultPictureStructDataGlobal failed", __FUNCTION__);
        return false;
    }

    return mDataBase->SetPictureStructDataGlobal(&params);
}

//CRI_DATA
bool CPQControl::FactoryGetWhitebalanceRGBGainOffsetData(RGB_GAIN_OFFSET *pData, int level)
{
    if (mWBDataBase == NULL ) {
        return false;
    }

    return mWBDataBase->GetRGBGainOffsetData(pData, level);
}

bool CPQControl::FactorySetWhitebalanceRGBGainOffsetData(RGB_GAIN_OFFSET *pData, int level)
{
    if (mWBDataBase == NULL ) {
        return false;
    }

    return mWBDataBase->SetRGBGainOffsetData(pData, level);
}

bool CPQControl::CheckCriDataWhitebalanceRGBGainOffsetData(void)
{
    int ret = -1;
    COLORTEMP_DATA Data;
    int colortemp = GetColorTemperature();
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&Data.rgbgo, colortemp)) {
        for (int i = COLOR_TMP_MODE_STANDARD; i < COLOR_TMP_MODE_MAX; i++) {
            ret = 0;
            if (!mDataBase->GetDefaultColorTemperatureData(&Data, CurSource, CurTimming, i)) {
                if (!mDataBase->GetDefaultColorTemperatureData(&Data, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, colortemp)) {
                    SYS_LOGE("%s GetDefaultColorTemperatureData colortemp: %d fail\n", __FUNCTION__, i);
                    ret = -1;
                }
            }

            if (ret == 0) {
                if (!FactorySetWhitebalanceRGBGainOffsetData(&Data.rgbgo, colortemp)) {
                    SYS_LOGE("%s FactorySetWhitebalanceRGBGainOffsetData colortemp: %d fail\n", __FUNCTION__, i);
                }
            }
        }
    }

    return true;
}

bool CPQControl::FactoryGetMultipointGammaData(Multipoint_GAMMA_DATA *pData, int level)
{
    if (mWBDataBase == NULL ) {
        return false;
    }

    return mWBDataBase->GetWhitebalanceGammaData(pData, level);
}

bool CPQControl::FactorySetMultipointGammaData(Multipoint_GAMMA_DATA *pData, int level)
{
    if (mWBDataBase == NULL ) {
        return false;
    }

    return mWBDataBase->SetWhitebalanceGammaData(pData, level);
}

bool CPQControl::CheckCriDataMultipointGammaData(void)
{
    int ret = -1;
    COLORTEMP_DATA Data;
    int colortemp = GetColorTemperature();
    if (!FactoryGetMultipointGammaData(&Data.GammaOffset.Gamma, colortemp)) {
        for (int i = COLOR_TMP_MODE_STANDARD; i < COLOR_TMP_MODE_MAX; i++) {
            ret = 0;
            if (!mDataBase->GetDefaultColorTemperatureData(&Data, CurSource, CurTimming, i)) {
                if (!mDataBase->GetDefaultColorTemperatureData(&Data, PQ_SRC_DEFAULT, PQ_SIGFMT_DEFAULT, colortemp)) {
                    SYS_LOGE("%s GetDefaultColorTemperatureData colortemp: %d fail\n", __FUNCTION__, i);
                    ret = -1;
                }
            }

            if (ret == 0) {
                if (!FactorySetMultipointGammaData(&Data.GammaOffset.Gamma, colortemp)) {
                    SYS_LOGE("%s FactorySetMultipointGammaData colortemp: %d fail\n", __FUNCTION__, i);
                }
            }
        }
    }

    return true;
}
