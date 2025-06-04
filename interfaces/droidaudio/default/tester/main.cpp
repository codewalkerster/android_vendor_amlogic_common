#define LOG_TAG "DroidAudio_tester"
//#define LOG_NDEBUG 0
#include <string>

#include <log/log.h>
#include <android/binder_process.h>

#include "DroidAudioManager.h"


int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(8);
    ABinderProcess_startThreadPool();
    int32_t ret = 0;
    while (1) {
        ALOGI("setAudioCmdParam");
        DroidAudioManager::setAudioCmdParam(155, 0, 0, 0);
        ALOGI("setAudioCmdParam ret:%d", ret);
        sleep(2);
    }
    ABinderProcess_joinThreadPool();
    return 0;
}
