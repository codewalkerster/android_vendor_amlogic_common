/** @file ScreenControlService.cpp
 *  @par Copyright:
 *  - Copyright 2011 Amlogic Inc as unpublished work
 *  All Rights Reserved
 *  - The information contained herein is the confidential property
 *  of Amlogic.  The use, copying, transfer or disclosure of such information
 *  is prohibited except by express written agreement with Amlogic Inc.
 *  @author   liangzhuo xie
 *  @version  1.0
 *  @date     2018/08/18
 *  @par function description:
 *  - screen capture
 *  - screen record
 *  @warning This class may explode in your face.
 *  @note If you inherit anything from this class, you're doomed.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "ScreenControlService"

#include "ScreenControlService.h"
#include <stdlib.h>
#include <string.h>
#include <utils/Errors.h>
#include <utils/Timers.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaDataBase.h>
#include <OMX_IVCommon.h>
#include <media/hardware/MetadataBufferType.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/MemoryHeapBase.h>
#include <binder/MemoryBase.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <media/stagefright/MediaBuffer.h>



//#include <gui/ISurfaceComposer.h>
#include <OMX_Component.h>

#include <utils/Log.h>
#include <utils/String8.h>

#include <private/gui/ComposerService.h>

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/videodev2.h>
#include <hardware/hardware.h>

#include <ScreenCatch/ScreenCatch.h>
#include <ui/PixelFormat.h>
// #include <ui/DisplayInfo.h>

#include <system/graphics.h>
#include <hidl/HidlLazyUtils.h>
#include <hidl/HidlBinderSupport.h>
#include "ScreenControlHal.h"


using android::hardware::LazyServiceRegistrar;
using ::vendor::amlogic::hardware::screencontrol::V1_0::implementation::ScreenControlHal;
using ::android::hidl::base::V1_0::IBase;

#define TIMEOUT_VAL 2 * 1000 * 1000

//#include <android/bitmap.h>

namespace android {
class DeathNotifier: public IBinder::DeathRecipient
{
    public:
        DeathNotifier(sp<ScreenControlService> screencontrolservice) {
            mScreenControlService = screencontrolservice;
        }

        void binderDied(const wp<IBinder>&) {
            mScreenControlService->release();
        }
    private:
        sp<ScreenControlService> mScreenControlService;
};
} // namespace android


namespace android {

ScreenControlService::ScreenControlService():
    mNeedStop(false),
    mPicFd(-1),
    mVideoConvertor(NULL),
    mRecordCorpX(-1),
    mRecordCorpY(-1),
    mRecordCorpWidth(-1),
    mRecordCorpHeight(-1),
    mRecordWidth(-1),
    mRecordHeight(-1),
    mMicroClientId(-1),
    mMicroWidth(-1),
    mMicroHeight(-1),
    mScreenManager(NULL),
    mYuvClientId(-1) ,
    mRecordSourceType(-1) {
}

ScreenControlService::~ScreenControlService() {
    ALOGI("~ScreenControlService");
}

ScreenControlService* ScreenControlService::getInstance() {
    ScreenControlService *mScreenControl = new ScreenControlService();
    return mScreenControl;
}



void ScreenControlService::instantiate(bool lazyMode) {
    android::status_t ret;
    if (!lazyMode) {
        ret = defaultServiceManager()->addService(
            String16("screen_control"), new ScreenControlService());
    } else {
        ret = LazyServiceRegistrar::getInstance().registerService(
            new ScreenControlHal, "default");
    }
    if (ret != android::OK) {
        ALOGE("Couldn't register screen_control service!");
    }
    ALOGI("instantiate add service result:%d", ret);

}

void ScreenControlService::forceStop() {
    ALOGI("forceStop()");
    mNeedStop = true;
    mRecordCorpX = -1 ;
    mRecordCorpY = -1;
    mRecordCorpWidth = -1;
    mRecordCorpHeight = -1;
    mRecordWidth = -1;
    mRecordHeight = -1;
    if (mVideoConvertor != NULL) {
        mVideoConvertor->stop();
        mNeedStop = false;
        mRecordSourceType = -1;
        mVideoConvertor=NULL;
    }
}
int ScreenControlService::setScreenRecordCropArea(int32_t left, int32_t top, int32_t right, int32_t bottom) {
    Mutex::Autolock autoLock(mLock);
    ALOGI("setScreenRecordCropArea left:%d, top:%d, right:%d, bottom:%d ", left, top, right, bottom);
    mRecordCorpX = left ;
    mRecordCorpY = top;
    mRecordCorpWidth = right;
    mRecordCorpHeight = bottom;
    return OK;
}

int ScreenControlService::startScreenRecord(int32_t width, int32_t height, int32_t frameRate, int32_t bitRate, int32_t limitTimeSec, int32_t sourceType, const char* filename) {
    Mutex::Autolock autoLock(mLock);
    ALOGI("startScreenRecord width:%d, height:%d, frameRate:%d, bitRate:%d, limitTimeSec:%d, sourceType:%d, filename:%s\n", width, height, frameRate, bitRate, limitTimeSec, sourceType, filename);

    int err;
    int video_dump_size = 0;
    int32_t limit_time = limitTimeSec * frameRate;
    MediaBufferBase *tVideoBuffer;
    mNeedStop = false;
    int64_t firsetNowUs = 0;
    struct timeval timeNow;
    ProcessState::self()->startThreadPool();

    int video_file = open(filename, O_CREAT | O_RDWR, 0666);
    if (video_file < 0) {
        ALOGE("open file [%s] error: %s", filename, strerror(errno));
        return !OK;
    }

    mTSPacker = new TSPacker(width, height, frameRate, bitRate, sourceType, 0);
//    mTSPacker->setMaxFrameCount(limit_time);
    mTSPacker->setTimeLimit(limitTimeSec*1000);
    if (mRecordCorpX != -1 && mRecordCorpY !=-1 && mRecordCorpWidth != -1 && mRecordCorpHeight != -1) {
        mTSPacker->setVideoCrop(mRecordCorpX, mRecordCorpY, mRecordCorpWidth, mRecordCorpHeight);
    }else
        mTSPacker->setVideoCrop(0, 0, width, height);
    err = mTSPacker->start();

    if (err != OK) {
        ALOGE("[%s %d]TSPacker start fail\n", __FUNCTION__, __LINE__);
        close(video_file);
        return !OK;
    }
    gettimeofday(&timeNow, NULL);
    firsetNowUs = (int64_t)timeNow.tv_sec*1000*1000 + (int64_t)timeNow.tv_usec;
    mRecordSourceType = sourceType;
    mRecordWidth = width;
    mRecordHeight = height;
    while (!mNeedStop) {
        tVideoBuffer = NULL;
        err = mTSPacker->read(&tVideoBuffer);
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        int64_t nowUs = (int64_t)timeNow.tv_sec*1000*1000 + (int64_t)timeNow.tv_usec;
        int64_t diff = nowUs -firsetNowUs;
        int64_t limitTimeUs = (int64_t)limitTimeSec *1000 *1000;
        if (video_dump_size == 0 && (diff >= limitTimeUs)) {
            ALOGE("[%s %d] no data !!!! break", __FUNCTION__, __LINE__);
            break;
        }

        if (err != OK) {
            usleep(10 *1000);
            continue;
        }

        err = write(video_file, tVideoBuffer->data(),tVideoBuffer->range_length());
        if (err < 0) {
            ALOGE("write file [%s] error:%s", filename, strerror(errno));
        }
        video_dump_size += tVideoBuffer->range_length();
        ALOGI("[%s %d] video limit_time:%d size:%d dump_size:%d\n", __FUNCTION__, __LINE__, limit_time, tVideoBuffer->range_length(), video_dump_size);

        tVideoBuffer->release();
        tVideoBuffer = NULL;
        if (OK == mTSPacker->checkConvertDone()) {
            ALOGI("Check convert done, break...");
            break;
        }
    }

    ALOGI("tspacker stop\n");
    mRecordCorpX = -1 ;
    mRecordCorpY = -1;
    mRecordCorpWidth = -1;
    mRecordCorpHeight = -1;
    mRecordSourceType = -1;
    mTSPacker->stop();
    mTSPacker = NULL;
    close(video_file);
	if (mNeedStop) {
        ALOGD("Control to stop record!");
    }
    return OK;
}

int ScreenControlService::startScreenCap(int32_t left, int32_t top, int32_t right, int32_t bottom, int32_t width, int32_t height, int32_t sourceType, const char* filename) {
    Mutex::Autolock autoLock(mLock);
    ALOGI("startScreenCap left:%d, top:%d, right:%d, bottom:%d, width:%d, height:%d, sourceType:%d, filename:%s\n", left, top, right, bottom, width, height, sourceType, filename);

    int status;
    int count = 0;
    uint32_t f = PIXEL_FORMAT_RGBA_8888;
    status_t result = NO_ERROR;
    ScreenCatch* mScreenCatch;
    const size_t size = width * height * 4;
    mNeedStop = false;

    //ProcessState::self()->startThreadPool();

    sp<MemoryHeapBase> memoryBase(new MemoryHeapBase(size, 0, "screen-capture"));
    void* const base = memoryBase->getBase();

    if (base != nullptr) {
        mScreenCatch = new ScreenCatch(width, height, sourceType);
        mScreenCatch->setVideoCrop(left, top, right, bottom);

        MetaDataBase* pMeta;
        pMeta = new MetaDataBase();
        pMeta->setInt32(kKeyColorFormat, OMX_COLOR_Format32bitARGB8888);
        result = mScreenCatch->start(pMeta);
        pMeta->clear();
        delete pMeta;
        if ( result != OK) {
            ALOGE("[%s %d] screenCatch start fail", __FUNCTION__, __LINE__);
            delete mScreenCatch;
            return UNKNOWN_ERROR;
        }
        MediaBuffer *buffer = NULL;

        while ((!mNeedStop) && (count < 1)) {
            status = mScreenCatch->read(&buffer);
            if (status != OK) {
                usleep(10 *1000);
                continue;
            }

            count++;
            ALOGI("[%s %d] dump:%s size:%d", __FUNCTION__, __LINE__, filename, buffer->size());
            if (buffer->data() == NULL) {
                buffer->release();
                buffer = NULL;
                break;
            }
            memcpy(base, buffer->data(), buffer->size());

            if (mPicFd < 0)
                mPicFd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
#if 0
            const SkImageInfo info = SkImageInfo::Make(width, height, flinger2skia(f), kPremul_SkAlphaType, nullptr);
            SkPixmap pixmap(info, base, width * getBytesPerPixel(f));
            struct FDWStream final : public SkWStream {
                size_t fBytesWritten = 0;
                int fFd;
                FDWStream(int f) : fFd(f) {}
                size_t bytesWritten() const override {
                    return fBytesWritten;
                }
                bool write(const void* buffer, size_t size) override {
                    fBytesWritten += size;
                    return size == 0 || ::write(fFd, buffer, size) > 0;
                }
            } fdStream(dumpfd);

            (void)SkEncodeImage(&fdStream, pixmap, SkEncodedImageFormat::kJPEG, 100);
#else
            //TODO: fix save JPEG/PNG
#endif
            if (mPicFd >= 0)
                close(mPicFd);
            buffer->release();
            buffer = NULL;
        }

        memoryBase.clear();
        mScreenCatch->stop();
        delete mScreenCatch;
    } else {
        result = UNKNOWN_ERROR;
    }

    if (mNeedStop) {
        ALOGD("Control to stop capture screen");
    }
    if (count < 1) {
        result = UNKNOWN_ERROR;
    }
    ALOGE("[%s %d] startScreenCap finish", __FUNCTION__, __LINE__);
    return result;
}

int ScreenControlService::startScreenCapBuffer(int32_t left, int32_t top, int32_t right, int32_t bottom, int32_t width, int32_t height, int32_t sourceType, void *dstBuffer, int32_t *dstBufferSize) {
    ALOGI("[%s] left:%d, top:%d, right:%d, bottom:%d, width:%d, height:%d, sourceType:%d\n",
        __func__, left, top, right, bottom, width, height, sourceType);

    int status;
    int count = 0;
    int32_t client_id = 0;
    uint32_t f = PIXEL_FORMAT_RGBA_8888;
    status_t result = NO_ERROR;
    ScreenCatch* mScreenCatch;
    struct timeval timeNow;
    const size_t size = width * height * 4;
    mNeedStop = false;
    if ((mTSPacker != NULL || mVideoConvertor != NULL ) && mRecordSourceType == sourceType ) {
        ALOGI("[%s %d] get same parameter", __FUNCTION__, __LINE__);
        long buf[3] ={ 0 };
        int32_t bufferSize = width * height * 4;
        MediaBuffer *tBuffer = new MediaBuffer(bufferSize);
        if (tBuffer == NULL || tBuffer->data() == NULL) {
            /* coverity[leaked_storage] */
            return UNKNOWN_ERROR;
        }
        if (mTSPacker != NULL ) {
            while (mTSPacker->readRawData(tBuffer,width,height) == !OK) {
                usleep(5 *1000); //5ms
            }
        }else {
            while (mVideoConvertor->readRawData(tBuffer,width,height) == !OK ) {
                usleep(5 *1000); //5ms
            }
        }
        memcpy(dstBuffer, tBuffer->data(), tBuffer->size());
        *dstBufferSize = tBuffer->size();
        ALOGI("[%s %d] get readRawData size:%d", __FUNCTION__, __LINE__, tBuffer->size());
        tBuffer->release();
        /* coverity[leaked_storage] */
        return result;
    }else if(mScreenManager != NULL && mRecordSourceType == sourceType) {
        void * raw = NULL;
        while (mScreenManager->readRawData(mYuvClientId,&raw) == !OK) {
                usleep(5 *1000); //5ms
        }
        if (raw == NULL)
            return !OK;
        if (width != mRecordWidth || height != mRecordHeight) {
            size_t temp_size = mRecordWidth*mRecordHeight*4;
            MediaBuffer* temp = new MediaBuffer(temp_size);
            if (temp) {
                nv21_to_rgb32_((unsigned char *)raw, (unsigned char *)temp->data() , mRecordWidth, mRecordHeight);
                if (temp->data() == NULL) {
                    ALOGE("[%s %d] nv21_to_rgb32 error !", __FUNCTION__, __LINE__);
                    temp->release();
                    free(raw);
                    /* coverity[leaked_storage] */
                    return !OK;
                }
                temp->set_range(0, temp_size);
                argb_scale((unsigned char *)temp->data(), (unsigned char *)dstBuffer, mRecordWidth, mRecordHeight, width, height);
                temp->release();
                /* coverity[leaked_storage] */
            }else {
                ALOGE("new MediaBuffer failed");
                free(raw);
                return !OK;
            }
            /* coverity[leaked_storage] */
        }else {
            nv21_to_rgb32_((unsigned char *)raw, (unsigned char *)dstBuffer , width, height);
            if (dstBuffer == NULL) {
                ALOGE("[%s %d] nv21_to_rgb32_ error 2!", __FUNCTION__, __LINE__);
                free(raw);
                return !OK;
            }
        }
        *dstBufferSize = size;
        ALOGI("[%s %d] get readRawData size:%d", __FUNCTION__, __LINE__, size);
        free(raw);
        return result;
    }


    mScreenManager = ScreenManager::instantiate();
    if (mScreenManager == NULL)
        return !OK;
    status_t err = mScreenManager->init(width, height, sourceType, 1, SCREENCONTROL_RGBA888_TYPE, &client_id);
    if ( err != OK ) {
        ALOGE("[%s %d] ScreenManage init error\n", __FUNCTION__, __LINE__);
        return !OK;
    }
    mScreenManager->setVideoCrop(left, top, right, bottom);
    err = mScreenManager->start(client_id,SCREENCONTROL_SCREEN_CATCH);
    if ( err != OK ) {
        ALOGE("[%s %d] ScreenManage init error\n", __FUNCTION__, __LINE__);
        return !OK;
    }
    gettimeofday(&timeNow, NULL);
    int64_t firsetNowUs = (int64_t)timeNow.tv_sec*1000*1000 + (int64_t)timeNow.tv_usec;
    while ((!mNeedStop) && (count < 1)) {
        int index = 0;
        long* buffer = NULL;
        status = mScreenManager->readBuffer(client_id,NULL,&index);
        if (status != OK) {
            gettimeofday(&timeNow, NULL);
            int64_t nowUs = (int64_t)timeNow.tv_sec*1000*1000 + (int64_t)timeNow.tv_usec;
            if ((nowUs - firsetNowUs) >= TIMEOUT_VAL) {
                ALOGE("[%s %d] no data !!!! break,firsetNowUs=%lld,nowUs=%lld", __FUNCTION__, __LINE__,firsetNowUs,nowUs);
                break;
            }
            usleep(5 *1000);
            continue;
        }

        count++;
        mScreenManager->getBufferByID(index,&buffer);
        if (buffer == NULL)
            break;
        memcpy(dstBuffer,buffer,width * height *4);
        *dstBufferSize = width * height * 4;
        long buf_info[3] ={0};
        sp<MemoryHeapBase> newMemoryHeap = new MemoryHeapBase(128*sizeof(long));
        sp<MemoryBase> memory = new MemoryBase(newMemoryHeap, 0, 3*sizeof(long));
        if (memory->unsecurePointer() == NULL)
            return !OK;
        buf_info[1] = (long) buffer;
        memcpy(memory->unsecurePointer(), buf_info, 3*sizeof(long));
        mScreenManager->freeBuffer(client_id, memory);
        buffer = NULL;
        ALOGI("[%s %d] readed buffer size = %d", __FUNCTION__, __LINE__,*dstBufferSize);
    }
    mScreenManager->stop(client_id);
    delete mScreenManager;
    mScreenManager = NULL;
    if (mNeedStop) {
        ALOGD("Control to stop capture screen buf");
    }
    if (count < 1) {
        result = UNKNOWN_ERROR;
    }
    ALOGI("[%s %d] finish", __FUNCTION__, __LINE__);
    return result;
}

int ScreenControlService::startYuvRecord(int32_t width, int32_t height, int32_t frameRate,int32_t sourceType){
    int32_t client_id = 0;
    Mutex::Autolock autoLock(mLock);
    ALOGI("[%s] width:%d, height:%d, frameRate =%d, sourceType:%d\n",
        __func__, width, height, frameRate, sourceType);
    mScreenManager = ScreenManager::instantiate();
    if (mScreenManager == NULL)
      return !OK;
    status_t err = mScreenManager->init(width, height, sourceType, frameRate, SCREENCONTROL_RAWDATA_TYPE, &client_id);
    if ( err != OK ) {
        ALOGE("[%s %d] ScreenManage init error\n", __FUNCTION__, __LINE__);
        return !OK;
    }
    if (mRecordCorpX != -1 && mRecordCorpY !=-1 && mRecordCorpWidth != -1 && mRecordCorpHeight != -1) {
        mScreenManager->setVideoCrop(mRecordCorpX, mRecordCorpY, mRecordCorpWidth, mRecordCorpHeight);
    }else
        mScreenManager->setVideoCrop(0, 0, width, height);
    err = mScreenManager->start(client_id,SCREENCONTROL_SCREEN_RECORD_HARDWARE_ENCODER);
    if ( err != OK ) {
        ALOGE("[%s %d] ScreenManage init error\n", __FUNCTION__, __LINE__);
        return !OK;
    }
    mNeedStop = false;
    mYuvClientId = client_id;
    mRecordSourceType = sourceType;
    mRecordWidth = width;
    mRecordHeight = height;

    return OK;
}
bool ScreenControlService::isHaveYuvDate(){
    Mutex::Autolock autoLock(mLock);

    if (mScreenManager == NULL)
      return false;
    return mScreenManager->isHaveOutputData();
}

int ScreenControlService::getYuvRecordData(void *dstBuffer,int32_t bufSize){
    Mutex::Autolock autoLock(mLock);
    int64_t pts;
    long *raw = NULL;
    int index = 0;
    long buf_info[3] ={0};
    //ALOGE("[%s %d]", __FUNCTION__, __LINE__);
    sp<MemoryHeapBase> newMemoryHeap = new MemoryHeapBase(bufSize);
    sp<MemoryBase> buffer = new MemoryBase(newMemoryHeap, 0, bufSize);
    int status = mScreenManager->readBuffer(mYuvClientId, buffer, &index);
    mScreenManager->getBufferByID(index,&raw);
    if (status == !OK || raw== NULL) {
      return status;
    }

    memmove(dstBuffer,raw,bufSize);
    buf_info[1] = (long) raw;
    if (buffer->unsecurePointer() == NULL)
        return !OK;
    memcpy(buffer->unsecurePointer(), buf_info, 3*sizeof(long));
    mScreenManager->freeBuffer(mYuvClientId, buffer);
    buffer.clear();
    newMemoryHeap.clear();
    return OK;
}

int ScreenControlService::checkYuvRecordDone(){
    Mutex::Autolock autoLock(mLock);
    if (mNeedStop) {
      mScreenManager->setPauseMode(true);
      if (OK == mScreenManager->checkConvertDone()) {
        ALOGD("Detect record data stop and convert done, need stop packer...");
        mScreenManager->stop(mYuvClientId);
        mScreenManager->uninit(mYuvClientId);
        mNeedStop = false;
        mScreenManager=NULL;
        return OK;
      }
    }
    return !OK;


}

int ScreenControlService::startAvcRecord(int32_t width, int32_t height, int32_t frameRate, int32_t bitRate, int32_t sourceType){
    Mutex::Autolock autoLock(mLock);
    int err;
    ALOGI("startAvcRecord width:%d, height:%d, frameRate:%d, bitRate:%d, sourceType:%d\n", width, height, frameRate, bitRate, sourceType);
    MetaDataBase* params_video = new MetaDataBase();
    params_video->setInt32(kKeyWidth, width);
    params_video->setInt32(kKeyHeight, height);

    params_video->setInt32(kKeyFrameRate, frameRate);
    params_video->setInt32(kKeyBitRate, bitRate);

    mVideoConvertor = new ESConvertor(sourceType, 0);
    if (mRecordCorpX != -1 && mRecordCorpY !=-1 && mRecordCorpWidth != -1 && mRecordCorpHeight != -1) {
        mVideoConvertor->setVideoCrop(mRecordCorpX, mRecordCorpY, mRecordCorpWidth, mRecordCorpHeight);
    }else
        mVideoConvertor->setVideoCrop(0, 0, width, height);
    err = mVideoConvertor->start(params_video);
    params_video->clear();
    delete params_video;
    if ( err != OK ) {
        ALOGE("[%s %d] start avc record error\n", __FUNCTION__, __LINE__);
        return !OK;
    }
    mRecordSourceType = sourceType;
    mRecordWidth = width;
    mRecordHeight = height;
    return OK;

}

bool ScreenControlService::isHaveAvcDate(){
    return mVideoConvertor->isHaveOutputData();
}
int ScreenControlService::getAvcRecordData(void *dstBuffer, int32_t *dstBufferSize, int64_t *nowtime){
    Mutex::Autolock autoLock(mLock);
    MediaBufferBase *tVideoBuffer = NULL;
    int64_t realNowTime = 0;
    int err = mVideoConvertor->read(&tVideoBuffer);
    if (err != OK) {
        return !OK;
    }
    memcpy(dstBuffer, tVideoBuffer->data(), tVideoBuffer->size());
    *dstBufferSize = tVideoBuffer->size();
    if (!tVideoBuffer->meta_data().findInt64(kKeyTime,&realNowTime)) {
        return !OK;
    }
    ALOGI("[%s %d] get record data, size:%d,pts =%lld", __FUNCTION__, __LINE__, tVideoBuffer->range_length(),*nowtime);
    tVideoBuffer->release();
    *nowtime = realNowTime;
    tVideoBuffer = NULL;
    return OK;
}
int ScreenControlService::checkAvcRecordDone(){
    Mutex::Autolock autoLock(mLock);
    if (mNeedStop) {
        if (OK == mVideoConvertor->checkAvcConvertDone()) {
            ALOGD("Detect record data stop and convert done, need stop packer...");
            mVideoConvertor->stop();
            mNeedStop = false;
            mRecordSourceType = -1;
            mVideoConvertor=NULL;
            return OK;
        }
    }

    return !OK;
}

int ScreenControlService::startMicroDim(int32_t width, int32_t height) {
    ALOGI("[%s %d]  width=%d,height=%d", __FUNCTION__, __LINE__,width,height);
    int32_t client_id = 0;
    mScreenManager = ScreenManager::instantiate();
    if (mScreenManager == NULL)
        return !OK;
    mScreenManager->init(1280, 720, 0, 1, SCREENCONTROL_MICRODIM_TYPE, &client_id);
    mScreenManager->setVideoCrop(0,0,1280,720);
    mMicroClientId = client_id;
    mMicroWidth = width;
    mMicroHeight = height;
    mScreenManager->setMicroSize(width,height);
    mScreenManager->start(client_id, SCREENCONTROL_SCREEN_RECORD_HARDWARE_ENCODER);
    return OK;
}

int ScreenControlService::getMicroDimData(void *dstBuffer,int32_t bufSize) {
    int32_t index;
    int status;
    ALOGE("[%s %d] mMicroClientId=%d", __FUNCTION__, __LINE__ ,mMicroClientId);
    sp<MemoryHeapBase> newMemoryHeap = new MemoryHeapBase(bufSize);
    sp<MemoryBase> buffer = new MemoryBase(newMemoryHeap, 0, bufSize);

    if (mScreenManager == NULL || mMicroClientId < 0) {
        ALOGE("[%s %d] getMicroDimData mScreenManager == NULL || mMicroClientId < 0", __FUNCTION__, __LINE__);
        return !OK;
    }

    status = mScreenManager->readBuffer(mMicroClientId, buffer, &index);
    if (status == !OK) {
      return status;
    }
    memmove(dstBuffer,buffer->unsecurePointer(),bufSize);
    buffer.clear();
    newMemoryHeap.clear();
    ALOGE("[%s %d] finish ", __FUNCTION__, __LINE__);
    return OK;
}

void ScreenControlService::stopMicroDim() {
    ALOGE("[%s %d]", __FUNCTION__, __LINE__);
    if (mScreenManager == NULL || mMicroClientId < 0)
        return;
    mScreenManager->stop(mMicroClientId);
    mScreenManager->uninit(mMicroClientId);
    mScreenManager = NULL;
    mMicroClientId =-1;
    mMicroWidth = 0;
    mMicroHeight = 0;
    ALOGE("[%s %d] finish", __FUNCTION__, __LINE__);
    return;
}

int ScreenControlService::notifyProcessDied (const sp<IBinder> &binder) {
    ALOGI("notifyProcessDied");
    if (binder == NULL) {
        ALOGE("notifyProcessDied binder is NULL");
        return -1;
    }
    binder->linkToDeath(mDeathNotifier);
    return NO_ERROR;
}

int ScreenControlService::release() {
    ALOGI("release");
    return NO_ERROR;
}

} // namespace android
