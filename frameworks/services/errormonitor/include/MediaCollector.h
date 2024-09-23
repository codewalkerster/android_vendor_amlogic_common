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
#ifndef AMLOGIC_EERRORMONITOR_MEDIACOLLECTOR_H
#define AMLOGIC_EERRORMONITOR_MEDIACOLLECTOR_H

#include <list>
#include <mutex>
#include <thread>
#include <vector>
#include "AmlMediaErrorCodes.h"
#include "AmlVideoUserdata.h"
#include "Collector.h"
namespace android {

enum VideoErrorType {
    AML_SYS_TYPE_V_BLACKSCREEN = 0,
    AML_SYS_TYPE_V_LAG = 1,
    AML_SYS_TYPE_V_FLOWER_SCREEN = 2,
    AML_SYS_TYPE_V_FREEZE = 3,
    AML_SYS_TYPE_V_SIZE_ABNORMAL = 4,
    AML_SYS_TYPE_V_CHANGE_CH_SLOW = 5,
    AML_SYS_TYPE_V_ENTER_SCREENSAVER = 6,
    AML_SYS_TYPE_V_UNKNOWN = 7,
};

typedef int32_t (*MPConsumer_initialize)(void);
typedef int32_t (*MPConsumer_register_msg_type)(int fd, int msgType);
typedef int32_t (*MPConsumer_read_data)(int32_t fd, struct aml_video_user_data* userData);
typedef int32_t (*MPConsumer_destroy)(int32_t fd);

class MediaProxyWrapper {

public:
    MediaProxyWrapper();
    virtual ~MediaProxyWrapper();
    bool start(CollectedDataCallback* cb);
    void stop();

    MPConsumer_initialize mpconsumer_initialize;
    MPConsumer_register_msg_type mpconsumer_register_msg_type;
    MPConsumer_read_data mpconsumer_read_data;
    MPConsumer_destroy mpconsumer_destroy;

private:
    void* mProxyHandle;
    CollectedDataCallback* mCallBack;
    void threadFunc();
    void processUserData(struct aml_video_user_data* userData);
    bool mStarted;
    int32_t mProxyFd;
    std::mutex mLock;
    std::vector<std::thread> mThreadPool;
    std::condition_variable mCondition;
};

class MediaCollector : public Collector {
public:
    MediaCollector();

    virtual ~MediaCollector();

    bool start();

    void stop();

private:
    void onCollectWork();
    void threadFunc();
    void onCollect(std::unique_ptr<CollectedData>& data);
    void onError(const std::string& msg);
    std::mutex mLock;
    std::list<std::unique_ptr<CollectedData>> mErrorDataQueue;
    bool mStarted;
    std::vector<std::thread> mThreadPool;
    std::condition_variable mCondition;
    std::unique_ptr<MediaProxyWrapper> mMediaProxyWrapper;
};

}; // namespace android

#endif