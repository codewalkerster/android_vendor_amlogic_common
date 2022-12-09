#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

#include "pq/adap_memc.h"
#include "ioctrl/memc_cmd_id.h"



/**
*** definition
**/
static int mMemcFd = -1;
static int mMemcFdIsOpened = 0;
pthread_mutex_t memc_mutex = PTHREAD_MUTEX_INITIALIZER;



/**
*** function
**/
ADAP_STATUS_T ADAP_MEMC_INIT(void)
{
    if (mMemcFdIsOpened) {
        LOGD("%s MemcFd has been opened.\n", __FUNCTION__);
        return ADAP_OK;
    }

    if (mMemcFd < 0) {
        mMemcFd = open("/dev/" MEMC_DEVICE_NAME, O_RDWR);
    }

    mMemcFdIsOpened = 1;

    if (mMemcFd < 0) {
        LOGE("%s failed err:%s\n", __FUNCTION__, strerror(errno));
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_MEMC_UNINIT(void)
{
    if (mMemcFd >= 0) {
        mMemcFd = -1;
        mMemcFdIsOpened = 0;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_MEMC_DevIoCtl(int request, ...)
{
    SINT32 ret = -1;
    if (mMemcFd < 0) {
        LOGE("%s mMemcFd is not opened.\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    pthread_mutex_lock(&memc_mutex);
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    ret = ioctl(mMemcFd, request, arg);
    LOGI("%s %s\n", __FUNCTION__, (ret < 0) ? "fail" : "success");
    pthread_mutex_unlock(&memc_mutex);

    return (ret < 0) ? ADAP_NOT_OK : ADAP_OK;
}

ADAP_STATUS_T ADAP_MEMC_SetMemcEnable(SINT32 enable)
{
    if (mMemcFd < 0) {
        LOGE("%s mMemcFd is not opened.\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    if (ioctl(mMemcFd, FRC_IOC_SET_MEMC_ON_OFF, &enable) < 0) {
        LOGE("%s MEMC FRC_IOC_SET_MEMC_ON_OFF fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_MEMC_SetMemcDeJudderLevel(SINT32 level)
{
    if (mMemcFd < 0) {
        LOGE("%s mMemcFd is not opened.\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    if (ioctl(mMemcFd, FRC_IOC_SET_MEMC_LEVEL, &level) < 0) {
        LOGE("%s MEMC FRC_IOC_SET_MEMC_LEVEL fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_MEMC_SetMemcDeBlurLevel(SINT32 level)
{
    if (mMemcFd < 0) {
        LOGE("%s mMemcFd is not opened.\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    // need driver support

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_MEMC_SetLgeMemcInit(SINT32 iLgeMemcInit)
{
    if (mMemcFd < 0) {
        LOGE("%s mMemcFd is not opened.\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    if (ioctl(mMemcFd, FRC_IOC_SET_LGE_MEMC_INIT, &iLgeMemcInit) < 0) {
        LOGE("%s MEMC FRC_IOC_SET_LGE_MEMC_INIT fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_MEMC_SetLgeMemcLevel(struct v4l2_ext_memc_motion_comp_info *pLgeMemcInfo)
{
    if (mMemcFd < 0) {
        LOGE("%s mMemcFd is not opened.\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    if (ioctl(mMemcFd, FRC_IOC_SET_LGE_MEMC_LEVEL, pLgeMemcInfo) < 0) {
        LOGE("%s MEMC FRC_IOC_SET_LGE_MEMC_INIT fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_MEMC_GetLgeMemcLevel(struct v4l2_ext_memc_motion_comp_info *pLgeMemcInfo)
{
    if (mMemcFd < 0) {
        LOGE("%s mMemcFd is not opened.\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    if (ioctl(mMemcFd, FRC_IOC_GET_LGE_MEMC_LEVEL, pLgeMemcInfo) < 0) {
        LOGE("%s MEMC FRC_IOC_SET_LGE_MEMC_INIT fail\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}
