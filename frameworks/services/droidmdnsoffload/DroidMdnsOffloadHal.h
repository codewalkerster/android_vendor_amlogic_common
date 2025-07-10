#ifndef vendor_amlogic_hardware_droidmdnsoffload_DroidMdnsOffloadHal_H_
#define vendor_amlogic_hardware_droidmdnsoffload_DroidMdnsOffloadHal_H_

#include <aidl/vendor/amlogic/hardware/droidmdnsoffload/BnDroidMdnsOffload.h>
#include <android-base/expected.h>
#include <android-base/thread_annotations.h>
#include <android/binder_auto_utils.h>

#include "wifi_mdns_offload.h"


#define LOG_TAG "DroidMdnsOffloadHal"

namespace vendor {
namespace amlogic {
namespace hardware {
namespace droidmdnsoffload {

using ::aidl::vendor::amlogic::hardware::droidmdnsoffload::IDroidMdnsOffload;

class DroidMdnsOffloadHal : public aidl::vendor::amlogic::hardware::droidmdnsoffload::BnDroidMdnsOffload {
public:
    DroidMdnsOffloadHal();
    virtual ~DroidMdnsOffloadHal();
    virtual ndk::ScopedAStatus  setOffloadState(bool enabled, bool *_aidl_return) override;
    virtual ndk::ScopedAStatus  resetAll() override;
    virtual ndk::ScopedAStatus addProtocolResponses(const std::string& networkInterface,
        const std::vector<uint8_t>& rawOffloadPacket, const std::vector<int>& type, const std::vector<int>& nameOffset,
        int32_t* _aidl_return) override;
    virtual ndk::ScopedAStatus  removeProtocolResponses(int recordKey) override;
    virtual ndk::ScopedAStatus  getAndResetHitCounter(int recordKey, int32_t* _aidl_return) override;
    virtual ndk::ScopedAStatus  getAndResetMissCounter(int32_t* _aidl_return) override;
    virtual ndk::ScopedAStatus  addToPassthroughList(const std::string& networkInterface, const std::string& qname, bool* _aidl_return) override;
    virtual ndk::ScopedAStatus  removeFromPassthroughList(const std::string& networkInterface, const std::string& qname) override;
    virtual ndk::ScopedAStatus  setPassthroughBehavior(const std::string& networkInterface, IDroidMdnsOffload::PassthroughBehavior behavior) override;
    virtual ndk::ScopedAStatus  setWakePorts(int num, const std::vector<int>& protocol, const std::vector<int>& matcher, const std::vector<int>& portNum) override;
};

}
}
}
}
#endif
