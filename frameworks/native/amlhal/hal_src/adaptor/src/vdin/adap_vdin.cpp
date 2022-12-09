#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "vdin/adap_vdin.h"
#include "ioctrl/vdin_cmd_id.h"


static int mVdinFd = -1;
static int mVdinFdIsOpened = 0;


ADAP_STATUS_T ADAP_VDIN_INIT(void)
{
    if (mVdinFdIsOpened) {
        LOGD("%s VdinFd has been opened.\n", __FUNCTION__);
        return ADAP_OK;
    }

    if (mVdinFd < 0) {
        mVdinFd = open("/dev/" VDIN_DEVICE_NAME, O_RDWR);
        LOGD("%s mVdinFd=%d.\n", __FUNCTION__, mVdinFd);
    }

    mVdinFdIsOpened = 1;

    if (mVdinFd < 0) {
        LOGE("%s failed.\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_VDIN_Uninit(void)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_VDIN_Open(void)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_VDIN_Close(void)
{
    return ADAP_OK;
}

