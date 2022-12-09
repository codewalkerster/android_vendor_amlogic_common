#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

#include "pq/adap_ld.h"
#include "ioctrl/ld_cmd_id.h"
//#include "PQTableLogic.h"



/**
*** definition
**/
static int mLdFd = -1;
static int mLdFdIsOpened = 0;
pthread_mutex_t ld_mutex = PTHREAD_MUTEX_INITIALIZER;



/**
*** function
**/
ADAP_STATUS_T ADAP_LD_INIT(void)
{
    if (mLdFdIsOpened) {
        LOGD("%s LdFd has been opened.\n", __FUNCTION__);
        return ADAP_OK;
    }

    if (mLdFd < 0) {
        mLdFd = open("/dev/" LD_DEVICE_NAME, O_RDWR);
    }

    mLdFdIsOpened = 1;

    if (mLdFd < 0) {
        LOGE("%s failed err:%s\n", __FUNCTION__, strerror(errno));
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LD_UNINIT(void)
{
    if (mLdFd >= 0) {
        mLdFd = -1;
        mLdFdIsOpened = 0;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_LD_DevIoCtl(int request, ...)
{
    /* note: todo
    ** due to there is no ld dev in driver now, and for tvfuse socts.
    ** so we temporary return OK for upper
    */
    if (request == AML_LDIM_IOC_NR_SET_FUNC_EN ||
        request == (int)AML_LDIM_IOC_NR_GET_FUNC_EN ||
        request == AML_LDIM_IOC_NR_SET_DEMOMODE ||
        request == (int)AML_LDIM_IOC_NR_GET_DEMOMODE) {
        return ADAP_OK;
    }

    int ret = -1;
    if (mLdFd < 0) {
        LOGE("%s mLdFd is not opened.\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    pthread_mutex_lock(&ld_mutex);
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    ret = ioctl(mLdFd, request, arg);
    LOGI("%s %s\n", __FUNCTION__, (ret < 0) ? "faile" : "success");
    pthread_mutex_unlock(&ld_mutex);

    return (ret < 0) ? ADAP_NOT_OK : ADAP_OK;
}

ADAP_STATUS_T ADAP_LD_GetPqInitStatus(void)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_NR_GET_PQ_INIT);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_LD_SetPqInit(void)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_NR_SET_PQ_INIT);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_LD_GetLevelIdx(int *pLevelIdx)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_NR_GET_LEVEL_IDX, pLevelIdx);
    LOGI("%s %d %d\n", __FUNCTION__, ret, *pLevelIdx);

    return ret;
}

ADAP_STATUS_T ADAP_LD_SetLevelIdx(int iLevelIdx)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_NR_SET_LEVEL_IDX, &iLevelIdx);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_LD_GetFuncEn(int *pFuncEn)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_NR_GET_FUNC_EN, pFuncEn);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_LD_SetFuncEn(int iFuncEn)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_NR_SET_FUNC_EN, &iFuncEn);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_LD_GetRemapEn(int *pRemapEn)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_NR_GET_REMAP_EN, pRemapEn);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_LD_SetRemapEn(int iRemapEn)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_NR_SET_REMAP_EN, &iRemapEn);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_LD_GetBlMatrix(int *pBlMatrix)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_NR_GET_BL_MATRIX, pBlMatrix);
    LOGI("%s %d %d\n", __FUNCTION__, ret, *pBlMatrix);

    return ret;
}

ADAP_STATUS_T ADAP_LD_SetBlMatrix(int iMatrix)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_NR_SET_BL_MATRIX, &iMatrix);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_LD_GetDemoMode(int *pDemoMode)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_NR_GET_DEMOMODE, pDemoMode);
    LOGI("%s %d %d\n", __FUNCTION__, ret, *pDemoMode);

    return ret;
}

ADAP_STATUS_T ADAP_LD_SetDemoMode(int iDemoMode)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_NR_SET_DEMOMODE, &iDemoMode);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_LD_GetLdmInfo(aml_ldim_pq_t *pLdmInfo)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_CMD_GET_INFO_NEW, pLdmInfo);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_LD_SetLdmInfo(adap_ld_level_e eLevel)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    /*
    aml_ldim_pq_t ldm_param_drv;
    ldim_pq_t ldm_param_table;

    memset(&ldm_param_table, 0, sizeof(ldim_pq_t));

    PQTableLogic::instance()->GetLdmParam((pq_level_t)eLevel, &ldm_param_table);
    memcpy(&ldm_param_drv, &ldm_param_table, sizeof(ldim_pq_t));

    ret = ADAP_LD_DevIoCtl(AML_LDIM_IOC_CMD_SET_INFO_NEW, &ldm_param_drv);
    LOGI("%s %d\n", __FUNCTION__, ret);
*/
    return ret;
}
