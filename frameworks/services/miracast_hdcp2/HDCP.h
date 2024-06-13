/*
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#ifndef _HDCP_IMPLEMENT_H_
#define _HDCP_IMPLEMENT_H_

#include <utils/Mutex.h>
#include <utils/Log.h>
#include <media/hardware/HDCPAPI.h>
#include <aidl/vendor/amlogic/hardware/miracast_hdcp2/BnHDCP.h>
#include <aidl/vendor/amlogic/hardware/miracast_hdcp2/IHDCP.h>
#include <aidl/vendor/amlogic/hardware/miracast_hdcp2/IHDCPObserver.h>
#include <aidl/vendor/amlogic/hardware/miracast_hdcp2/EncryptResult.h>

namespace vendor {
namespace amlogic {
namespace hardware {
namespace miracast_hdcp2 {

using ::aidl::vendor::amlogic::hardware::miracast_hdcp2::BnHDCP;
using ::aidl::vendor::amlogic::hardware::miracast_hdcp2::IHDCPObserver;
using ::aidl::vendor::amlogic::hardware::miracast_hdcp2::EncryptResult;
using ::android::sp;
using ::android::Mutex;
using ::android::HDCPModule;

class HDCP : public BnHDCP {
public:
  HDCP(bool in_createEncryptionModule);
  ~HDCP();

  ::ndk::ScopedAStatus decrypt(const std::vector<uint8_t>& in_inData,
    int32_t in_streamCTR, int64_t in_outInputCTR, int32_t in_outAddr,
    std::vector<uint8_t>* _aidl_return) override;
  ::ndk::ScopedAStatus decryptSecure(const std::vector<uint8_t>& in_decryptInfo,
    const std::vector<uint8_t>& in_inData, std::vector<uint8_t>* _aidl_return) override;
  ::ndk::ScopedAStatus encrypt(const std::vector<uint8_t>& in_inData, int32_t in_streamCTR,
    EncryptResult* _aidl_return) override;
  ::ndk::ScopedAStatus getCaps(int32_t* _aidl_return) override;
  ::ndk::ScopedAStatus initAsync(const std::string& in_host, int32_t in_port) override;
  ::ndk::ScopedAStatus setObserver(const std::shared_ptr<IHDCPObserver>& in_observer) override;
  ::ndk::ScopedAStatus shutdownAsync() override;

private:
    Mutex mLock;
    bool mIsEncryptionModule;
    void *mLibHandle;
    HDCPModule *mHDCPModule;
    std::shared_ptr<IHDCPObserver> mObserver;

    static void ObserveWrapper(void *me, int msg, int ext1, int ext2);
    void observe(int msg, int ext1, int ext2);
    HDCP();
};

}  // namespace miracast_hdcp2
}  // namespace hardware
}  // namespace amlogic
}  // namespace vendor

#endif
