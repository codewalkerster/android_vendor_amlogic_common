/*
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#ifndef _HDCP_SERVICE_H
#define _HDCP_SERVICE_H

#include <aidl/vendor/amlogic/hardware/miracast_hdcp2/BnHDCPService.h>
#include <aidl/vendor/amlogic/hardware/miracast_hdcp2/IHDCP.h>
#include "HDCP.h"

namespace vendor {
namespace amlogic {
namespace hardware {
namespace miracast_hdcp2 {

using ::aidl::vendor::amlogic::hardware::miracast_hdcp2::IHDCP;
using ::aidl::vendor::amlogic::hardware::miracast_hdcp2::BnHDCPService;

class HDCPService : public BnHDCPService {
public:
  ::ndk::ScopedAStatus makeHDCP(bool in_createEncryptionModule,
    std::shared_ptr<IHDCP>* _aidl_return) override {
    ::ndk::ScopedAStatus _aidl_status;
    std::shared_ptr<HDCP> hdcp = ::ndk::SharedRefBase::make<HDCP>(in_createEncryptionModule);
    *_aidl_return = hdcp;
    if (_aidl_return->get())
      _aidl_status.set(AStatus_fromStatus(android::OK));
    else
      _aidl_status.set(AStatus_fromStatus(STATUS_UNKNOWN_TRANSACTION));

    return _aidl_status;
  }
};

}  // namespace miracast_hdcp2
}  // namespace hardware
}  // namespace amlogic
}  // namespace vendor

#endif
