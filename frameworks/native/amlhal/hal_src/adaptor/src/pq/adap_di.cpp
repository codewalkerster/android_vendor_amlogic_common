#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "pq/adap_di.h"
#include "ioctrl/di_cmd_id.h"


static int mDiFd = -1;
static int mDiFdIsOpened = 0;


ADAP_STATUS_T ADAP_DI_INIT(void)
{
    if (mDiFdIsOpened) {
        return ADAP_OK;
    }

    if (mDiFd < 0)
        mDiFd = open("/dev/" DI_DEVICE_NAME, O_RDWR);

    mDiFdIsOpened = 1;

    if (mDiFd < 0) {
        return ADAP_NOT_OK;
    } else {
        if (ioctl(mDiFd, DI_IOC_INIT) < 0) {
            return ADAP_NOT_OK;
        }
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_DI_Uninit(void)
{
    if (mDiFd >= 0) {
        mDiFd = -1;
        mDiFdIsOpened = 0;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_DI_Open(void)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_DI_Close(void)
{
    return ADAP_OK;
}

