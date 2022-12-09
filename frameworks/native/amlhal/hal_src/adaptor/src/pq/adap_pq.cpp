#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>

#include "pq/adap_pq.h"
#include "ioctrl/pq_cmd_id.h"

static int mPqFd = -1;
static int mPqFdIsOpened = 0;
static adap_pq_source_timing_e eSrcTimingCur = ADAP_PQ_SRC_INDEX_VGA;
static adap_pq_dnlp_mode_e eDnlpModeCur = ADAP_PQ_DNLP_OFF;
static adap_pq_lc_mode_e eLcModeCur = ADAP_PQ_LC_OFF;

pthread_mutex_t pq_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
*** function
**/
ADAP_STATUS_T ADAP_PQ_INIT(void)
{
    if (mPqFdIsOpened) {
        LOGD("%s mPqFd has been opened.\n", __FUNCTION__);
        return ADAP_OK;
    }

    if (mPqFd < 0) {
        mPqFd = open("/dev/" PQ_DEVICE_NAME, O_RDWR);
    }

    if (mPqFd < 0) {
        LOGE("%s open mPqFd failed %s\n", __FUNCTION__, strerror(errno));
        return ADAP_NOT_OK;
    }

    mPqFdIsOpened = 1;

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_UNINIT(void)
{
    if (mPqFd >= 0) {
        mPqFd = -1;
        mPqFdIsOpened = 0;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_DevIoCtl(int request, ...)
{
    int ret = -1;
    if (mPqFd < 0) {
        LOGE("%s mPqFd is not opened.\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    pthread_mutex_lock(&pq_mutex);
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    ret = ioctl(mPqFd, request, arg);
    LOGI("%s %s\n", __FUNCTION__, (ret < 0) ? "fail" : "success");
    pthread_mutex_unlock(&pq_mutex);

    return (ret < 0) ? ADAP_NOT_OK : ADAP_OK;
}


ADAP_STATUS_T ADAP_PQ_SetBrightness(SINT32 value)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    SINT32 data = (value * (BRIGHTNESS_MAX - BRIGHTNESS_MIN) / 256) - BRIGHTNESS_MAX; //-512 ~ 512
    if (value >= 255) data = BRIGHTNESS_MAX;
    if (value <= 0)   data = BRIGHTNESS_MIN;

    am_pic_mode_t params;
    memset(&params, 0, sizeof(params));

    params.flag |= 0x1;
    params.brightness = data;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_PIC_MODE, &params);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_SetBrightness_OSD(SINT32 value)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    SINT32 data = (value * (BRIGHTNESS_MAX - BRIGHTNESS_MIN) / 256) - BRIGHTNESS_MAX; //-512 ~ 512
    if (value >= 255)
        data = BRIGHTNESS_MAX;
    if (value <= 0)
        data = BRIGHTNESS_MIN;

    am_pic_mode_t params;
    memset(&params, 0, sizeof(params));

    params.flag |= (0x1 << 1);
    params.brightness2 = data;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_PIC_MODE, &params);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetBrightness(SINT32 *pValue)
{

    return ADAP_NOT_SUPPORTED;
}

ADAP_STATUS_T ADAP_PQ_GetBrightness_OSD(SINT32 *pValue)
{
    return ADAP_NOT_SUPPORTED;
}

ADAP_STATUS_T ADAP_PQ_SetContrast(SINT32 value)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    SINT32 data = ((value * (CONTRAST_MAX - CONTRAST_MIN)) / 256) - CONTRAST_MAX;

    if (value >= 255)
        data = CONTRAST_MAX;
    if (value <= 0)
        data = CONTRAST_MIN;

    LOGD("%s Contrast = %d\n", __FUNCTION__, data);

    am_pic_mode_t params;
    memset(&params, 0, sizeof(params));

    params.flag |= (0x1 << 4);
    params.contrast = value;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_PIC_MODE, &params);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_SetContrast_OSD(SINT32 value)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    SINT32 data = ((value * (CONTRAST_MAX - CONTRAST_MIN)) / 256) - CONTRAST_MAX;

    if (value >= 255)
        data = CONTRAST_MAX;
    if (value <= 0)
        data = CONTRAST_MIN;

    am_pic_mode_t params;
    memset(&params, 0, sizeof(params));

    params.flag |= (0x1 << 5);
    params.contrast2 = value;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_PIC_MODE, &params);

    LOGI("%s %d\n", __FUNCTION__, ret);
    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetContrast(SINT32 *pValue)
{
    return ADAP_NOT_SUPPORTED;
}

ADAP_STATUS_T ADAP_PQ_GetContrast_OSD(SINT32 *pValue)
{
    return ADAP_NOT_SUPPORTED;
}

ADAP_STATUS_T ADAP_PQ_SetSaturation(SINT32 value)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_SATURATION, &value);

    LOGI("%s %d\n", __FUNCTION__, ret);
    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetSaturation(SINT32 *pValue)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_SetHue(SINT32 value)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_HUE, &value);

    LOGI("%s %d\n", __FUNCTION__, ret);
    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetHue(SINT32 *pValue)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_SetSaturationHue(SINT32 sat, SINT32 hue)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    int data_hue = ((hue * (HUE_MAX - HUE_MIN)) / 256) - HUE_MAX;
    if (hue >= 255)
        data_hue = HUE_MAX;
    if (hue <= 0)
        data_hue = HUE_MIN;

    SINT32 data_sat = (sat * (SATURATION_MAX - SATURATION_MIN) / 256) - SATURATION_MAX;
    if (sat >= 255)
        data_sat = SATURATION_MAX;
    if (sat <= 0)
        data_sat = SATURATION_MIN;
    SLONG temp = 0;
    video_set_saturation_hue((SINT8)data_sat, (SINT8)data_hue, &temp);

    LOGD("%s Saturation = %d, Hue = %d\n", __FUNCTION__, data_sat, data_hue);
    LOGD("%s temp = %x \n", __FUNCTION__, temp);

    am_pic_mode_t params;
    memset(&params, 0, sizeof(params));

    params.flag |= (0x1 << 2);
    params.saturation_hue = (int)temp;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_PIC_MODE, &params);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_SetSaturationHue_OSD(SINT32 sat, SINT32 hue)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    int data_hue = ((hue * (HUE_MAX - HUE_MIN)) / 256) - HUE_MAX;
    if (hue >= 255)
        data_hue = HUE_MAX;
    if (hue <= 0)
        data_hue = HUE_MIN;

    SINT32 data_sat = (sat * (SATURATION_MAX - SATURATION_MIN) / 256) - SATURATION_MAX;
    if (sat >= 255)
        data_sat = SATURATION_MAX;
    if (sat <= 0)
        data_sat = SATURATION_MIN;
    SLONG temp;
    video_set_saturation_hue((SINT8)data_hue, (SINT8)data_sat, &temp);

    am_pic_mode_t params;
    memset(&params, 0, sizeof(params));

    params.flag |= (0x1<<3);
    params.saturation_hue_post = (int)temp;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_PIC_MODE, &params);

    LOGI("%s %d\n", __FUNCTION__, ret);
    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetSaturationHue(SINT32 *pValue)
{
    return ADAP_NOT_SUPPORTED;
}

ADAP_STATUS_T ADAP_PQ_GetSaturationHue_OSD(SINT32 *pValue)
{
    return ADAP_NOT_SUPPORTED;
}


ADAP_STATUS_T ADAP_PQ_SetSharpness(SINT32 value)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_GetSharpness(SINT32 *pValue)
{
    return ADAP_NOT_SUPPORTED;
}

ADAP_STATUS_T ADAP_PQ_SetBacklight(SINT32 value)
{

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_GetBacklight(SINT32 *pValue)
{
    return ADAP_NOT_SUPPORTED;
}

ADAP_STATUS_T ADAP_PQ_SetColorTemp(vpp_white_balance_s *ptAdapPqWb)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    tcon_rgb_ogo_s st_wb_io;
    memset(&st_wb_io, 0, sizeof(struct tcon_rgb_ogo_s));

    st_wb_io.en = 1;
    st_wb_io.r_gain = ptAdapPqWb->R_val;
    st_wb_io.g_gain = ptAdapPqWb->G_val;
    st_wb_io.b_gain = ptAdapPqWb->B_val;
    st_wb_io.r_post_offset = ptAdapPqWb->R_offset_val;
    st_wb_io.g_post_offset = ptAdapPqWb->G_offset_val;
    st_wb_io.b_post_offset = ptAdapPqWb->B_offset_val;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_RGB_OGO, &st_wb_io);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetColorTemp(vpp_white_balance_s *ptAdapPqWb)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    tcon_rgb_ogo_s st_wb_io;
    memset(&st_wb_io, 0, sizeof(struct tcon_rgb_ogo_s));

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_GET_RGB_OGO, &st_wb_io);

    ptAdapPqWb->R_val = st_wb_io.r_gain;
    ptAdapPqWb->G_val = st_wb_io.g_gain;
    ptAdapPqWb->B_val = st_wb_io.b_gain;
    ptAdapPqWb->R_offset_val = st_wb_io.r_post_offset;
    ptAdapPqWb->G_offset_val = st_wb_io.g_post_offset;
    ptAdapPqWb->B_offset_val = st_wb_io.b_post_offset;

    return ret;
}

ADAP_STATUS_T ADAP_PQ_SetPreGamma(struct vpp_pre_gamma_table_s *pPreGamma)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_PRE_GAMMA_DATA, &pPreGamma);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetPreGamma(SINT32 *pValue)
{
    return ADAP_NOT_SUPPORTED;
}

ADAP_STATUS_T ADAP_PQ_SetGammaCurve(adap_pq_gamma_curve_e eGammaCurve)
{
    return ADAP_NOT_OK;
}

ADAP_STATUS_T ADAP_PQ_GetGamma(SINT32 *pValue)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_SetGammaChannel_R(vpp_gamma_ch_table_s *pData)
{
    if (mPqFd < 0) {
        LOGE("%s mPqFd is not open\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    if (pData == NULL) {
        LOGE("%s Channel_R pData is NULL\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    if (ioctl(mPqFd, VPP_IOC_SET_GAMMA_TABLE_R, pData) < 0) {
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_SetGammaChannel_G(vpp_gamma_ch_table_s *pData)
{
    if (mPqFd < 0) {
        LOGE("%s mPqFd is not open\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    if (pData == NULL) {
        LOGE("%s Channel_G pData is NULL\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    if (ioctl(mPqFd, VPP_IOC_SET_GAMMA_TABLE_G, pData) < 0) {
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_SetGammaChannel_B(vpp_gamma_ch_table_s *pData)
{
    if (mPqFd < 0) {
        LOGE("%s mPqFd is not open\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    if (pData == NULL) {
        LOGE("%s Channel_B pData is NULL\n", __FUNCTION__);
        return ADAP_NOT_OK;
    }

    if (ioctl(mPqFd, VPP_IOC_SET_GAMMA_TABLE_B, pData) < 0) {
        return ADAP_NOT_OK;
    }

    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_SetModuleCtrl(struct vpp_module_ctrl_s *pModuleCtrl)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_MODULE_STATUS, pModuleCtrl);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetModuleCtrl(SINT32 *pValue)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_SetMatrixParam(struct vpp_mtrx_info_s *pMatrixInfo)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_MATRIX_PARAM, pMatrixInfo);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_SetPqState(struct vpp_pq_state_s *pPqState)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_PQ_STATE, pPqState);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetPqState(struct vpp_pq_state_s *pPqState)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_GET_PQ_STATE, pPqState);
    LOGI("%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__, ret,
            pPqState->pq_en, pPqState->pq_cfg.vadj1_en, pPqState->pq_cfg.vd1_ctrst_en,
            pPqState->pq_cfg.vadj2_en, pPqState->pq_cfg.post_ctrst_en, pPqState->pq_cfg.pregamma_en, pPqState->pq_cfg.gamma_en,
            pPqState->pq_cfg.wb_en, pPqState->pq_cfg.dnlp_en, pPqState->pq_cfg.lc_en, pPqState->pq_cfg.black_ext_en,
            pPqState->pq_cfg.chroma_cor_en, pPqState->pq_cfg.sharpness0_en, pPqState->pq_cfg.sharpness1_en, pPqState->pq_cfg.cm_en);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_SetPcMode(enum vpp_pc_mode_e ePcMode)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_PC_MODE, &ePcMode);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetPcMode(enum vpp_pc_mode_e *pPcMode)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_GET_PC_MODE, pPcMode);
    LOGI("%s %d =%d\n", __FUNCTION__, ret, *pPcMode);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_SetDnlpMode(adap_pq_dnlp_mode_e eDnlpMode)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;
    struct vpp_dnlp_curve_param_s stDcParamToDrv;
    memset(&stDcParamToDrv, 0, sizeof(struct vpp_dnlp_curve_param_s));

    eDnlpModeCur = eDnlpMode;

    if (HALPQ_DO_MAPPING) {
        //todo remap different dnlp param by different level
    } else {
        //todo halpq send level to vpp by io
    }

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_DNLP_PARAM, &stDcParamToDrv);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_SetLcMode(adap_pq_lc_mode_e eLcMode)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_SetLcParam(struct vpp_lc_param_s *pLcParam)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_LC_PARAM, pLcParam);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_SetCscType(enum vpp_csc_type_e eCscType)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_CSC_TYPE, &eCscType);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetCscType(enum vpp_csc_type_e *pCscType)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_GET_CSC_TYPE, pCscType);
    LOGI("%s %d %d\n", __FUNCTION__, ret, *pCscType);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_Set3DLutData(SINT32 value)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_SET_3DLUT_DATA, &value);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_Get3DLutData(SINT32 *pValue)
{
    return ADAP_OK;
}

ADAP_STATUS_T ADAP_PQ_GetHdrType(enum vpp_hdr_type_e *pHdrType)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_GET_HDR_TYPE, pHdrType);
    LOGI("%s %d %d\n", __FUNCTION__, ret, *pHdrType);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetColorPrim(enum vpp_color_primary_e *pColorPrim)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_GET_COLOR_PRIM, pColorPrim);
    LOGI("%s %d %d\n", __FUNCTION__, ret, *pColorPrim);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetHdrMetadata(struct vpp_hdr_metadata_s *pHdrMetadata)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_GET_HDR_METADATA, pHdrMetadata);
    LOGI("%s %d\n", __FUNCTION__, ret);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetHistAvg(struct vpp_histgm_ave_s *pHistAve)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_GET_HIST_AVG, pHistAve);
    LOGI("%s %d %d %d %d %d\n", __FUNCTION__, ret, pHistAve->sum, pHistAve->width, pHistAve->height, pHistAve->ave);

    return ret;
}

ADAP_STATUS_T ADAP_PQ_GetHistParam(struct vpp_histgm_param_s *pHistParam)
{
    ADAP_STATUS_T ret = ADAP_NOT_OK;

    ret = ADAP_PQ_DevIoCtl(VPP_IOC_GET_HIST_BIN, pHistParam);
    return ret;
}

ADAP_STATUS_T ADAP_PQ_SetInputSrcTiming(adap_pq_source_timing_e eSrcTiming)
{
    return ADAP_OK;
}

#define PI 3.14159265358979

ADAP_STATUS_T video_set_saturation_hue(signed char saturation, signed char hue, signed long *mab)
{
    signed short ma = (signed short) (cos((float) hue * PI / 128.0) * ((float) saturation / 128.0
                                      + 1.0) * 256.0);
    signed short mb = (signed short) (sin((float) hue * PI / 128.0) * ((float) saturation / 128.0
                                      + 1.0) * 256.0);

    if (ma > 511) {
        ma = 511;
    }

    if (ma < -512) {
        ma = -512;
    }

    if (mb > 511) {
        mb = 511;
    }

    if (mb < -512) {
        mb = -512;
    }

    *mab = ((ma & 0x3ff) << 16) | (mb & 0x3ff);

    return ADAP_OK;
}

