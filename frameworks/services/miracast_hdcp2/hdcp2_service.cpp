/*
 * Copyright (c) 2024 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#define LOG_TAG "miracast_hdcp_aidl"

#include <android-base/logging.h>
#include <android/binder_ibinder_platform.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include "HDCPService.h"

using vendor::amlogic::hardware::miracast_hdcp2::HDCPService;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(8);

    std::shared_ptr<HDCPService> service = ::ndk::SharedRefBase::make<HDCPService>();
    AIBinder_setRequestingSid(service->asBinder().get(), true);

    const std::string Instance =
            std::string() + HDCPService::descriptor + "/default";

    binder_status_t status =
            AServiceManager_addService(service->asBinder().get(), Instance.c_str());
    CHECK(status == STATUS_OK)
        << "Failed to add Miracast HDCP Factory, status=" << status;

    ABinderProcess_joinThreadPool();
}
