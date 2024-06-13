/*
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#define LOG_NDEBUG 0
#define LOG_TAG "miracast_hdcp_aidl"


#include "HDCP.h"
#include <dlfcn.h>

namespace vendor {
namespace amlogic {
namespace hardware {
namespace miracast_hdcp2 {

HDCP::HDCP(bool createEncryptionModule)
  : mIsEncryptionModule(createEncryptionModule),
    mLibHandle(NULL),
    mHDCPModule(NULL),
    mObserver(NULL) {

  mLibHandle = dlopen("libstagefright_hdcp.so", RTLD_NOW);
  if (mLibHandle == NULL) {
    ALOGE("Unable to locate libstagefright_hdcp.so");
    return;
  }

  typedef HDCPModule *(*CreateHDCPModuleFunc)(void *, HDCPModule::ObserverFunc);

  CreateHDCPModuleFunc createHDCPModule =
    mIsEncryptionModule
      ? (CreateHDCPModuleFunc)dlsym(mLibHandle, "createHDCPModule")
      : (CreateHDCPModuleFunc)dlsym(mLibHandle, "createHDCPModuleForDecryption");

  if (createHDCPModule == NULL) {
    ALOGE("Unable to find symbol 'createHDCPModule'.");
  } else if ((mHDCPModule = createHDCPModule(this, &HDCP::ObserveWrapper)) == NULL) {
    ALOGE("CreateHDCPModule failed.");
  }
}

HDCP::~HDCP() {
  Mutex::Autolock autoLock(mLock);
  if (mHDCPModule != NULL) {
      delete mHDCPModule;
      mHDCPModule = NULL;
  }

  if (mLibHandle != NULL) {
      dlclose(mLibHandle);
      mLibHandle = NULL;
  }
}

::ndk::ScopedAStatus HDCP::setObserver(const std::shared_ptr<IHDCPObserver>& observer) {
  Mutex::Autolock autoLock(mLock);

  if (mHDCPModule == NULL)
    return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_NO_INIT));

  mObserver = observer;
  return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_OK));
}

::ndk::ScopedAStatus HDCP::initAsync(const std::string& host, int32_t port) {
  Mutex::Autolock autoLock(mLock);
  const char *hostname = NULL;

  if (mHDCPModule == NULL)
    return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_NO_INIT));

  if (host.size())
    hostname = host.c_str();

  if (mHDCPModule->initAsync(hostname, port))
    return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_UNKNOWN_ERROR));

  return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_OK));
}

::ndk::ScopedAStatus HDCP::shutdownAsync() {
  Mutex::Autolock autoLock(mLock);

  if (mHDCPModule == NULL)
    return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_NO_INIT));

  if (mHDCPModule->shutdownAsync())
    return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_UNKNOWN_ERROR));

  return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_OK));
}

::ndk::ScopedAStatus HDCP::getCaps(int32_t* _aidl_return) {
  Mutex::Autolock autoLock(mLock);

  if (mHDCPModule == NULL)
    *_aidl_return = 0;

  *_aidl_return = mHDCPModule->getCaps();
  return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_OK));
}

::ndk::ScopedAStatus HDCP::encrypt(const std::vector<uint8_t>& in_inData, int32_t in_streamCTR,
    EncryptResult* _aidl_return) {
  (void)in_inData;
  (void)in_streamCTR;
  (void)_aidl_return;

  return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_INVALID_OPERATION));
}

::ndk::ScopedAStatus HDCP::decrypt(const std::vector<uint8_t>& inData,
  int32_t streamCTR, int64_t outInputCTR, int32_t outAddr, std::vector<uint8_t>* _aidl_return) {
  Mutex::Autolock autoLock(mLock);
  std::vector<uint8_t> outData;
  uint32_t size = inData.size();
  uint32_t streamCtrInfo = 0;
  android::status_t ret = 0;

  assert(!mIsEncryptionModule);

  if (mHDCPModule == NULL)
    return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_NO_INIT));

  if (outAddr == 0) {
    streamCtrInfo = streamCTR << 4 | 0;
    outData.resize(size);
    ret = (android::status_t)mHDCPModule->decrypt(inData.data(), size, streamCtrInfo,
         outInputCTR, outData.data());
  } else {
    streamCtrInfo = streamCTR << 4 | 1;
    ret = (android::status_t)mHDCPModule->decrypt(inData.data(), size, streamCtrInfo,
        outInputCTR, (void *)(long)outAddr);
  }

  if (ret)
    return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_UNKNOWN_ERROR));

  if (outAddr == 0)
    *_aidl_return = outData;
  return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_OK));
}

::ndk::ScopedAStatus HDCP::decryptSecure(const std::vector<uint8_t>& decryptInfo,
  const std::vector<uint8_t>& inData, std::vector<uint8_t>* _aidl_return) {
  Mutex::Autolock autoLock(mLock);
  std::vector<uint8_t> outData = decryptInfo;

  uint32_t streamCtrInfo = 2;
  android::status_t ret = 0;

  assert(!mIsEncryptionModule);

 if (mHDCPModule == NULL)
    return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_NO_INIT));

  ret = (android::status_t)mHDCPModule->decrypt(inData.data(), inData.size(), streamCtrInfo,
     0, outData.data());
  if (ret)
    return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_UNKNOWN_ERROR));

  *_aidl_return = outData;
  return ndk::ScopedAStatus(AStatus_fromStatus(STATUS_OK));
}

void HDCP::ObserveWrapper(void *me, int msg, int ext1, int ext2) {
  static_cast<HDCP *>(me)->observe(msg, ext1, ext2);
}

void HDCP::observe(int msg, int ext1, int ext2) {
  Mutex::Autolock autoLock(mLock);
  mObserver->notify(msg, ext1, ext2);
}

}  // namespace miracast_hdcp2
}  // namespace hardware
}  // namespace amlogic
}  // namespace vendor
