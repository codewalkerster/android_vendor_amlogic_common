#define LOG_TAG "DroidMdnsOffloadHal"

#include <unistd.h>

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include "DroidMdnsOffloadHal.h"

using ::ndk::ScopedAStatus;
using ::ndk::SharedRefBase;
using vendor::amlogic::hardware::droidmdnsoffload::DroidMdnsOffloadHal;
int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    std::shared_ptr<DroidMdnsOffloadHal> vib = ndk::SharedRefBase::make<DroidMdnsOffloadHal>();
    LOG(ERROR) << "INIT DROID MDNS_OFFLOAD SERVICE";
    const std::string instance = std::string() + DroidMdnsOffloadHal::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(vib->asBinder().get(), instance.c_str());
    CHECK_EQ(status, STATUS_OK);

    ABinderProcess_joinThreadPool();
    LOG(ERROR) << "INIT DROID MDNS_OFFLOAD SERVICE FAILED";
    return EXIT_FAILURE;  // should not reach
}


