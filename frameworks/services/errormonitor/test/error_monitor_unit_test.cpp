/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "ErrorMonitorUnitTest"
#include <dlfcn.h>
#include <gtest/gtest.h>
#include <log/log.h>
#include <string>
#include "ErrorMonitorClient.h"
// #define TESTNOTIFYERROR
using namespace android;
class ErrorMonitorUnitTest : public ::testing::Test {
public:
    ~ErrorMonitorUnitTest() {}

    virtual void SetUp() override {
        ::testing::Test::SetUp();
#ifndef TESTNOTIFYERROR
        client = new ErrorMonitorClient();
        ASSERT_NE(client, nullptr);
#endif
    }

    virtual void TearDown() override {}

    sp<ErrorMonitorClient> client;
};

#ifndef TESTNOTIFYERROR

TEST_F(ErrorMonitorUnitTest, testMonitorConfig1) {
    ALOGD("testMonitorConfig1 begin");
    std::string config1 = "[{\"module\":1,\"level\":1},{\"module\":2,\"level\":2}]";
    int32_t ret1 = client->startErrorMonitor(config1);
    ASSERT_EQ(ret1, 0);
    std::string config2;
    bool ret2 = client->getMonitorConfig(config2);
    ASSERT_TRUE(ret2);
    ASSERT_STREQ(config1.c_str(), config2.c_str());
    client->stopErrorMonitor();
    ALOGD("testMonitorConfig1 success");
}

TEST_F(ErrorMonitorUnitTest, testMonitorConfig2) {
    ALOGD("testMonitorConfig2 begin");
    std::list<std::shared_ptr<MonitorConfig>> config1;
    for (int i = 0; i < 3; i++) {
        auto con = std::make_shared<MonitorConfig>();
        con->module = i;
        con->level = i;
        config1.push_back(con);
    }
    int32_t ret = client->startErrorMonitor(config1);
    ASSERT_EQ(ret, 0);
    std::list<std::shared_ptr<MonitorConfig>> config2;
    bool ret1 = client->getMonitorConfig(config2);
    ASSERT_TRUE(ret1);
    int j = 0;
    for (auto it = config2.begin(); it != config2.end(); ++it) {
        ASSERT_EQ((*it)->module, j);
        ASSERT_EQ((*it)->level, j);
        j++;
    }
    config1.clear();
    config2.clear();
    client->stopErrorMonitor();
    ALOGD("testMonitorConfig2 success");
}
TEST_F(ErrorMonitorUnitTest, testsetLoglevel) {
    ALOGD("testsetLoglevel begin");
    std::string configString = "Video_Dec:amvdec_mh264_v4l:0xff,amvdec_ports:0xfff;";
    int32_t ret = client->setLogLevel(configString);
    ASSERT_EQ(ret, 0);
    ALOGD("testsetLoglevel success");
}
#else
TEST_F(ErrorMonitorUnitTest, testNotfiyError) {
    ALOGD("testNotfiyError begin");
    void* libHandle = dlopen("liberrormonitorclient.so", RTLD_NOW | RTLD_NODELETE);
    ASSERT_NE(libHandle, nullptr);
    typedef void (*ErrorNotifyFun)(int, int, int, int, const char*);
    ErrorNotifyFun notify = (ErrorNotifyFun)dlsym(libHandle, "_ZN7android18AmlogicErrorNotifyEiiiiPKc");
    ASSERT_NE(notify, nullptr);
    notify(1, 1, 1, 1, "1");
    notify(2, 2, 2, 2, "2");
    ALOGD("testNotfiyError end");
}
#endif

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int status = RUN_ALL_TESTS();
    ALOGE("Test status = %d", status);
    return status;
}
