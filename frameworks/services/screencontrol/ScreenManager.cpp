/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "ScreenManager"
//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <android-base/logging.h>

#include <ScreenManager.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaDataBase.h>
#include <OMX_IVCommon.h>
#include <media/hardware/MetadataBufferType.h>

#include <ui/GraphicBuffer.h>
#include <cutils/properties.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/MemoryHeapBase.h>
#include <binder/MemoryBase.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <media/stagefright/MediaBuffer.h>

#include <Media2Ts/tspack.h>

//#include <gui/ISurfaceComposer.h>
#include <OMX_Component.h>

#include <utils/Log.h>
#include <utils/String8.h>

#include <private/gui/ComposerService.h>

#include <ScreenManager.h>

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/videodev2.h>
#include "ScreenControlDebug.h"


namespace android {

#define BOUNDARY 32
#define ALIGN(x) (x + (BOUNDARY) - 1)& ~((BOUNDARY) - 1)

#define MAX_CLIENT 4
#define PORTTYPE_VALUE_VPP0_VIDEO_OSD  0x11000001
#define PORTTYPE_VALUE_VPP0_VIDEO_ONLY 0x11000000
#define PORTTYPE_VALUE_VPP1_VIDEO_ONLY 0x11000002
#define PORTTYPE_VALUE_VPP1_VIDEO_OSD  0x11000003
#define PORTTYPE_VALUE_VPP0_OSD_ONLY   0x11000004

#define SCREENMANAGER_DUMP_BASEDIR "/data/temp/sm-drvin"
#define PERSIST_SYS_ROTATION_PROP "persist.vendor.sys.builtinrotation"

static const int64_t VDIN_MEDIA_SOURCE_TIMEOUT_NS = 3000000000LL;

static void VdinDataCallBack(void *user, aml_screen_buffer_info_t *buffer){
    ScreenManager *source = static_cast<ScreenManager *>(user);
    source->dataCallBack(buffer);
    return;
}

static void microdimming(uint8_t *s, uint8_t *dest,int W,int H, int w_count,int h_count)
{
    ALOGE("[%s %d]", __FUNCTION__, __LINE__);
    int c_width = w_count;
    int c_height = h_count;
    int map = 0;
    int i=0;
    int j=0;
    int m=0;
    int n=0;
    int sum;
    int k;
    uint8_t* d = (uint8_t *)dest;

    int mFrameHeight;
    int mFrameWidth;
    int count_m = 0;
    int pixcount = 0;

    mFrameWidth = W/c_width;
    mFrameHeight = H/c_height;
    memset(dest, 0x00, c_width*c_height);
    ALOGE("[W:%d H:%d,mW:%d mH:%d,c_height:%d,c_width:%d]",W,H, mFrameWidth, mFrameHeight,c_height,c_width);

    for (i = 0; i < c_height; i++) {
            for (j = 0;j < c_width; j++) {
                    sum = 0;
                    pixcount = 0;
                    for (m = 0; m < mFrameHeight ; m = m +4) {
                            for (n = 0; n < mFrameWidth ; n = n +4) {
                                    map = (m + i * mFrameHeight)* W + j * mFrameWidth + n;
                                    k = s[map /8 *8] * 1.1 + 3;
                                    if (k > 255)
                                            k = 255;
                                    sum = sum + k;
                                    pixcount++;
                            }
                    }
                    sum=sum / pixcount;
                    *(d++) = (uint8_t)sum;
                    //memset(d++, (unsigned char)sum, sizeof(unsigned char) );
            }


    }
    //memset(dest+(c_width*c_height), 0x80, (c_width*c_height) / 2);
}

static int getRotationDegree(){
    char prop[PROPERTY_VALUE_MAX];
    if (property_get(PERSIST_SYS_ROTATION_PROP, prop, "0") > 0) {
       ALOGI("start prop =%s",prop);
        char *tmp = NULL;
        long int degree = strtol(prop, &tmp, 0);
        ALOGI("propValue =%ld",degree);
        if (LONG_MIN != degree && LONG_MAX != degree ) {
            return degree;
        }
    }
    return -1;
}

ScreenManager::ScreenManager() :
    mWidth(-1),
    mHeight(-1),
    mSourceType(-1),
    mCurrentTimestamp(0),
    mFrameRate(30),
    mStarted(false),
    mError(false),
    mNumFramesReceived(0),
    mNumFramesEncoded(0),
    mFirstFrameTimestamp(0),
    mStartTimeOffsetUs(0),
    mMaxAcquiredBufferCount(4),  // XXX double-check the default
    mUseAbsoluteTimestamps(false),
    bufferTimeUs(0),
    mCanvasClientExist(0),
    mBufferGet(NULL),
    mCorpX(0),
    mCorpY(0),
    mCorpWidth(0),
    mCorpHeight(0),
    mOutFrameCounter(0),
    mNeedPause(false),
    mIsScreenRecord(false),
    mScreenModule(NULL),
    mScreenDev(NULL),
    mTempBuffer(NULL),
    mMeanWhileFlag(false),
    mMicroWidth(0),
    mMicroHeight(0) {
    mScreenBuffers[0] = NULL;
    mScreenBuffers[1] = NULL;
    mScreenBuffers[2] = NULL;
    mScreenBuffers[3] = NULL;
    mScreenBuffers[4] = NULL;
    mScreenBuffers[5] = NULL;

    mRawBufferQueue.clear();

    ALOGI("[%s %d] Construct", __FUNCTION__, __LINE__);
}

ScreenManager::~ScreenManager() {
    ALOGI("~ScreenManager");
    CHECK(!mStarted);

    stop(1);

}



static int saveBufferAsFile(void *buffer, size_t size, char *file)
{
    int fd = open(file, O_CREAT|O_RDWR, 0666);
    int wrSize = 0;
    int ret = 0;

    if (fd < 0) {
        return fd;
    }

    do {
        int ret = write (fd, (char *)buffer + wrSize, size - wrSize);
        if (ret > 0) {
            wrSize += ret;
        } else {
            break;
        }
    } while (wrSize < size);
    close(fd);
    return ret;
}

static void checkAndSaveBufferToFile(const char *baseFile, char *filename, void *buffer, size_t size)
{
    if (0 == access(baseFile, F_OK)) {
        if (saveBufferAsFile(buffer, size, filename) >= 0) {
            ALOGD("Save file ok: %s", filename);
        } else {
            ALOGW("Save file fail: %s!", filename);
        }
    }
}

void yuv_to_rgb32(unsigned char y,unsigned char u,unsigned char v,unsigned char *rgb)
{
    int r,g,b;

    r = (1192 * (y - 16) + 1634 * (v - 128) ) >> 10;
    g = (1192 * (y - 16) - 833 * (v - 128) - 400 * (u -128) ) >> 10;
    b = (1192 * (y - 16) + 2066 * (u - 128) ) >> 10;

    r = r > 255 ? 255 : r < 0 ? 0 : r;
    g = g > 255 ? 255 : g < 0 ? 0 : g;
    b = b > 255 ? 255 : b < 0 ? 0 : b;

    /*ARGB*/
    *rgb = (unsigned char)r;
    rgb++;
    *rgb = (unsigned char)g;
    rgb++;
    *rgb = (unsigned char)b;
    rgb++;
    *rgb = 0xff;
}

void nv21_to_rgb32_(unsigned char *buf, unsigned char *rgb, int width, int height)
{
    int x,y,z=0;
    int h,w;
    int blocks;
    unsigned char Y1, Y2, U, V;

    blocks = (width * height) * 2;
    for (h=0, z=0; h< height; h+=2) {
        for (y = 0; y < width*2; y+=2) {
            Y1 = buf[ h*width + y + 0];
            V = buf[ blocks/2 + h*width/2 + y%width + 0 ];
            Y2 = buf[ h*width + y + 1];
            U = buf[ blocks/2 + h*width/2 + y%width + 1 ];
            yuv_to_rgb32(Y1, U, V, &rgb[z]);
            yuv_to_rgb32(Y2, U, V, &rgb[z + 4]);
            z+=8;
        }
    }
}


ScreenManager* ScreenManager::instantiate() {
    ScreenManager *mScreenControl = new ScreenManager();
    return mScreenControl;
}

bool ScreenManager::isHaveOutputData(){
    Mutex::Autolock lock(mLock);

    if (mRawBufferQueue.size() > 0)
      return true;
    return false;
}

void ScreenManager::setPauseMode(bool isPause){
    mNeedPause=isPause;
}

status_t ScreenManager::init(int32_t width,int32_t height,
                            int32_t source_type,int32_t framerate,
                            SCREENCONTROLDATATYPE data_type,
                            int32_t* client_id) {
    Mutex::Autolock autoLock(mLock);
    int clientTotalNum;
    int clientNum = -1;
    clientTotalNum = mClientList.size();
    ScreenControlDebug::initDebug();
    ALOGI("[%s %d] clientTotalNum:%d width:%d height:%d framerate:%d data_type:%d", __FUNCTION__, __LINE__,
             clientTotalNum, width, height, framerate, data_type);

    if (clientTotalNum >= MAX_CLIENT) {
        ALOGE("[%s %d] clientTotalNum:%d ", __FUNCTION__, __LINE__, clientTotalNum);
        return !OK;
    }

    ScreenClient* Client_tmp = (ScreenClient*)malloc(sizeof(ScreenClient));
    if (Client_tmp == NULL ) {
        ALOGE("[%s %d] malloc ScreenClient error! ", __FUNCTION__, __LINE__);
        return !OK;
    }
    Client_tmp->width = width;
    Client_tmp->height = height;
    Client_tmp->framerate = framerate;
    Client_tmp->isPrimateClient = 0;
    Client_tmp->data_type = data_type;

    if (clientTotalNum == 0) {
        clientNum = 1;
    } else {
        ScreenClient* client_temp;
        for (int i = 0; i < clientTotalNum; i++) {
            client_temp = mClientList.valueAt(i);
            if (client_temp->mClient_id != i + 1) {
                clientNum = i + 1;
        }
    }

    if (clientNum == -1)
        clientNum = clientTotalNum + 1;
    }

    ALOGI("[%s %d] clientNum:%d clientTotalNum:%d", __FUNCTION__, __LINE__,  clientNum, clientTotalNum);

    Client_tmp->mClient_id = clientNum;
    *client_id = clientNum;

    if (SCREENCONTROL_CANVAS_TYPE == data_type) {
        int client_num = 0;
        client_num = mClientList.size();
        ScreenClient* client_local;

        for (int i = 0; i < client_num; i++) {
            client_local = mClientList.valueAt(i);
            if (client_local->data_type == SCREENCONTROL_CANVAS_TYPE) {
                ALOGE("[%s %d] screen source owned canvas client already, so reject another canvas client", __FUNCTION__, __LINE__);
                free(Client_tmp);
                return !OK;
            }
        }

        mWidth = width;
        mHeight = height;
        mSourceType = source_type;
        mFrameRate = framerate;

    } else if (SCREENCONTROL_RAWDATA_TYPE == data_type || SCREENCONTROL_RGBA888_TYPE == data_type ||
                SCREENCONTROL_MICRODIM_TYPE == data_type) {
        ALOGI("[%s %d] clientTotalNum:%d width:%d height:%d framerate:%d data_type:%d", __FUNCTION__, __LINE__,
            clientTotalNum, width, height, framerate, data_type);
        if (clientTotalNum == 0) {
            mWidth = width;
            mHeight = height;
            mSourceType = source_type;
            mFrameRate = framerate;
        }
    }
    ALOGI("[%s %d] clientNum:%d", __FUNCTION__, __LINE__, clientNum);
    mClientList.add(clientNum, Client_tmp);

    return OK;
}

status_t ScreenManager::uninit(int32_t client_id) {
    ALOGI("[%s %d] client_id:%d", __FUNCTION__, __LINE__, client_id);
    Mutex::Autolock autoLock(mLock);

    ScreenClient* client;
    client = mClientList.valueFor(client_id);
    mClientList.removeItem(client_id);

    free(client);
    return OK;
}

nsecs_t ScreenManager::getTimestamp() {
    ALOGI("[%s %d]", __FUNCTION__, __LINE__);
    Mutex::Autolock autoLock(mLock);
    return mCurrentTimestamp;
}

bool ScreenManager::isMetaDataStoredInVideoBuffers() const {
    ALOGI("[%s %d]", __FUNCTION__, __LINE__);
    return true;
}

int32_t ScreenManager::getFrameRate( )
{
    ALOGI("[%s %d]", __FUNCTION__, __LINE__);
    Mutex::Autolock autoLock(mLock);
    return mFrameRate;
}

status_t ScreenManager::setVideoRotation(int degree)
{
    int angle;

    ALOGI("[%s %d] setVideoRotation degree:%x", __FUNCTION__, __LINE__, degree);

    if (degree == 0)
        angle = 0;
    else if (degree == 1)
        angle = 270;
    else if (degree == 2)
        angle = 180;
    else if (degree == 3)
        angle = 90;
    else {
        ALOGE("degree is not right");
        return !OK;
    }

    if (mScreenDev != NULL) {
        ALOGI("[%s %d] setVideoRotation angle:%d", __FUNCTION__, __LINE__, angle);
        mScreenDev->ops.set_rotation(mScreenDev, angle);
    }

    return OK;
}

status_t ScreenManager::setVideoCrop(const int32_t x, const int32_t y, const int32_t width, const int32_t height)
{
    ALOGI("[%s %d] setVideoCrop x:%d y:%d width:%d height:%d", __FUNCTION__, __LINE__, x, y, width, height);
    Mutex::Autolock autoLock(mLock);
    mCorpX = x;
    mCorpY = y;
    mCorpWidth = width;
    mCorpHeight = height;
    return OK;
}

status_t ScreenManager::start(int32_t client_id, int flag )
{
    Mutex::Autolock autoLock(mLock);

    int client_num = mClientList.size();
    SCREENCONTROLDATATYPE source_data_type;
    ScreenClient* client;
    bool isSoftwareEncoder = false;
    bool mIsScreenRecord = true;
    client = mClientList.valueFor(client_id);
    source_data_type = client->data_type;
    if (!mScreenModule || client_num == 1) {
        if (hw_get_module(AML_SCREEN_HARDWARE_MODULE_ID, (const hw_module_t **)&mScreenModule) < 0) {
            ALOGE("[%s %d] can`t get AML_SCREEN_HARDWARE_MODULE_ID module", __FUNCTION__, __LINE__);
            return !OK;
        }
        char sourceType[] = "1";
        int port_type;
        if (mSourceType == AML_CAPTURE_VIDEO) { //video only
            port_type = PORTTYPE_VALUE_VPP0_VIDEO_ONLY;
        } else if(mSourceType == AML_CAPTURE_OSD_VIDEO) {
            port_type = PORTTYPE_VALUE_VPP0_VIDEO_OSD;
        } else if(mSourceType == AML_CAPTURE_OSD_ONLY) {
            port_type = PORTTYPE_VALUE_VPP0_OSD_ONLY;
        } else {
            ALOGE("[%s %d] For now ,we don't capture osd only by AML_SCREEN_HARDWARE_MODULE_ID module!", __FUNCTION__, __LINE__);
            return !OK;
        }

        ALOGI("[%s %d] sourcetype=%s, port_type=%#x(%s)", __FUNCTION__, __LINE__, sourceType,
        port_type, (PORTTYPE_VALUE_VPP0_VIDEO_ONLY==port_type?"video only":(PORTTYPE_VALUE_VPP0_VIDEO_OSD==port_type?"video+osd":"osd only")));

        if (mScreenModule->common.methods->open((const hw_module_t *)mScreenModule, sourceType,
                (struct hw_device_t**)&mScreenDev) < 0) {
            mScreenModule = NULL;
            ALOGE("[%s %d] open AML_SCREEN_SOURCE fail", __FUNCTION__, __LINE__);
            return !OK;
        }

        ALOGI("[%s %d] start AML_SCREEN_SOURCE", __FUNCTION__, __LINE__);
        int degree = getRotationDegree();
        if ( degree > 0) {
            setVideoRotation(degree);
            if (degree == 90 || degree == 270) {
                int temp = mCorpWidth-mCorpX;
                mCorpWidth = mCorpHeight-mCorpY;
                mCorpHeight = temp;
            }

        }
        mScreenDev->ops.set_port_type(mScreenDev, port_type);
        mScreenDev->ops.set_frame_rate(mScreenDev, mFrameRate);
        if (flag & SCREENCONTROL_SCREEN_CATCH) {
            mIsScreenRecord =false;
        }else if (flag & SCREENCONTROL_SCREEN_RECORD_SOFTWARE_ENCODER)
            isSoftwareEncoder =true;

        if (mIsScreenRecord) {
            mScreenDev->ops.set_mode(mScreenDev, AML_SCREEN_RECODE_MODE);
        }else
            mScreenDev->ops.set_mode(mScreenDev, AML_SCREEN_CATCH_MODE);

        if (isSoftwareEncoder && mIsScreenRecord) {
            mScreenDev->ops.set_format(mScreenDev, mWidth, mHeight, V4L2_PIX_FMT_NV12);
        } else if (SCREENCONTROL_RGBA888_TYPE == source_data_type) {
            mScreenDev->ops.set_format(mScreenDev, mWidth, mHeight, V4L2_PIX_FMT_RGB32);
        }else {
            mScreenDev->ops.set_format(mScreenDev, mWidth, mHeight, V4L2_PIX_FMT_NV21);
        }
        mScreenDev->ops.setDataCallBack(mScreenDev, VdinDataCallBack, (void*)this);
        mScreenDev->ops.set_amlvideo2_crop(mScreenDev, mCorpX, mCorpY, mCorpWidth-mCorpX, mCorpHeight-mCorpY);
        mScreenDev->ops.start(mScreenDev);
        mScreenDev->ops.get_all_ptr(mScreenDev,mScreenBuffers);
    }


    ALOGI("[%s %d] client_id:%d client_num:%d", __FUNCTION__, __LINE__, client_id, client_num);
#if 0
    if (SCREENCONTROL_HANDLE_TYPE == source_data_type && mANativeWindow != NULL) {
        mANativeWindow->incStrong((void*)ANativeWindow_acquire);
    }
#endif
    if (SCREENCONTROL_CANVAS_TYPE == source_data_type) {
        mCanvasClientExist = 1;
    }

    mStartTimeOffsetUs = 0;
    mNumFramesReceived = mNumFramesEncoded = 0;

    mStarted = true;

    return OK;
}

status_t ScreenManager::setMaxAcquiredBufferCount(size_t count) {
    ALOGI("setMaxAcquiredBufferCount(%d)", count);
    Mutex::Autolock autoLock(mLock);
    mMaxAcquiredBufferCount = count;
    return OK;
}

status_t ScreenManager::setUseAbsoluteTimestamps() {
    ALOGI("[%s %d]", __FUNCTION__, __LINE__);
    Mutex::Autolock autoLock(mLock);
    mUseAbsoluteTimestamps = true;

    return OK;
}

status_t ScreenManager::stop(int32_t client_id)
{
    Mutex::Autolock autoLock(mLock);

    int client_num = mClientList.size();
    FrameBufferInfo* frame = NULL;
    ALOGI("[%s %d] client_num:%d client_id:%d", __FUNCTION__, __LINE__, client_num, client_id);

    if (!mStarted) {
        ALOGE("ScreenSource::stop X Do nothing");
        return OK;
    }
    if (mScreenDev)
        mScreenDev->ops.stop(mScreenDev);

    {
        mFrameAvailableCondition.signal();
        while (!mCanvasFramesReceived.empty()) {
            frame = *mCanvasFramesReceived.begin();
            mCanvasFramesReceived.erase(mCanvasFramesReceived.begin());
            if (frame != NULL)
                delete frame;
        }
    }


    SCREENCONTROLDATATYPE source_data_type;
    ScreenClient* client;
    client = mClientList.valueFor(client_id);
    source_data_type = client->data_type;

    if (SCREENCONTROL_CANVAS_TYPE == source_data_type)
        mCanvasClientExist = 0;
    mOutFrameCounter = 0;
#if 0
    if (SCREENCONTROL_HANDLE_TYPE == source_data_type && mANativeWindow != NULL) {
        mANativeWindow->decStrong((void*)ANativeWindow_acquire);
    }
#endif
    if (SCREENCONTROL_RAWDATA_TYPE == source_data_type ||SCREENCONTROL_RGBA888_TYPE == source_data_type) {
        while (!mRawBufferQueue.empty()) {
            int index = *mRawBufferQueue.begin();
            mRawBufferQueue.erase(mRawBufferQueue.begin());
        }
        if (mTempBuffer)
            free(mTempBuffer);
    }else if (SCREENCONTROL_MICRODIM_TYPE == source_data_type) {
        while (!mMicroBufferQueue.empty()) {
            uint8_t* rawBuffer = *mMicroBufferQueue.begin();
            mMicroBufferQueue.erase(mMicroBufferQueue.begin());
            if (rawBuffer != NULL)
                free(rawBuffer);
        }
        mMicroWidth = 0;
        mMicroHeight = 0;
    }

     if (mScreenDev)
        mScreenDev->common.close((struct hw_device_t *)mScreenDev);

    mScreenModule = NULL;
    mStarted = false;
    ALOGI("ScreenSource::stop done");

    return OK;
}

status_t ScreenManager::readRawData(int32_t client_id,void **buffer) {
    Mutex::Autolock autoLock(mLock);
    // ALOGI("[%s %d] in ", __FUNCTION__, __LINE__);
    ScreenClient* client;
    FrameBufferInfo* frame = NULL;
    SCREENCONTROLDATATYPE source_data_type;
    client = mClientList.valueFor(client_id);
    source_data_type = client->data_type;
    if (mTempBuffer == NULL) {
        mMeanWhileFlag = true;
        return !OK;
    }
    *buffer = mTempBuffer;
    mMeanWhileFlag = false;
    mTempBuffer = NULL;
    ALOGI("[%s %d] ok", __FUNCTION__, __LINE__);
    return OK;
}

status_t ScreenManager::readBuffer(int32_t client_id, sp<IMemory> buffer, int *index)
{
    Mutex::Autolock autoLock(mLock);

    long buff_info[3] = {0,0,0};
    int ret = 0;
    int count = 0;
    FrameBufferInfo* frame = NULL;
    SCREENCONTROLDATATYPE source_data_type;

    ScreenClient* client;
    client = mClientList.valueFor(client_id);
    source_data_type = client->data_type;

    if (!mStarted ) {
        ALOGE("[%s %d]", __FUNCTION__, __LINE__);
        return !OK;
    }

    if (SCREENCONTROL_CANVAS_TYPE == source_data_type) {
        if (mCanvasFramesReceived.empty())
            return !OK;

        frame = *mCanvasFramesReceived.begin();
        mCanvasFramesReceived.erase(mCanvasFramesReceived.begin());
        if (!frame)
            return !OK;


        buff_info[0] = kMetadataBufferTypeCanvasSource;
        buff_info[1] = (long)frame->buf_ptr;
        buff_info[2] = (long)frame->canvas;
        if (buffer->unsecurePointer() == NULL)
            return !OK;
        memcpy((long *)buffer->unsecurePointer(), &buff_info[0],sizeof(buff_info));

        if (ScreenControlDebug::canDebug()) {
            // dump buffer to file
            static int i = 0;
            char filename[64] = {0};
            const char *dump_path = SCREENMANAGER_DUMP_BASEDIR;
            snprintf(filename, 64, "%s/drvin-cvs-%d.yuv", dump_path, i++);
            checkAndSaveBufferToFile(dump_path, filename, frame->buf_ptr, mWidth*mHeight*3/2);
        }


        ALOGI("[%s %d] buf_ptr:%p canvas:%x  size:%d OK:%d", __FUNCTION__, __LINE__,
                frame->buf_ptr, frame->canvas, mCanvasFramesReceived.size(), OK);

        delete frame;
        return OK;

    }

    if ((SCREENCONTROL_RAWDATA_TYPE == source_data_type || SCREENCONTROL_RGBA888_TYPE == source_data_type) && !mRawBufferQueue.empty()) {
        int index_ = *mRawBufferQueue.begin();
        mRawBufferQueue.erase(mRawBufferQueue.begin());
        *index = index_;
        if (ScreenControlDebug::canDebug()) {
            // dump buffer to file
            static int i = 0;
            char filename[64] = {0};
            int size = mWidth*mHeight*3/2;
            const char *dump_path = SCREENMANAGER_DUMP_BASEDIR;
            snprintf(filename, 64, "%s/drvin-rd-%d.bin", dump_path, i++);
            if (SCREENCONTROL_RGBA888_TYPE == source_data_type)
                size = mWidth*mHeight*4;
            checkAndSaveBufferToFile(dump_path, filename, mScreenBuffers[index_], size);
        }
    }else if (SCREENCONTROL_MICRODIM_TYPE == source_data_type && !mMicroBufferQueue.empty()){
        uint8_t* microBuffer = *mMicroBufferQueue.begin();
        mMicroBufferQueue.erase(mMicroBufferQueue.begin());
        if (microBuffer != NULL && buffer != NULL) {
            memmove((char *)buffer->unsecurePointer(), microBuffer, mMicroWidth*mMicroHeight);
            free(microBuffer);
        }else{
            ALOGE("[%s] microBuffer invalid data(null)", __func__);
            return !OK;
        }


    } else {
        //ALOGE("[%s %d] read raw data fail", __FUNCTION__, __LINE__);
        return !OK;
    }

    delete frame;
    mOutFrameCounter++;
    return OK;
}

status_t ScreenManager::getBufferByID(int32_t index,long **buffer) {
    *buffer = mScreenBuffers[index];
    return OK;
}

status_t ScreenManager::checkConvertDone(){
    Mutex::Autolock autoLock(mLock);
    if (mOutFrameCounter > 0)
      return OK;
    return !OK;
}

status_t ScreenManager::freeBuffer(int32_t client_id, sp<IMemory>buffer) {

    Mutex::Autolock autoLock(mLock);

    if (mStarted == false)
        return OK;

    SCREENCONTROLDATATYPE source_data_type;
    ScreenClient* client;
    client = mClientList.valueFor(client_id);
    source_data_type = client->data_type;

    if (buffer->unsecurePointer() != NULL) {
        long buff_info[3] = {0,0,0};
        memcpy(&buff_info[0],(long *)buffer->unsecurePointer(), sizeof(buff_info));
        if (mTempBuffer == (long *)buff_info[1])
            mTempBuffer = NULL;
        if (mScreenDev)
            mScreenDev->ops.release_buffer(mScreenDev, (long *)buff_info[1]);
    }

    ++mNumFramesEncoded;

    return OK;
}

void ScreenManager::setMicroSize(int32_t width, int32_t height) {
    Mutex::Autolock autoLock(mLock);
    mMicroWidth = width;
    mMicroHeight = height;

}

int ScreenManager::dataCallBack(aml_screen_buffer_info_t *buffer){
    int ret = NO_ERROR;
    long buff_info[3] = {0,0,0};
    int status = OK;
    ANativeWindowBuffer* buf;
    void *src = NULL;
    void *dest = NULL;

    if ((mStarted) && (mError == false)) {
        if (buffer == NULL || (buffer->buffer_mem == 0)) {
            return BAD_VALUE;
        }
        if (buffer->buffer_canvas == 0) {
            mError = true;
            ALOGE("Could get canvas info from device!");
            return BAD_VALUE;
        }


        ++mNumFramesReceived;
        {
            Mutex::Autolock autoLock(mLock);

            int client_num = 0;
            client_num = mClientList.size();
            ScreenClient* client;

            //first, process hdmi and screencatch.
            for (int i = 0; i < client_num; i++) {
                client = mClientList.valueAt(i);
                switch (client->data_type) {
                    case SCREENCONTROL_RAWDATA_TYPE:{
                        if (!mNeedPause) {
                            ALOGD("dataCallBack index =%d",buffer->index);
                            mRawBufferQueue.push_back(buffer->index);
                            if (mMeanWhileFlag && mTempBuffer == NULL) {
                                mTempBuffer = (long*) malloc(client->width*client->height*3/2);
                                if (mTempBuffer != NULL) {
                                    memmove(mTempBuffer, buffer->buffer_mem, client->width*client->height*3/2);
                                }
                            }
                        }
                    } break;
                    case SCREENCONTROL_RGBA888_TYPE:{
                        if (!mNeedPause) {
                            ALOGD("dataCallBack index =%d",buffer->index);
                            mRawBufferQueue.push_back(buffer->index);
                        }
                    } break;
                    case SCREENCONTROL_MICRODIM_TYPE:{
                        if (mMicroBufferQueue.size() < 2) {
                            if (mMicroHeight > 0 && mMicroWidth > 0) {
                                uint8_t *temp = (uint8_t *)malloc(mMicroWidth*mMicroHeight);
                                microdimming((uint8_t *)buffer->buffer_mem,temp,client->width,client->height,mMicroWidth,mMicroHeight);
                                mMicroBufferQueue.push_back(temp);
                            }else{
                                ALOGE("datacallback error: mMicroHeight < 0");
                            }


                        }
                        if (mScreenDev)
                            mScreenDev->ops.release_buffer(mScreenDev, buffer->buffer_mem);
                    } break;
                    default:{
                        if (mCanvasClientExist == 0) {//release buffer
                            mScreenDev->ops.release_buffer(mScreenDev, buffer->buffer_mem);
                        }
                    }
                }
            }

           //second, process canvas
            if (mCanvasClientExist == 1) {
                for (int i = 0; i < client_num; i++) {
                    client = mClientList.valueAt(i);
                    if (client->data_type == SCREENCONTROL_CANVAS_TYPE) {
                        buff_info[0] = kMetadataBufferTypeCanvasSource;
                        buff_info[1] = (long)buffer->buffer_mem;
                        buff_info[2] = buffer->buffer_canvas;

                        FrameBufferInfo* frame = new FrameBufferInfo;

                        frame->buf_ptr = buffer->buffer_mem;
                        frame->canvas = buffer->buffer_canvas;
                        frame->timestampUs = 0;
                        mCanvasFramesReceived.push_back(frame);
                        mCanvasClientExist = 1;
                        if (mMeanWhileFlag && mTempBuffer == NULL) {
                            mTempBuffer = buffer->buffer_mem;
                        }
                    }
                }
            }
            mFrameAvailableCondition.signal();
        }
    }
    return ret;
}


}; // namespace android
