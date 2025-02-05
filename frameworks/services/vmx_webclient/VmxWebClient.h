/*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#pragma once

#include <aidl/vendor/amlogic/hardware/vmx_webclient/BnVmxWebClient.h>
#include <aidl/vendor/amlogic/hardware/vmx_webclient/IVmxWebClientCallback.h>
#include <aidl/vendor/amlogic/hardware/vmx_webclient/Status.h>
#include <utils/Mutex.h>

namespace aidl::vendor::amlogic::hardware::vmx_webclient::implementation {

using ::aidl::vendor::amlogic::hardware::vmx_webclient::Mode;
using ::aidl::vendor::amlogic::hardware::vmx_webclient::Status;
using ::aidl::vendor::amlogic::hardware::vmx_webclient::VmxWebClientDecryptParam;
using ::aidl::vendor::amlogic::hardware::vmx_webclient::IVmxWebClientCallback;
using ::aidl::vendor::amlogic::hardware::vmx_webclient::KeyRequestParam;
using ::aidl::vendor::amlogic::hardware::vmx_webclient::PipelineParam;

inline ::ndk::ScopedAStatus toNdkScopedAStatus(Status status,
                                               const char* msg = nullptr) {
    if (Status::OK == status) {
        return ::ndk::ScopedAStatus::ok();
    } else {
        auto err = static_cast<int32_t>(status);
        if (msg) {
            return ::ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(err, msg);
        } else {
            return ::ndk::ScopedAStatus::fromServiceSpecificError(err);
        }
    }
}

struct VmxWebClient : public BnVmxWebClient {
    VmxWebClient();
    ~VmxWebClient();
    ::ndk::ScopedAStatus createInstance(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus decrypt(const std::vector<uint8_t>& sessionid,
                                const std::vector<uint8_t>& keyid,
                                const std::vector<uint8_t>& keyurl,
                                const std::vector<uint8_t>& indata,
                                const std::vector<uint8_t>& iv,
                                std::vector<uint8_t>* outdata,
                                int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus decryptSecure(const VmxWebClientDecryptParam& para,
            int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus destroyInstance(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus setCallback(const std::vector<uint8_t>& sessionId,
            const std::shared_ptr<IVmxWebClientCallback>& callback) override;
    ::ndk::ScopedAStatus getProperty(const std::string& in_prop,
            std::vector<uint8_t>* out_value) override;
    ::ndk::ScopedAStatus setProperty(const std::string& in_prop,
            const std::vector<uint8_t>& in_value) override;
    ::ndk::ScopedAStatus getCdmErr(int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus fetchKey(const std::vector<uint8_t>& sessionId,
            const KeyRequestParam& para,
            int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus provision(const std::vector<uint8_t>& in_request,
            int32_t* _aidl_return) override;
    ::ndk::ScopedAStatus isProvisioned(bool* _aidl_return) override;
    ::ndk::ScopedAStatus createPipeline(const PipelineParam& para,
            std::vector<uint8_t>* _aidl_return) override;
    ::ndk::ScopedAStatus destroyPipeline(const std::vector<uint8_t>& in_engineId,
            int32_t* _aidl_return) override;

    ::std::shared_ptr<IVmxWebClientCallback> mCallback;
    std::vector<uint8_t> mSessionId;
private:
    ::android::Mutex mLock;
    void *mLibHandle;
    void *mWebClientObj;
};

}  // namespace aidl::vendor::amlogic::hardware::vmx_webclient::implementation
