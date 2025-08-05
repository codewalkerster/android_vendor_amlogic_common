#ifndef AML_AUDIO_UEVENT_H
#define AML_AUDIO_UEVENT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <cutils/log.h>
#include <sys/epoll.h>
#include <cutils/uevent.h>
#include <string>

using namespace std;

#define KEY_DEV_PATH_NAME   "DEVPATH"
#define KEY_AI_SOUND        "AI_SOUND_MODE"
#define KEY_MPEGH_ASI       "AUDIO_FORMAT=25"
#define KEY_MPEGH_PERSIST   "AUDIO_FORMAT=26"
#define VAL_DEV_PATH        "/devices/platform/auge_sound"

#define UEVENT_MSG_LEN          2048
#define EPOLL_MAX_EVENTS        16

enum UEventType {
    UN_KNOWN = -1,
    AI_SOUND = 0,
    MPEGH_ASI,
    MPEGH_PERSIST,
};

typedef int (*uevent_callback_t)(void *owner, std::string match);

class DroidAudioUEvent {
public:
    struct UEventItem {
        UEventType type;
        std::string eventName;
    };

    DroidAudioUEvent();
    ~DroidAudioUEvent();

    bool open(void *owner, uevent_callback_t callback);
    void close();

private:
    static void* threadEntry(void* arg);
    static void signalHandler(int signum);
    void processUEvent();
    void threadLoop();

    int epollFd_;
    int ueventFd_;
    pthread_t threadId_;
    bool shouldExit_;
    struct epoll_event pendingEventItems_[EPOLL_MAX_EVENTS];
    uevent_callback_t mEventCallback;
    void *mOwner;

    static const UEventItem UEVENT_LIST[];
};

#endif // AML_AUDIO_UEVENT_H
