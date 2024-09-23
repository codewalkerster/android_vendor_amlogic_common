/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "errormonitor-jni"
#include "com_droidlogic_app_ErrorMonitorManager.h"

static sp<ErrorMonitorClient> spErrorMnt = NULL;
static jclass g_jclazz;
static jmethodID g_proc;
static JavaVM* g_vm = NULL;
static jobject g_obj = NULL;

class ErrorMonitorMsg;

static sp<ErrorMonitorMsg> gErrorMonitorMsg;

JNIEnv* attach_java_thread(const char* threadName) {
    static __thread JNIEnv* g_t_env = NULL;
    if (g_t_env != NULL) {
        // return g_t_env;
    }
    JavaVMAttachArgs args;
    jint result;
    JNIEnv* e = NULL;
    args.version = JNI_VERSION_1_4;
    args.name = (char*)threadName;
    args.group = NULL;
    if ((result = g_vm->AttachCurrentThread(&e, (void*)&args)) != JNI_OK) {
        ALOGE("NOTE: attach of thread '%s' failed\n", threadName);
        return NULL;
    }
    g_t_env = e;
    return e;
}

class ErrorMonitorMsg : public ErrorMonitorClient::ErrorMonitorCallback {
public:
    ErrorMonitorMsg() {}
    virtual ~ErrorMonitorMsg() {}
    void onReport(int32_t mainModule, int32_t submodule, int32_t level, int32_t logType, int64_t events,
                  const char* msg) {
        ALOGD("onReport ---- mainModule=%d, submodule=%d,level=%d,logType=%d,msg=%s,events=0x%02x", mainModule, submodule, level,
              logType, msg, events);
        JNIEnv* env = attach_java_thread("error_monitor");
        env->CallStaticIntMethod(g_jclazz, g_proc, g_obj, MSG_MODULE_ERROR, mainModule, submodule, level, logType,
                                 events, env->NewStringUTF(msg));
    }
    void onError(const char* msg) {
        ALOGD("onError---- msg=%s", msg);
        JNIEnv* env = attach_java_thread("error_monitor");
        int64_t temp = 1;
        env->CallStaticIntMethod(g_jclazz, g_proc, g_obj, MSG_SYSTEM_ERROR, 0, 0, 0, 0, temp, env->NewStringUTF(msg));
    }
};

static sp<ErrorMonitorClient>& getErrorMonitorClient() {
    if (spErrorMnt == NULL)
        spErrorMnt = new ErrorMonitorClient();
    return spErrorMnt;
}

static void ConnectErrorMonitor(JNIEnv* env __unused, jclass clazz __unused) {
    ALOGI("Connect Error Monitor");
}

static jint ErrorMonitorStart(JNIEnv* env, jobject, jstring jconfig) {
    sp<ErrorMonitorClient>& emc = getErrorMonitorClient();
    if (emc != NULL) {
        gErrorMonitorMsg = new ErrorMonitorMsg();
        if (gErrorMonitorMsg == NULL)
            return -1;
        emc->setErrorMonitorCallback(gErrorMonitorMsg);
        const char* config = env->GetStringUTFChars(jconfig, nullptr);
        if (!config)
            return -1;
        std::string configString = std::string(config);
        return emc->startErrorMonitor(configString);
    }
    return -1;
}

static void ErrorMonitorStartReceiver(JNIEnv* env, jobject, jobject wo) {
    jclass cls;
    if ((cls = env->FindClass("com/droidlogic/app/ErrorMonitorManager")) == NULL) {
        ALOGE("Can't find class : com/droidlogic/app/ErrorMonitorManager");
        return;
    }
    g_jclazz = (jclass)env->NewGlobalRef(cls);
    if ((g_proc = env->GetStaticMethodID(g_jclazz, "native_proc", "(Ljava/lang/Object;IIIIIJLjava/lang/String;)I")) ==
        NULL) {
        ALOGE("no such method: native_proc");
        return;
    }
    if ((g_obj = env->NewGlobalRef(wo)) == NULL) {
        return;
    }
}

static void ErrorMonitorUpdateConfig(JNIEnv* env, jobject, jstring jmonitorConfig) {
    sp<ErrorMonitorClient>& emc = getErrorMonitorClient();
    if (emc != NULL) {
        const char* config = env->GetStringUTFChars(jmonitorConfig, nullptr);
        if (!config)
            return;
        std::string configString = std::string(config);
        emc->updateMonitorConfig(configString);
    }
}

static jstring ErrorMonitorGetConfig(JNIEnv* env, jobject) {
    sp<ErrorMonitorClient>& emc = getErrorMonitorClient();
    if (emc != NULL) {
        std::string configString;
        if (emc->getMonitorConfig(configString) && !configString.empty())
            return env->NewStringUTF(configString.c_str());
    }
    return nullptr;
}

static void ErrorMonitorSetLogLevelConfig(JNIEnv* env, jobject, jstring jlogLevelConfig) {
    sp<ErrorMonitorClient>& emc = getErrorMonitorClient();
    if (emc != NULL) {
        const char* config = env->GetStringUTFChars(jlogLevelConfig, nullptr);
        if (!config)
            return;
        std::string configString = std::string(config);
        emc->setLogLevel(configString);
    }
}

static jstring ErrorMonitorGetLogLevelConfig(JNIEnv* env, jobject) {
    sp<ErrorMonitorClient>& emc = getErrorMonitorClient();
    if (emc != NULL) {
        std::string configString;
        if (emc->getLogLevel(configString) && !configString.empty())
            return env->NewStringUTF(configString.c_str());
    }
    return nullptr;
}

static void ErrorMonitorStop(JNIEnv*, jobject) {
    sp<ErrorMonitorClient>& emc = getErrorMonitorClient();
    if (emc != NULL) {
        emc->stopErrorMonitor();
    }
}

static void ErrorMonitorNotifyError(JNIEnv* env, jobject, jint subModule, jint logType, jint errorType, jint level,
                                    jstring jerrorMsg) {
    sp<ErrorMonitorClient>& emc = getErrorMonitorClient();
    if (emc != NULL) {
        const char* msg = env->GetStringUTFChars(jerrorMsg, nullptr);
        if (!msg)
            return;
        emc->notifyErrorInfo(subModule, level, logType, errorType, msg);
    }
}

static JNINativeMethod ErrorMonitor_Methods[] = {
    {"native_ConnectErrorMonitor", "()V", (void*)ConnectErrorMonitor},
    {"native_startErrorMonitor", "(Ljava/lang/String;)I", (void*)ErrorMonitorStart},
    {"native_stopErrorMonitor", "()V", (void*)ErrorMonitorStop},
    {"native_StartReceiver", "(Ljava/lang/ref/WeakReference;)V", (void*)ErrorMonitorStartReceiver},
    {"native_updateMonitorConfig", "(Ljava/lang/String;)V", (void*)ErrorMonitorUpdateConfig},
    {"native_getMonitorConfig", "()Ljava/lang/String;", (void*)ErrorMonitorGetConfig},
    {"native_setLogLevelConfig", "(Ljava/lang/String;)V", (void*)ErrorMonitorSetLogLevelConfig},
    {"native_getLogLevelConfig", "()Ljava/lang/String;", (void*)ErrorMonitorGetLogLevelConfig},
    {"native_notifyError", "(IIIILjava/lang/String;)V", (void*)ErrorMonitorNotifyError},

};

#define FIND_CLASS(var, className)   \
    var = env->FindClass(className); \
    LOG_FATAL_IF(!var, "Unable to find class " className);

#define GET_METHOD_ID(var, clazz, methodName, methodDescriptor)  \
    var = env->GetMethodID(clazz, methodName, methodDescriptor); \
    LOG_FATAL_IF(!var, "Unable to find method " methodName);

int register_com_droidlogic_app_ErrorMonitorManager(JNIEnv* env) {
    static const char* const kClassPathName = "com/droidlogic/app/ErrorMonitorManager";
    jclass clazz;
    int rc;
    FIND_CLASS(clazz, kClassPathName);

    if (clazz == NULL) {
        ALOGE("Native registration unable to find class '%s'\n", kClassPathName);
        return -1;
    }

    rc = (env->RegisterNatives(clazz, ErrorMonitor_Methods, NELEM(ErrorMonitor_Methods)));
    if (rc < 0) {
        env->DeleteLocalRef(clazz);
        ALOGE("RegisterNatives failed for '%s' %d\n", kClassPathName, rc);
        return -1;
    }

    return rc;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved __unused) {
    JNIEnv* env = NULL;
    jint result = -1;

    g_vm = vm;

    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGI("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (register_com_droidlogic_app_ErrorMonitorManager(env) < 0) {
        ALOGE("Can't register DtvkitGlueClient");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}
