#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "DV/adap_dv.h"
#include "ioctrl/dv_cmd_id.h"


static int mAmDolbyFd = -1;
static int mAmDolbyFdIsOpened = 0;


ADAP_STATUS_T ADAP_AMDOLBY_INIT(void)
{
    if (mAmDolbyFdIsOpened) {
        LOGD("%s mAmDolbyFd has been opened.\n", __FUNCTION__);
        return ADAP_OK;
    }

    if (mAmDolbyFd < 0) {
        mAmDolbyFd = open("/dev/" AMDOLBY_DEVICE_NAME, O_RDWR);
        LOGD("%s mAmDolbyFd=%d.\n", __FUNCTION__, mAmDolbyFd);
    }

    mAmDolbyFdIsOpened = 1;

    if (mAmDolbyFd < 0) {
        LOGE("%s failed.\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_AMDOLBY_Uninit(void)
{
    mAmDolbyFd = -1;
    mAmDolbyFdIsOpened = 0;
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_AMDOLBY_Open(void)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_AMDOLBY_Close(void)
{
    return ADAP_OK;
}

