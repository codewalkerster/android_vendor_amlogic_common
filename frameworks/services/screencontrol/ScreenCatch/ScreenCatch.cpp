/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_NDEBUG 1
#define LOG_TAG "ScreenCatch"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaDataBase.h>
#include <OMX_IVCommon.h>
#include <media/hardware/MetadataBufferType.h>

#include <ui/GraphicBuffer.h>
#include <OMX_Component.h>
#include <cutils/properties.h>

#include <utils/Log.h>
#include <utils/String8.h>

#include "ScreenCatch.h"
#include "../ScreenControlDebug.h"
#include "am_gralloc_ext.h"

#include <binder/IPCThreadState.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <binder/IServiceManager.h>

#define BOUNDARY 32

#define ALIGN(x) (x + (BOUNDARY) - 1)& ~((BOUNDARY) - 1)

namespace android {


//////////////////////////////  screen capture when use keystone  //////////////////////////////

static int32_t gralloc_unref_dma_buf(native_handle_t * hnd) {
    static GraphicBufferMapper & maper = GraphicBufferMapper::get();

    bool bfreed = false;
    if (am_gralloc_is_valid_graphic_buffer(hnd)) {
        if (NO_ERROR == maper.freeBuffer(hnd)) {
            bfreed = true;
        }
    }

    if (bfreed == false) {
        /*may be we got handle not alloc by gralloc*/
        native_handle_close(hnd);
        native_handle_delete(hnd);
    }

    return 0;
}

static int32_t gralloc_lock_dma_buf(
    native_handle_t * handle, void** vaddr) {
    static GraphicBufferMapper & maper = GraphicBufferMapper::get();
    uint32_t usage = GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN;
    int w = am_gralloc_get_width(handle);
    int h = am_gralloc_get_height(handle);

    Rect r(w, h);
    if (NO_ERROR == maper.lock(handle, usage, r, vaddr))
        return 0;

    ALOGE("lock buffer failed\n");
    return -EINVAL;
}

static int32_t gralloc_unlock_dma_buf(native_handle_t * handle) {
    static GraphicBufferMapper & maper = GraphicBufferMapper::get();
    if (NO_ERROR == maper.unlock(handle))
        return 0;
    return -EINVAL;
}

static inline void rgb24_to_rgb32(unsigned char *src, unsigned char *dist, int srcWidth, int srcHeight)
{
    int srcIdx = 0, dstIdx = 0;
    int size = srcWidth * srcHeight * 3;
    for (;srcIdx < size; srcIdx+=3, dstIdx+=4) {
        memmove(&dist[dstIdx], &src[srcIdx], 3);
        dist[dstIdx+4] = 0xff;
    }
}

ScreenCatch::ScreenCatch(uint32_t bufferWidth, uint32_t bufferHeight, uint32_t bitSize, uint32_t type) :
    /*mWidth(ALIGN(bufferWidth)),*/
    mWidth(bufferWidth),
    mHeight(bufferHeight),
    mType(type),
    mUseKeystone(false),
    mScreenManager(NULL),
    mColorFormat(OMX_COLOR_Format32bitARGB8888),
    mStart(false),
    mThread(NULL),
    mClientId(-1) {
    ALOGI("ScreenCatch: %dx%d", bufferWidth, bufferHeight);

    if (bufferWidth <= 0 || bufferHeight <= 0 || bufferWidth > 1920 || bufferHeight > 1080) {
        ALOGE("Invalid dimensions %dx%d", bufferWidth, bufferHeight);
    }

    mCorpX = -1;
    mCorpY = -1;
    mCorpWidth = -1;
    mCorpHeight = -1;
    mRawBufferQueue.clear();
    ScreenControlDebug::initDebug();
}

ScreenCatch::~ScreenCatch() {
    ALOGI("~ScreenCatch");
}


void ScreenCatch::setVideoCrop(int x, int y, int width, int height)
{
    mCorpX = x;
    mCorpY = y;
    mCorpWidth = width;
    mCorpHeight = height;
}

static inline void yuv_to_rgb32(unsigned char y,unsigned char u,unsigned char v,unsigned char *rgb)
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

void nv21_to_rgb32(unsigned char *buf, unsigned char *rgb, int width, int height)
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

static inline void yuv_to_rgb24(unsigned char y,unsigned char u,unsigned char v,unsigned char *rgb)
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
}

void nv21_to_rgb24(unsigned char *buf, unsigned char *rgb, int width, int height)
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

            yuv_to_rgb24(Y1, U, V, &rgb[z]);
            yuv_to_rgb24(Y2, U, V, &rgb[z + 3]);
            z+=6;
        }
    }
}


int ScreenCatch::threadFuncForScreenManager()
{
    int64_t pts;
    int status;

    sp<MemoryHeapBase> newMemoryHeap = new MemoryHeapBase(mWidth*mHeight*3/2);
    sp<MemoryBase> buffer = new MemoryBase(newMemoryHeap, 0, mWidth*mHeight*3/2);
    if (buffer->unsecurePointer() == NULL ) {
        ALOGE("[%s %d] ,can't malloc memory !", __FUNCTION__, __LINE__);
        return -1;
    }

    ALOGI("[%s %d] empty:%d", __FUNCTION__, __LINE__, mRawBufferQueue.empty());

    while (mStart == true) {
        status = mScreenManager->readBuffer(mClientId, buffer, &pts);

        if (status != OK && mStart == true) {
            usleep(100);
            continue;
        }

        if (mStart != true)
            break;

        {
            Mutex::Autolock autoLock(mLock);
            MediaBuffer* accessUnit = NULL;

            if (OMX_COLOR_Format24bitRGB888 == mColorFormat) {//rgb 24bit
                accessUnit = new MediaBuffer(mWidth*mHeight*3);
                if (accessUnit != NULL && accessUnit->data() != NULL) {
                    nv21_to_rgb24((unsigned char *)buffer->unsecurePointer(), (unsigned char *)accessUnit->data(), mWidth, mHeight);
                    accessUnit->set_range(0, mWidth*mHeight*3);
                }
            } else if (OMX_COLOR_Format32bitARGB8888 == mColorFormat) {//rgba 32bit
                accessUnit = new MediaBuffer(mWidth*mHeight*4);
                if (accessUnit != NULL && accessUnit->data() != NULL) {
                    nv21_to_rgb32((unsigned char *)buffer->unsecurePointer(), (unsigned char *)accessUnit->data(), mWidth, mHeight);
                    accessUnit->set_range(0, mWidth*mHeight*4);
                }
            } else if (OMX_COLOR_FormatYUV420SemiPlanar ==  mColorFormat){//nv21
                accessUnit = new MediaBuffer(mWidth*mHeight*3/2);
                if (accessUnit != NULL && accessUnit->data() != NULL) {
                    memcpy((unsigned char *)accessUnit->data(), (unsigned char *)buffer->unsecurePointer(), mWidth*mHeight*3/2);
                    accessUnit->set_range(0, mWidth*mHeight*3/2);
                }
            }

            if (accessUnit != NULL)
                mRawBufferQueue.push_back(accessUnit);
        }
    }

    //buffer->decStrong(this);
    buffer.clear();
    //newMemoryHeap->decStrong(this);
    newMemoryHeap.clear();

    return 0;
}

int ScreenCatch::threadFunc()
{
    int result = 0;
    if (!mUseKeystone) {
        result = threadFuncForScreenManager();
    }
    return result;
}

void *ScreenCatch::ThreadWrapper(void *me) {
    ScreenCatch *Convertor = static_cast<ScreenCatch *>(me);
    Convertor->threadFunc();
    return NULL;
}



status_t ScreenCatch::start(MetaDataBase *params)
{
    ALOGI("[%s %d] mWidth:%d mHeight:%d", __FUNCTION__, __LINE__, mWidth, mHeight);
    Mutex::Autolock autoLock(mLock);

    status_t status = 0;
    int64_t pts;
    int client_id = -1;
    char postprocessor[8] = {0};
    char keystone[256] = {0};


    if (property_get(PROP_KEYSTONE, keystone, "") > 0 && strlen(keystone) > 0) {
        //(PROP_KEYSTONE not empty)
        mUseKeystone = true;
    }
    ALOGI("Screencatch source from [%s]", mUseKeystone?"DisplayAdapter":"ScreenManager");

    if (mUseKeystone) {
        const native_handle_t *outBufferHandle = nullptr;
        native_handle_t *bufferHandle = nullptr;
        int width=0, height=0, format=0, stride=0;
        std::unique_ptr<meson::DisplayAdapter> displayAdapter = meson::DisplayAdapterCreateRemote();
        if (!displayAdapter) {
            ALOGE("DisplayAdapter init failed");
            return !OK;
        }
        if ((displayAdapter->captureDisplayScreen(&outBufferHandle))
                && (NULL != outBufferHandle)) {
            MediaBuffer* accessUnit = NULL;
            size_t bufSize = 0;
            void* mapBase = nullptr;
            bufferHandle = const_cast<native_handle_t*> (outBufferHandle);

            // get information
            width = am_gralloc_get_width(bufferHandle);
            height = am_gralloc_get_height(bufferHandle);
            format = am_gralloc_get_format(bufferHandle);
            stride = am_gralloc_get_stride_in_pixel(bufferHandle);
            bufSize = stride * height * bytesPerPixel(format);
            ALOGD("[%s %d]mDisplayAdapter get width=%d, height=%d, format=%d, stride=%d, bufSize=%d",
                __func__, __LINE__, width, height, format, stride, bufSize);


            if (!gralloc_lock_dma_buf(bufferHandle, &mapBase)) {
                int unitSize = 0;
                switch (mColorFormat) {  // app needed
                case OMX_COLOR_Format24bitRGB888:
                    unitSize = stride * height*3;
                    accessUnit = new MediaBuffer(unitSize);
                    accessUnit->set_range(0, unitSize);
                    if (PIXEL_FORMAT_RGB_888 == format && accessUnit ->data() != NULL) {
                        memcpy(accessUnit->data(), mapBase, bufSize); //HAL_PIXEL_FORMAT_RGB_888
                    }
                    break;
                case OMX_COLOR_Format32bitARGB8888:
                    unitSize = stride * height*4;
                    accessUnit = new MediaBuffer(unitSize);
                    accessUnit->set_range(0, unitSize);
                    if (PIXEL_FORMAT_RGB_888 == format && accessUnit ->data() != NULL) {
                        ALOGD("format rgb888, call rgb24_to_rgb32\n");
                        rgb24_to_rgb32((unsigned char*)mapBase,
                            (unsigned char*)accessUnit->data(), width, height);
                    }
                    break;

                default:
                    break;
                }

                gralloc_unlock_dma_buf(bufferHandle);
                gralloc_unref_dma_buf(bufferHandle);

                if (accessUnit != NULL) {
                    mRawBufferQueue.push_back(accessUnit);
                }
                return OK;
            } else  {
                ALOGE("lock mem failed");
                return !OK;
            }

        }else {
            ALOGE("captureDisplayScreen failed");
            return !OK;

        }

    } else {
        mScreenManager = ScreenManager::instantiate();
        ALOGI("[%s %d] mWidth:%d mHeight:%d", __FUNCTION__, __LINE__, mWidth, mHeight);

        mScreenManager->init(mWidth, mHeight, mType, 1, SCREENCONTROL_RAWDATA_TYPE, &client_id);

        ALOGI("[%s %d] client_id:%d, mType:%d", __FUNCTION__, __LINE__, client_id, mType);

        mClientId = client_id;

        if (status != OK) {
            ALOGE("setResolutionRatio fail");
            return !OK;
        }

        ALOGI("[%s %d] mCorpX:%d mCorpY:%d mCorpWidth:%d mCorpHeight:%d", __FUNCTION__, __LINE__,  mCorpX, mCorpY, mCorpWidth, mCorpHeight);

        if (mCorpX != -1)
            mScreenManager->setVideoCrop(client_id, mCorpX, mCorpY, mCorpWidth, mCorpHeight);

        status = mScreenManager->start(client_id);

        if (status != OK) {
            mScreenManager->uninit(mClientId);
            ALOGE("ScreenControlService start fail");
            return !OK;
        }

    }


    if (!(params->findInt32(kKeyColorFormat, &mColorFormat)
           && (mColorFormat != OMX_COLOR_FormatYUV420SemiPlanar
            && mColorFormat != OMX_COLOR_Format24bitRGB888
            && mColorFormat != OMX_COLOR_Format32bitARGB8888)))
        mColorFormat = OMX_COLOR_Format32bitARGB8888;
    mStart = true;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&mThread, &attr, ThreadWrapper, this);
    pthread_attr_destroy(&attr);



    ALOGD("[%s %d]", __FUNCTION__, __LINE__);
    return OK;
}

status_t ScreenCatch::stop()
{
    ALOGI("[%s %d]", __FUNCTION__, __LINE__);
    status_t ret = OK;
    Mutex::Autolock autoLock(mLock);
    mStart = false;
    void *dummy;
    pthread_join(mThread, &dummy);
    ALOGI("[%s %d]", __FUNCTION__, __LINE__);
    ret = static_cast<status_t>(reinterpret_cast<uintptr_t>(dummy));
    ALOGI("[%s %d], ret = %d", __FUNCTION__, __LINE__, ret);

    while (!mRawBufferQueue.empty()) {
		    ALOGE("[%s %d] free buffer", __FUNCTION__, __LINE__);
        MediaBuffer* rawBuffer = *mRawBufferQueue.begin();
        mRawBufferQueue.erase(mRawBufferQueue.begin());
        if (rawBuffer != NULL)
            rawBuffer->release();
    }
    if (!mUseKeystone) {
        mScreenManager->stop(mClientId);
        mScreenManager->uninit(mClientId);
    }

    return ret;
}

status_t ScreenCatch::read(MediaBuffer **buffer)
{
    Mutex::Autolock autoLock(mLock);

    if (!mRawBufferQueue.empty()) {
        MediaBuffer* rawBuffer = *mRawBufferQueue.begin();
        mRawBufferQueue.erase(mRawBufferQueue.begin());
        *buffer = rawBuffer;
        return OK;
    }

    return !OK;
}

} // end of namespace android
