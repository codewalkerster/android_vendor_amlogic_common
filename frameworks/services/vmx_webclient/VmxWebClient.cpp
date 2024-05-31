/*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#include <aidlcommonsupport/NativeHandle.h>
#include <utils/Log.h>
#include <dlfcn.h>
#include "VmxWebClient.h"
#include "AmVmxWebClientAdaptor.h"

typedef void *(*WebClientAllocContextFunc)(uint32_t *);

typedef int (*WebClientDecryptFunc)(const void *, struct amVmxWebClientDecryptParam *);

typedef int (*WebClientFreeContextFunc)(void *);

typedef void (*WebClientSetCallbackFunc)(const void *,amVmxWebClientCallback callback, void *);

typedef void (*WebClientGetPropertyFunc)(const void *, std::string);

static WebClientAllocContextFunc webclient_alloc = NULL;
static WebClientDecryptFunc webclient_decrypt = NULL;
static WebClientFreeContextFunc webclient_free = NULL;
static WebClientSetCallbackFunc webclient_setCallback = NULL;
static WebClientGetPropertyFunc webclient_getProperty = NULL;

namespace aidl::vendor::amlogic::hardware::vmx_webclient::implementation {

VmxWebClient::VmxWebClient()
{
    mLibHandle = dlopen("libverimatrixadaptor.so", RTLD_NOW);
    if (mLibHandle == NULL) {
        ALOGE("Unable to locate libverimatrixadaptor.so %s ", dlerror());
        return;
    }

    webclient_alloc =
         (WebClientAllocContextFunc)dlsym(mLibHandle, "amVmxWebClientAllocContext");
    webclient_decrypt =
        (WebClientDecryptFunc)dlsym(mLibHandle, "amVmxWebClientDecrypt");
    webclient_free =
        (WebClientFreeContextFunc)dlsym(mLibHandle, "amVmxWebClientFreeContext");
    webclient_setCallback =
        (WebClientSetCallbackFunc)dlsym(mLibHandle, "amVmxWebClientSetCallback");
    webclient_getProperty =
        (WebClientGetPropertyFunc)dlsym(mLibHandle, "amVmxWebClientGetProperty");

    if (webclient_alloc)
        mWebClientObj = webclient_alloc(NULL);

    ALOGI("Create mWebClientObj is %p", mWebClientObj);
}

VmxWebClient::~VmxWebClient() {
    ::android::Mutex::Autolock autoLock(mLock);

    if (mWebClientObj) {
        if (webclient_free) {
            webclient_free(mWebClientObj);
            mWebClientObj = NULL;
        }
    }

    if (mLibHandle != NULL) {
        dlclose(mLibHandle);
        mLibHandle = NULL;
    }
}

::ndk::ScopedAStatus VmxWebClient::createInstance(int32_t* _aidl_return) {
    (void)_aidl_return;
    ::android::Mutex::Autolock autoLock(mLock);

    uint32_t errorCode = 0;
    if (webclient_alloc && !mWebClientObj)
        mWebClientObj = webclient_alloc(&errorCode);

    ALOGI("Create mWebClientObj is %p errorCode %d ", mWebClientObj, errorCode);
    if (!mWebClientObj)
        return toNdkScopedAStatus(static_cast<Status>(errorCode));
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus VmxWebClient::destroyInstance(int32_t* _aidl_return) {
    (void)_aidl_return;
    ::android::Mutex::Autolock autoLock(mLock);

    ALOGI("Destroy mWebClientObj is %p", mWebClientObj);
    if (mWebClientObj) {
        if (webclient_free) {
            webclient_free(mWebClientObj);
            mWebClientObj = NULL;
        }
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus VmxWebClient::decrypt(const VmxWebClientDecryptParam& para,
        std::vector<uint8_t>* outData, int32_t* _aidl_return) {
    (void)para;
    (void)outData;
    (void)_aidl_return;
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus VmxWebClient::decryptSecure(const VmxWebClientDecryptParam& para,
        int32_t* _aidl_return) {
    (void)_aidl_return;
    ::android::Mutex::Autolock autoLock(mLock);

    struct amVmxWebClientDecryptParam amPara;
    int ret = 0;

    memset(&amPara, 0, sizeof(amPara));

    if (mWebClientObj && webclient_decrypt) {
        amPara.mSecure = para.secure;
        amPara.mSampleAES = para.sampleAES;
        amPara.mKeySeq = para.keySeq;

        if (para.mode == Mode::UNENCRYPTED) {
            amPara.mMode = kMode_Unencrypted;
        } else if (para.mode == Mode::AES_CTR) {
            amPara.mMode = kMode_AES_CTR;
        } else if (para.mode == Mode::AES_CBC) {
            amPara.mMode = kMode_AES_CBC;
        } else {
            ALOGE("Can't support decrypt mode");
            return toNdkScopedAStatus(Status::ERROR_DRM_CANNOT_HANDLE);
        }

        amPara.mPattern.mEncryptBlocks = para.pattern.encryptBlocks;
        amPara.mPattern.mSkipBlocks = para.pattern.skipBlocks;

        if (para.key.size() > 0) {
            amPara.mKeyUrl = (const char *)para.key.data();
            amPara.mKeyUrlLen = para.key.size();
        }

        if (para.iv.size() > 0) {
            amPara.mIv = para.iv.data();
            amPara.mIvLen = para.iv.size();
        }

        if (para.subSamples.size() > 0) {
            amPara.mSubSamples = reinterpret_cast<const struct SubSamples *>(para.subSamples.data());
            amPara.mNumSubSamples = para.subSamples.size();
        }

        amPara.mSourceHandle = ::android::makeFromAidl(para.sourceDesc);
        amPara.mSecureHandle = ::android::makeFromAidl(para.secureDesc);
        amPara.mSrcOffset = para.srcOffset;
        amPara.mOffset = para.offset;

        ret = webclient_decrypt(mWebClientObj, &amPara);
        if (ret) {
            ALOGE("decrypt failed 0x%x", ret);
            return toNdkScopedAStatus(static_cast<Status>(ret));
        }
    } else {
        ALOGE("Invalid obj or decrypt interface %p %p", mWebClientObj, webclient_decrypt);
        return toNdkScopedAStatus(Status::ERROR_DRM_SESSION_LOST_STATE);
    }

    return ::ndk::ScopedAStatus::ok();
}

void OnCallback(uint8_t type, uint8_t *data, uint32_t dataLen, void *pUserData) {
    ALOGI("OnCallback type %d len %d", type, dataLen);

    if (pUserData != NULL) {
        std::vector<uint8_t> event;
        VmxWebClient *p = (VmxWebClient *)pUserData;
        event.assign(data, data + dataLen);
        if (p->mCallback != NULL) {
            p->mCallback->sendEvent(static_cast<VmxWebClientEventType>(type), p->mSessionId, event);
        }
    }
}

::ndk::ScopedAStatus VmxWebClient::setCallback(const std::vector<uint8_t>& sessionId,
        const std::shared_ptr<IVmxWebClientCallback>& callback) {
    ::android::Mutex::Autolock autoLock(mLock);
    ALOGI("setCallback");

    if (mWebClientObj && webclient_setCallback && callback) {
        mCallback = callback;
        mSessionId.assign(sessionId.data(), sessionId.data() + sessionId.size());
        webclient_setCallback(mWebClientObj, OnCallback, this);
        ALOGI("setCallback done");
    }
    return toNdkScopedAStatus(Status::OK);
}

::ndk::ScopedAStatus VmxWebClient::getProperty(const std::string& value) {
    ::android::Mutex::Autolock autoLock(mLock);
    ALOGI("getProperty");
    if (mWebClientObj && webclient_getProperty && !value.empty()) {
        std::string prop = value;
        webclient_getProperty(mWebClientObj, prop);
        ALOGI("getProperty done");
    }
    return toNdkScopedAStatus(Status::OK);
}

}  // namespace vendor::amlogic::hardware::vmx_webclient::implementation
