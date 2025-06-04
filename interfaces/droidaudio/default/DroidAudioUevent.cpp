
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <cutils/uevent.h>
#include <cutils/log.h>
#include <signal.h>
#include <string.h>

#include "DroidAudioUevent.h"

#undef LOG_TAG
#define LOG_TAG "DroidAudioUEvent"

const DroidAudioUEvent::UEventItem DroidAudioUEvent::UEVENT_LIST[] = {
    {AI_SOUND, KEY_AI_SOUND},
};

DroidAudioUEvent::DroidAudioUEvent()
    : epollFd_(-1)
    , ueventFd_(-1)
    , threadId_(0)
    , shouldExit_(false)
    , mOwner(nullptr)
{
    memset(pendingEventItems_, 0, sizeof(pendingEventItems_));
}

DroidAudioUEvent::~DroidAudioUEvent() {
    close();
}

bool DroidAudioUEvent::open(void *owner, uevent_callback_t callback) {
    signal(SIGUSR1, signalHandler);

    // 打开uevent socket
    ueventFd_ = uevent_open_socket(64 * 1024, true);
    if (ueventFd_ < 0) {
        ALOGE("uevent_open_socket failed. error:%s", strerror(errno));
        return false;
    }

    if (fcntl(ueventFd_, F_SETFL, O_NONBLOCK) == -1) {
        ALOGE("fcntl ueventFd_ failed.");
        ::close(ueventFd_);
        return false;
    }

    // 创建epoll
    epollFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd_ < 0) {
        ALOGE("epoll_create failed.");
        ::close(ueventFd_);
        return false;
    }

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = ueventFd_;

    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, ueventFd_, &ev) == -1) {
        ALOGE("epoll_ctl failed.");
        ::close(epollFd_);
        ::close(ueventFd_);
        return false;
    }

    mEventCallback = callback;
    shouldExit_ = false;
    mOwner = owner;
    // 创建监听线程
    if (pthread_create(&threadId_, nullptr, threadEntry, this)) {
        ALOGE("create thread failed");
        ::close(epollFd_);
        ::close(ueventFd_);
        return false;
    }

    return true;
}

void DroidAudioUEvent::close() {
    if (threadId_ != 0) {
        shouldExit_ = true;
        pthread_kill(threadId_, SIGUSR1);
        pthread_join(threadId_, nullptr);
        threadId_ = 0;
    }

    if (ueventFd_ >= 0) {
        ::close(ueventFd_);
        ueventFd_ = -1;
    }

    if (epollFd_ >= 0) {
        ::close(epollFd_);
        epollFd_ = -1;
    }
}

void* DroidAudioUEvent::threadEntry(void* arg) {
    auto* instance = static_cast<DroidAudioUEvent*>(arg);
    instance->threadLoop();
    return nullptr;
}

void DroidAudioUEvent::threadLoop() {
    ALOGI("enter DroidAudioUEvent thread");

    while (!shouldExit_) {
        int eventNum = epoll_wait(epollFd_, pendingEventItems_,
                                EPOLL_MAX_EVENTS, -1);
        if (eventNum <= 0) {
            ALOGE("epoll_wait fails.");
            continue;
        }

        for (int i = 0; i < eventNum; i++) {
            if (pendingEventItems_[i].events & EPOLLIN) {
                processUEvent();
            }
        }
    }

    ALOGI("exit DroidAudioUEvent thread");
}

void DroidAudioUEvent::processUEvent() {
    char msg[UEVENT_MSG_LEN + 2];
    int n = uevent_kernel_multicast_recv(ueventFd_, msg, UEVENT_MSG_LEN);

    if (n <= 0 || n >= UEVENT_MSG_LEN) {
        return;
    }

    msg[n] = '\0';
    msg[n + 1] = '\0';

    for (const auto& item : UEVENT_LIST) {
        char* cp = msg;
        char* sub_str = NULL;
        while (*cp) {
            sub_str = strstr(cp, item.eventName.c_str());
            if (sub_str) {
                //ALOGD("%s() find AI Sound Uevent, Msg:%s", __func__, sub_str);
                int len = strlen(sub_str);
                if (len > 0) { //"key=value, len of value > 0"
                    std::string msg(sub_str, len);
                    if (mEventCallback) {
                        mEventCallback(mOwner, msg);
                    }
                }
            }

            while (*cp++);
        }
    }
}

void DroidAudioUEvent::signalHandler(int signum) {
    (void)signum;
}