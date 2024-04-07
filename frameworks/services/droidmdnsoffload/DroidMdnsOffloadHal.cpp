#define ATRACE_TAG ATRACE_TAG_HAL

#include <DroidMdnsOffloadHal.h>
#include <android-base/result.h>
#include <android-base/stringprintf.h>
#include <android/binder_ibinder.h>
#include <utils/Log.h>
#include <utils/SystemClock.h>
#include <utils/Trace.h>
#include <android-base/logging.h>
#include <inttypes.h>
#include <set>
#include <unordered_set>

namespace vendor {
namespace amlogic {
namespace hardware {
namespace droidmdnsoffload {


namespace {

//using ::android::base::ERROR;
using ::android::base::expected;
using ::android::base::Result;
using ::android::base::StringPrintf;
using ::ndk::ScopedAIBinder_DeathRecipient;
using ::ndk::ScopedAStatus;

}

    DroidMdnsOffloadHal::DroidMdnsOffloadHal() {
        ALOGI("init DroidMdnsOffloadHal");
        int ret = wifi_mdns_offload_init();
        LOG(ERROR) << "  ret:" << ret;
    }

    DroidMdnsOffloadHal::~DroidMdnsOffloadHal() {
        ALOGI("uinit DroidMdnsOffloadHal");
        int ret = wifi_mdns_offload_deinit();
        LOG(ERROR) << "  ret:" << ret;
    }

    ndk::ScopedAStatus  DroidMdnsOffloadHal::setOffloadState(bool enabled, bool* _aidl_return) {
        LOG(ERROR) << "setOffloadState:" << enabled;
        //bool ret = setOffloadState(enabled);
        *_aidl_return = ::setOffloadState(enabled);
        return ScopedAStatus::ok();
    }
    ndk::ScopedAStatus  DroidMdnsOffloadHal::resetAll() {
        LOG(ERROR) << "resetAll";
        ::resetAll();
        return ScopedAStatus::ok();
    }


    ndk::ScopedAStatus  DroidMdnsOffloadHal::addProtocolResponses(const std::string& networkInterface,
        const std::vector<uint8_t>& rawOffloadPacket, const std::vector<int>& type, const std::vector<int>& nameOffset,
        int32_t* _aidl_return) {
        LOG(ERROR) << "addProtocolResponses, netInterface:" << networkInterface.c_str();

        int len_rawPacket = rawOffloadPacket.size();
        LOG(ERROR) << "    len_rawPacket:" << len_rawPacket;
        unsigned char raw_offload_packet[len_rawPacket+1];
        //for (const auto& rawPacket : rawOffloadPacket) {
        for (int i = 0; i < len_rawPacket; i++) {
          raw_offload_packet[i] = rawOffloadPacket[i];
        }

        int len_criterialList = type.size();
        matchCriteria matchCriteriaList[len_criterialList];
        for (int i = 0; i < len_criterialList; i++) {
          matchCriteriaList[i] = {type[i], nameOffset[i]};
          LOG(ERROR) << "    type:" << type[i] << ",nameOffset:" << nameOffset[i];
        }

        //const char* networkInterface = networkInterface.c_str();
        mdnsProtocolData mdnsProtocolData = {raw_offload_packet, (uint32_t)len_rawPacket, matchCriteriaList, (uint32_t)len_criterialList} ;
        *_aidl_return  = ::addProtocolResponses((char *)networkInterface.c_str(), &mdnsProtocolData);
        return ScopedAStatus::ok();
    }

    ndk::ScopedAStatus  DroidMdnsOffloadHal::removeProtocolResponses(int recordKey) {
        LOG(ERROR) << "removeProtocolResponses:" << recordKey;
        ::removeProtocolResponses(recordKey);
        return ScopedAStatus::ok();
    }
    ndk::ScopedAStatus  DroidMdnsOffloadHal::getAndResetHitCounter(int recordKey, int32_t* _aidl_return) {
        LOG(ERROR) << "getAndResetHitCounter:" << recordKey;
        *_aidl_return  = ::getAndResetHitCounter(recordKey);
        return ScopedAStatus::ok();
    }
    ndk::ScopedAStatus  DroidMdnsOffloadHal::getAndResetMissCounter(int32_t* _aidl_return) {
        LOG(ERROR) << "getAndResetMissCounter";
        *_aidl_return  = ::getAndResetMissCounter();
        return ScopedAStatus::ok();
    }
    ndk::ScopedAStatus  DroidMdnsOffloadHal::addToPassthroughList(const std::string& networkInterface, const std::string& qname, bool* _aidl_return) {
        LOG(ERROR) << "addToPassthroughList, netInterface:" << networkInterface.c_str() << ",qname:" << qname;
        *_aidl_return = ::addToPassthroughList((char *)networkInterface.c_str(), (char *)qname.c_str());
        return ScopedAStatus::ok();
    }
    ndk::ScopedAStatus  DroidMdnsOffloadHal::removeFromPassthroughList(const std::string& networkInterface, const std::string& qname) {
        LOG(ERROR) << "addToPassthroughList, netInterface:" << networkInterface.c_str() << ",qname:" << qname;
        ::removeFromPassthroughList((char *)networkInterface.c_str(), (char *)qname.c_str()); //ww
        return ScopedAStatus::ok();
    }
    ndk::ScopedAStatus  DroidMdnsOffloadHal::setPassthroughBehavior(const std::string& networkInterface, IDroidMdnsOffload::PassthroughBehavior behavior) {
        LOG(ERROR) << "setPassthroughBehavior, netInterface:" << networkInterface.c_str();
        ::setPassthroughBehavior((char *)networkInterface.c_str(), (passthroughBehavior)behavior  /*PASSTHROUGH_LIST*/);
        return ScopedAStatus::ok();
    }

}
}
}
}
