#include <jni.h>
#include <string>
#include "fbjni/fbjni.h"
#include "dlopen.h"
#include "alloctracker.h"
#include "jvmti.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_dodola_alloctrack_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {

    return facebook::jni::initialize(vm, [] {
        ndk_init(facebook::jni::Environment::current());
        hookFunc();
    });

}
