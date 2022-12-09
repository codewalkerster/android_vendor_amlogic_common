#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "display/adap_display.h"


ADAP_STATUS_T ADAP_DISPLAY_INIT(void)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_DISPLAY_Uninit(void)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_DISPLAY_Open(void)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_DISPLAY_Close(void)
{
    return ADAP_OK;
}
