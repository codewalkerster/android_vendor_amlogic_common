/*
 * Copyright (C) 2006 The Android Open Source Project
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
#define LOG_NDEBUG 0
#define LOG_TAG "MediaCollector"
#include <dlfcn.h>
#include <stdio.h>
#include <utils/Log.h>
#include "MediaCollector.h"
namespace android {
MediaProxyWrapper::MediaProxyWrapper()
        : mProxyHandle(nullptr),
          mCallBack(nullptr),
          mStarted(false),
          mProxyFd(-1) {
    mProxyHandle = dlopen("libmediaproxy_consumer.so", RTLD_NOW);
    if (!mProxyHandle) {
        ALOGE("Failed to load libmediaproxy_consumer.so");
        return;
    }
    mpconsumer_initialize = (MPConsumer_initialize)dlsym(mProxyHandle, "initialize");
    mpconsumer_register_msg_type = (MPConsumer_register_msg_type)dlsym(mProxyHandle, "registerMsgType");
    mpconsumer_read_data = (MPConsumer_read_data)dlsym(mProxyHandle, "readData");
    mpconsumer_destroy = (MPConsumer_destroy)dlsym(mProxyHandle, "destroy");
    if (mpconsumer_initialize == nullptr || mpconsumer_register_msg_type == nullptr ||
        mpconsumer_read_data == nullptr || mpconsumer_destroy == nullptr) {
        ALOGE("Failed to load symbols from libmediaproxyconsumer.so");
        dlclose(mProxyHandle);
        mProxyHandle = nullptr;
        return;
    }
    ALOGI("MediaProxyWrapper construct");
}
MediaProxyWrapper::~MediaProxyWrapper() {
    ALOGI("~MediaProxyWrapper");
    if (mProxyHandle)
        dlclose(mProxyHandle);
}

bool MediaProxyWrapper::start(CollectedDataCallback* cb) {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::unique_lock<std::mutex> lock(mLock);
    if (mStarted || !mProxyHandle)
        return false;
    if (!cb) {
        ALOGE("[%s %d] can't get the callback", __FUNCTION__, __LINE__);
        return false;
    }
    mProxyFd = mpconsumer_initialize();
    if (mProxyFd < 0) {
        cb->onError("Failed to initialize media proxy consumer");
        ALOGE("Failed to initialize media proxy consumer");
        return false;
    }
    if (mpconsumer_register_msg_type(mProxyFd, MEDIA_VIDEO_ERROR_EVENT)) {
        cb->onError("Failed to register message");
        ALOGW("Failed to register message");
        return false;
    }
    mStarted = true;
    mCallBack = cb;
    mThreadPool.push_back(std::thread(&MediaProxyWrapper::threadFunc, this));
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
    return true;
}

void MediaProxyWrapper::stop() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::unique_lock<std::mutex> lock(mLock);
    if (!mStarted) {
        ALOGE("[%s %d] the module collector has not been started !!!", __FUNCTION__, __LINE__);
        return;
    }
    mStarted = false;
    mCondition.wait(lock);
    for (int i = 0; i < mThreadPool.size(); i++) {
        mThreadPool[i].join();
    }
    mThreadPool.clear();
    if (mProxyFd > 0)
        mpconsumer_destroy(mProxyFd);
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
}

void MediaProxyWrapper::processUserData(struct aml_video_user_data* userData) {
    if (!mStarted || !mCallBack)
        return;
    uint32_t messageType = userData->message_type;
    ALOGI("[%s %d]  messageType = 0x%02x", __FUNCTION__, __LINE__, messageType);
    if (messageType == MEDIA_VIDEO_ERROR_EVENT) {
        uint32_t error = userData->data.error_info.error_event;
        ALOGI("[%s %d]  error = 0x%02x", __FUNCTION__, __LINE__, error);
        int32_t subModule = EERORMONITOR_SUBMODULE_UNKNOWN;
        int32_t level = EERORMONITOR_ERROR_LEVEL_SLIGHT;
        int32_t errorType = error;
        char msg[128] = {0};
        switch (error) {
            case MEDIA_ERRORCODES_VDEC_M_BAD_INPUT:
            case MEDIA_ERRORCODES_VDEC_E_TIMEOUT:
                subModule = EERORMONITOR_SUBMODULE_VEDC;
                level = EERORMONITOR_ERROR_LEVEL_SERIOUS;
                snprintf(msg, 128, "%s-%s", "vdec", "bad-input");
                break;
            case MEDIA_ERRORCODES_VDEC_W_INPUT_UNDERRUN:
                subModule = EERORMONITOR_SUBMODULE_VEDC;
                level = EERORMONITOR_ERROR_LEVEL_NORMAL;
                snprintf(msg, 128, "%s-%s", "vdec", "input-underrun");
                break;
            case MEDIA_ERRORCODES_VDEC_W_OUTPUT_UNDERRUN:
                subModule = EERORMONITOR_SUBMODULE_VEDC;
                level = EERORMONITOR_ERROR_LEVEL_SERIOUS;
                snprintf(msg, 128, "%s-%s", "vdec", "output-underrun");
                break;
            default:
                ALOGE("the error data can't be known,so drop it !!!");
                break;
        }
        if (subModule != EERORMONITOR_SUBMODULE_UNKNOWN) {
            ALOGI("[%s %d]  get the error event,subModule = %d,level=%d,errorType=%d,msg=%s", __FUNCTION__, __LINE__,
                  subModule, level, errorType, msg);
            auto data = std::make_unique<CollectedData>(subModule, level, 0x01, errorType, getNowTimesUs(),msg);
            if (mCallBack)
                mCallBack->onCollect(data);
        }
    }
}
void MediaProxyWrapper::threadFunc() {
    while (1) {
        {
            std::lock_guard<std::mutex> lock(mLock);
            if (!mStarted) {
                mCondition.notify_one();
                break;
            }
            struct aml_video_user_data userData;
            if (mpconsumer_read_data(mProxyFd, &userData) > 0)
                processUserData(&userData);
        }
        usleep(5 * 1000); // 5ms
    }
}

MediaCollector::MediaCollector()
        : mStarted(false) {
    ALOGI("MediaCollector construct");
}

MediaCollector::~MediaCollector() {
    ALOGI("~MediaCollector");
    if (mStarted) {
        stop();
    }
}
bool MediaCollector::start() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    if (mStarted) {
        ALOGE("[%s %d] the media collector has been started !!!", __FUNCTION__, __LINE__);
        return false;
    }
    CollectedDataCallback* cb = getListener();
    if (!cb) {
        ALOGE("[%s %d] can't get the callback", __FUNCTION__, __LINE__);
        return false;
    }
    mMediaProxyWrapper = std::make_unique<MediaProxyWrapper>();
    if (!mMediaProxyWrapper)
        return false;
    if (!mMediaProxyWrapper->start(this)) {
        ALOGE("[%s %d] the media proxy can't be started !!", __FUNCTION__, __LINE__);
        //return false;
    }
    mStarted = true;
    mThreadPool.push_back(std::thread(&MediaCollector::threadFunc, this));
    mErrorDataQueue.clear();
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
    return true;
}

void MediaCollector::stop() {
    ALOGI("[%s %d]  begin", __FUNCTION__, __LINE__);
    std::unique_lock<std::mutex> lock(mLock);
    if (!mStarted) {
        ALOGE("[%s %d] the module collector has not been started !!!", __FUNCTION__, __LINE__);
        return;
    }
    mMediaProxyWrapper->stop();
    mStarted = false;
    mCondition.wait(lock);
    for (int i = 0; i < mThreadPool.size(); i++) {
        mThreadPool[i].join();
    }
    mThreadPool.clear();
    mErrorDataQueue.clear();
    ALOGI("[%s %d]  end", __FUNCTION__, __LINE__);
    return;
}

void MediaCollector::onCollectWork() {
    if (mErrorDataQueue.empty())
        return;
    auto data = mErrorDataQueue.begin();
    CollectedDataCallback* cb = getListener();
    if ((*data) && cb) {
        cb->onCollect(*data);
    }
    mErrorDataQueue.pop_front();
}

void MediaCollector::threadFunc() {
    while (1) {
        {
            std::lock_guard<std::mutex> lock(mLock);
            if (!mStarted) {
                mCondition.notify_one();
                break;
            }
            onCollectWork();
        }
        usleep(5 * 1000); // 5ms
    }
}

void MediaCollector::onCollect(std::unique_ptr<CollectedData>& data) {
    ALOGI("[%s %d] %s", __FUNCTION__, __LINE__, data->toString().c_str());
    mErrorDataQueue.push_back(std::move(data));
}

void MediaCollector::onError(const std::string& msg) {
    CollectedDataCallback* cb = getListener();
    if (cb) {
        cb->onError(msg);
    }
}

}; // namespace android
