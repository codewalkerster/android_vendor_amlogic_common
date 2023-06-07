/*
 * Copyright (C) 2011 The Android Open Source Project
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
 *  @author   Tellen Yu
 *  @version  2.0
 *  @date     2019/09/12
 *  @par function description:
 *  - 1 open vad related devices first before enter freeze mode
 */

#define LOG_TAG "vadservice"

#include <android-base/logging.h>
#include <cutils/properties.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <tinyalsa/asoundlib.h>

extern "C" {
#include "aml_alsa_mixer.h"
#include "aml_vad_wakeup.h"
#include "alsa_device_parser.h"
}

#define ANDROID_RB_PROPERTY "sys.powerctl"

#define PATH_CMD_LINE "/proc/cmdline"
#define PATH_REBOOT_REASON "/sys/devices/platform/reboot/reboot_reason"
#define PATH_FREEZE "/sys/power/state"
#define PATH_HDCP_ESM "/sys/module/tvin_hdmirx/parameters/hdcp22_kill_esm"

#define BUF_LEN_MAX    (4096)
#define BUF_LEN_NORMAL    (32)

typedef struct VadCaptureConfig {
    int card;
    char source;
    char device;
    char channel;
    int rate;
} VadConfig;

static int                     g_kernel_log_fd = -1;

static pthread_condattr_t      g_cond_attr;
static pthread_cond_t          g_cond_signal;
static pthread_mutex_t         g_cond_mutex;
static pthread_t               g_thread_id;
static struct pcm*             g_pcm_handle = NULL;
static unsigned int            g_hread_exit = 0;

static void VadAborter(const char* abort_message) {
    android::base::DefaultAborter(abort_message);
}

void VadKernelLogging(char* argv[]) {
    // Make stdin/stdout/stderr all point to /dev/null.
    g_kernel_log_fd = open("/sys/fs/selinux/null", O_RDWR);
    if (g_kernel_log_fd == -1) {
        int saved_errno = errno;
        android::base::InitLogging(argv, &android::base::KernelLogger, VadAborter);
        errno = saved_errno;
        PLOG(FATAL) << "Couldn't open /sys/fs/selinux/null";
        return;
    }
    dup2(g_kernel_log_fd, 0);
    dup2(g_kernel_log_fd, 1);
    dup2(g_kernel_log_fd, 2);
    if (g_kernel_log_fd > 2) close(g_kernel_log_fd);

    android::base::InitLogging(argv, &android::base::KernelLogger, VadAborter);
}

static void ts_wait_time(struct timespec *ts, uint32_t time_us) {
    clock_gettime(CLOCK_MONOTONIC, ts);
    ts->tv_sec += time_us / 1000000L;
    ts->tv_nsec += (time_us * 1000) % 1000000000LL;
    if (ts->tv_nsec >= 1000000000LL) {
        ts->tv_sec++;
        ts->tv_nsec -=1000000000LL;
    }
}

static void *vadThreadloop(void *arg __unused) {
    unsigned int b_pdm_is_ready = 0;
    unsigned int size = pcm_frames_to_bytes(g_pcm_handle, pcm_get_buffer_size(g_pcm_handle));
    char *buffer = (char *)calloc(1, size);
    prctl(PR_SET_NAME, (unsigned long)"vadThreadloop");
    /* You need to keep the PDM working at all times. Otherwise, alsa XRUN will cause the
     * vad buffer to fail to be switched before system suspend.
     */
    do {
        int ret = pcm_read(g_pcm_handle, buffer, size);
        if (ret == 0 && b_pdm_is_ready == 0) {
            pthread_mutex_lock(&g_cond_mutex);
            pthread_cond_signal(&g_cond_signal);
            pthread_mutex_unlock(&g_cond_mutex);
            b_pdm_is_ready = 1;
        }
        usleep(10000);
    } while (!g_hread_exit);
    free(buffer);
    return NULL;
}

static void vadTinycapStop(void) {
    g_hread_exit = 1;
    pthread_join(g_thread_id, NULL);
    if (g_pcm_handle) {
        pcm_close(g_pcm_handle);
        g_pcm_handle = NULL;
    } else {
        fprintf(stderr, "%s already stop!\n", __func__);
        return;
    }
    pthread_cond_destroy(&g_cond_signal);
}

static int vadTinycapStart(int card, int device, int rate, int channels) {
    struct pcm_config config;
    memset(&config, 0, sizeof(config));
    config.channels = channels;
    config.rate = rate;
    config.period_size = 1024;
    config.period_count = 4;
    config.format = PCM_FORMAT_S16_LE;

    pthread_condattr_init(&g_cond_attr);
    pthread_mutex_init(&g_cond_mutex, NULL);
    pthread_condattr_setclock(&g_cond_attr, CLOCK_MONOTONIC);
    pthread_cond_init(&g_cond_signal, &g_cond_attr);

    g_pcm_handle = pcm_open(card, device, PCM_IN, &config);
    if (!g_pcm_handle || !pcm_is_ready(g_pcm_handle)) {
        fprintf(stderr, "Unable to open PCM device (%s)\n", pcm_get_error(g_pcm_handle));
        return -1;
    }

    g_hread_exit = 0;
    pthread_mutex_lock(&g_cond_mutex);
    int ret = pthread_create(&g_thread_id, NULL, &vadThreadloop, NULL);
    if (ret != 0) {
        fprintf(stderr, "%s err\n", __func__);
        pthread_mutex_unlock(&g_cond_mutex);
        pcm_close(g_pcm_handle);
        g_pcm_handle = NULL;
        return -1;
    }
    /* 1.Wait for PDM to be ready; 2. Timeout 200ms. */
    struct timespec ts;
    ts_wait_time(&ts, 200000);
    pthread_cond_timedwait(&g_cond_signal, &g_cond_mutex, &ts);
    pthread_mutex_unlock(&g_cond_mutex);
    return 0;
}

void writeSys(const char *path, const char *val) {
    int fd;

    if ((fd = open(path, O_RDWR)) < 0) {
        LOG(android::base::ERROR) << "writeSysFs, open fail:" << path;
        return;
    }
    write(fd, val, strlen(val));
    close(fd);
}

int readSys(const char *path, char *buf, int count) {
    int fd, len = -1;

    if (NULL == buf) {
        LOG(android::base::ERROR) << "readSys, buf is null";
        return len;
    }

    if ((fd = open(path, O_RDONLY)) < 0) {
        LOG(android::base::ERROR) << "readSys, open fail: " << path << ", error=" << strerror(errno);
        return len;
    }

    len = read(fd, buf, count - 1);
    if (len < 0 || len > count - 1) {
        LOG(android::base::ERROR) << "readSys, len:%d"<< len << ", read error: " << path << ", error=" << strerror(errno);
        len = -1;
    } else {
        buf[len] = '\0';
    }
    close(fd);
    return len;
}

int processBuffer(char *t_buf, char split_ch, char *t_name, char *t_value) {
    char *tmp_ptr = NULL, *tmp_name = NULL, *tmp_value = NULL;

    tmp_ptr = t_buf;
    while (tmp_ptr && *tmp_ptr) {
        char *x = strchr(tmp_ptr, split_ch);
        if (x != NULL) {
            *x++ = '\0';
        }
        tmp_name = tmp_ptr;
        if (tmp_name[0] != '\0') {
            tmp_value = strchr(tmp_ptr, '=');
            if (tmp_value != NULL) {
                *tmp_value++ = '\0';
                if (strncmp(tmp_name, t_name, strlen(t_name)) == 0) {
                    strncpy(t_value, tmp_value, BUF_LEN_NORMAL);
                    return 0;
                }
            }
        }
        tmp_ptr = x;
    }
    return -1;
}

int processReadFile(char *file_path, int offset, char *pBuf, int len) {
    int tmp_cnt = 0;
    int dev_fd = open(file_path, O_RDONLY);
    if (dev_fd >= 0) {
        off_t ret = lseek(dev_fd, offset, SEEK_SET);
        if (ret == -1) {
            LOG(android::base::ERROR) << "processReadFile lseek fail";
            close(dev_fd);
            return 0;
        }
        tmp_cnt = read(dev_fd, pBuf, len);
        if (tmp_cnt < 0)
            tmp_cnt = 0;
        /* get rid of trailing newline, it happens */
        if (tmp_cnt > 0 && pBuf[tmp_cnt - 1] == '\n')
            tmp_cnt--;
        pBuf[tmp_cnt] = 0;
        close(dev_fd);
    } else {
        pBuf[0] = 0;
    }

    return tmp_cnt;
}

int processFile(char *t_fname, char split, char *t_name, char *t_value) {
    char line_buf[BUF_LEN_MAX];

    processReadFile(t_fname, 0, line_buf, BUF_LEN_MAX);
    return processBuffer(line_buf, split, t_name, t_value);
}

void vadReboot(void) {
    LOG(android::base::ERROR) << "ffv normal reboot";
    if (true) {
        writeSys(PATH_HDCP_ESM, "1");
        usleep(50 * 1000);
        syscall(__NR_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
                LINUX_REBOOT_CMD_RESTART2, "normal");
    } else {
        property_set(ANDROID_RB_PROPERTY, "normal");
    }
}

bool isCoolboot() {
    char buf[BUF_LEN_NORMAL] = { 0 };

    int ret = readSys(PATH_REBOOT_REASON, buf, BUF_LEN_NORMAL);\
    if (ret < 0) {
        LOG(android::base::ERROR) << "isCoolboot: readSys fail.";
        return false;
    }
    LOG(android::base::WARNING) << "reboot reason is " << buf;
    if (strncmp(buf, "0", 1) == 0) {    // cold boot
        return true;
    } else if (strncmp(buf, "15", 2) == 0) { // ffv reboot
        return true;
    }
    return false;
}

bool isFFVFreezeMode() {
    char buf[BUF_LEN_NORMAL] = { 0 };

    memset(buf, 0, BUF_LEN_NORMAL);
    if (processFile((char*)PATH_CMD_LINE, ' ', (char*)"ffv_freeze", buf) == 0) {
        if (strncmp(buf, "on", 2) == 0)
            return true;
    }
    return false;
}

static void getVadConfig(VadConfig* vadconfig) {
    vadconfig->source = property_get_int32(AML_AUDIO_VAD_SOURCE_PROP, 4);
    vadconfig->device = property_get_int32(AML_AUDIO_VAD_DEVICE_PROP, 3);
    vadconfig->channel = property_get_int32(AML_AUDIO_VAD_CHANNEL_PROP, 1);
    vadconfig->rate = property_get_int32(AML_AUDIO_VAD_RATE_PROP, 16000);
}

int main(int argc __attribute__((unused)), char **argv __attribute__((unused))) {
    VadConfig tmpConfig = {};
    bool is_snd_dev_ok = false;

    VadKernelLogging(argv);
    LOG(android::base::ERROR) << "VadService starting...";

    tmpConfig.card = -1;
    if (/*isCoolboot() && */isFFVFreezeMode()) {
        int tmp_cnt = 0;

        LOG(android::base::ERROR) << "VadService ffv freeze mode";
        getVadConfig(&tmpConfig);
        while (1) {
            if (tmpConfig.card == -1) {
                tmpConfig.card = alsa_device_get_card_index();
            } else {
                if (is_snd_dev_ok == false) {
                    char fn[256];
                    snprintf(fn, sizeof(fn), "/dev/snd/controlC%u", tmpConfig.card);
                    if (access(fn, R_OK | W_OK) == 0) {
                        is_snd_dev_ok = true;
                    } else {
                        LOG(android::base::ERROR) << "VadService dev: " << fn << " not found.";
                    }
                }
            }
            if (tmpConfig.card != -1 && is_snd_dev_ok) {
                break;
            }
            LOG(android::base::INFO) << "VadService get card again ...";
            if (tmp_cnt++ >= 100) {
                LOG(android::base::ERROR) << "VadService try get card timeout 10s ...";
                return 0;
            }
            usleep(100 * 1000);
        }

        LOG(android::base::ERROR) << "VadService card id: " << tmpConfig.card;
        struct aml_mixer_handle alsa_mixer;
        memset(&alsa_mixer, 0, sizeof(struct aml_mixer_handle));
        open_mixer_handle(&alsa_mixer);
        aml_mixer_ctrl_set_int(&alsa_mixer, AML_MIXER_ID_VAD_ENABLE, 1);
        aml_mixer_ctrl_set_int(&alsa_mixer, AML_MIXER_ID_VAD_SOURCE_SEL, tmpConfig.source);

        vadTinycapStart(tmpConfig.card, tmpConfig.device, tmpConfig.rate, tmpConfig.channel);
        aml_mixer_ctrl_set_int(&alsa_mixer, AML_MIXER_ID_VAD_SWITCH, 1);//enable the vad engine to get data from ddr

        LOG(android::base::ERROR) << "VadService enter the freeze mode.";
        writeSys(PATH_FREEZE, "freeze"); // block here
        vadTinycapStop();
        LOG(android::base::ERROR) << "VadService exit, and reboot";
        vadReboot();
    } else {
        LOG(android::base::ERROR) << "VadService exit. Normal startup.";
    }
    if (g_kernel_log_fd != -1) {
        close(g_kernel_log_fd);
    }
    return 0;
}

