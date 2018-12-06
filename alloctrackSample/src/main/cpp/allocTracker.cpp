
#include "alloctracker.h"
#include "dlopen.h"
#include "Substrate/SubstrateHook.h"
//#include "HookZz/include/hookzz.h"
#include <pthread.h>
#include <ctime>
#include <cstdlib>
#include "fb/fbjni/fbjni.h"

pthread_t pthread_self(void);

#ifdef __cplusplus
extern "C" {
#endif
static int randomDouble(double start, double end) {
    return static_cast<int>(start + (end - start) * rand() / (RAND_MAX + 1.0));
}
static int randomInt(int start, int end) {
    return (int) randomDouble(start, end);
}

/**
 * start alloc tracker
 */
JNI_METHOD_DECL(void, startAllocationTracker)
(JNIEnv *env, jobject jref) {
    if (allocTrackerType == ALLOC_TRACKER_ART) {
        startARTAllocationTracker();
    } else if (allocTrackerType == ALLOC_TRACKER_DVM) {
        startDvmAllocationTracker();
    }
}

/**
 * stop
 */
JNI_METHOD_DECL(void, stopAllocationTracker)
(JNIEnv *env, jobject jref) {
    LOGI("stopAllocationTracker, allocTrackerType: %d", allocTrackerType);
    if (allocTrackerType == ALLOC_TRACKER_ART) {
        stopARTAllocationTracker();
    } else if (allocTrackerType == ALLOC_TRACKER_DVM) {
        stopDvmAllocationTracker();
    }
}

//void beforeRecordAllocation(RegisterContext *reg_ctx, const HookEntryInfo *info) {
//    int objptr = Read4(reg_ctx->general.regs.r2);//此处获取的其实是一个 ref 对象
//    int classRef = Read4(objptr);//根据 ref 获取真实的对象地址
//    //r3是 bytecount
//    //r0寄存器是 this
//    //r2是 class
//    newArtRecordAllocationDoing24((void *) reg_ctx->general.regs.r0,
//                                  reinterpret_cast<Class *>(classRef),
//                                  reg_ctx->general.regs.r3);
//}

/// methods for art
//force dlopen 需要等待 env 构造完成
//static __attribute__((__constructor__))
void hookFunc() {
    LOGI("start hookFunc");
    void *handle = ndk_dlopen("libart.so", RTLD_LAZY | RTLD_GLOBAL);

    if (!handle) {
        LOGE("libart.so open fail");
        return;
    }
    void *hookRecordAllocation26 = ndk_dlsym(handle,
                                             "_ZN3art2gc20AllocRecordObjectMap16RecordAllocationEPNS_6ThreadEPNS_6ObjPtrINS_6mirror6ObjectEEEj");

    void *hookRecordAllocation24 = ndk_dlsym(handle,
                                             "_ZN3art2gc20AllocRecordObjectMap16RecordAllocationEPNS_6ThreadEPPNS_6mirror6ObjectEj");

    void *hookRecordAllocation23 = ndk_dlsym(handle,
                                             "_ZN3art3Dbg16RecordAllocationEPNS_6ThreadEPNS_6mirror5ClassEj");

    void *hookRecordAllocation22 = ndk_dlsym(handle,
                                             "_ZN3art3Dbg16RecordAllocationEPNS_6mirror5ClassEj");

    //此处说明一下26和24版本需要使用 hookzz 的原因。
    //hookzz 框架有个优势是可以获取方法进入时候的寄存器内容，而很多时候我们要根据r0来获取 this
    if (hookRecordAllocation26 != nullptr) {
        LOGI("Finish get symbol26");
//        ZzWrap((void *) hookRecordAllocation26, beforeRecordAllocation, nullptr);
        MSHookFunction(hookRecordAllocation26, (void *) &newArtRecordAllocation26,
                       (void **) &oldArtRecordAllocation26);

    } else if (hookRecordAllocation24 != nullptr) {
        LOGI("Finish get symbol24");
//        ZzWrap((void *) hookRecordAllocation24, beforeRecordAllocation, nullptr);
        MSHookFunction(hookRecordAllocation26, (void *) &newArtRecordAllocation26,
                       (void **) &oldArtRecordAllocation26);

    } else if (hookRecordAllocation23 != NULL) {
        LOGI("Finish get symbol23");
        MSHookFunction(hookRecordAllocation23, (void *) &newArtRecordAllocation23,
                       (void **) &oldArtRecordAllocation23);
    } else {
        LOGI("Finish get symbol22");
        if (hookRecordAllocation22 == NULL) {
            LOGI("error find hookRecordAllocation22");
            return;
        } else {
            MSHookFunction(hookRecordAllocation22, (void *) &newArtRecordAllocation22,
                           (void **) &oldArtRecordAllocation22);
        }
    }
    dlclose(handle);
}


/**
 * art 初始化
 * apilevel 系统版本
 * allocRecordmax是对象分配数量的最大值
 */
JNI_METHOD_DECL(jint, initForArt)
(JNIEnv *env, jobject jref, jint apiLevel, jint allocRecordMax) {

    srand(unsigned(time(0)));
    allocTrackerType = ALLOC_TRACKER_ART;
    libHandle = ndk_dlopen("libart.so", RTLD_LOCAL);
    if (libHandle == NULL) {
        LOGI("initForArt, dlopen libart failed!");
        return -2;
    }

    LOGI("set allocRecordMax: %d", allocRecordMax);
    setAllocRecordMax = allocRecordMax;

    artDbgDumpRecentAllocations = (void (*)()) (ndk_dlsym(libHandle,
                                                          "_ZN3art3Dbg21DumpRecentAllocationsEv"));

    if (GetDescriptor == nullptr) {
        GetDescriptor = (char *(*)(Class *, std::string *)) ndk_dlsym(libHandle,
                                                                      "_ZN3art6mirror5Class13GetDescriptorEPNSt3__112basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEE");//根据 class 获取类名
    }

    artSetAllocTrackingEnable = (void (*)(bool)) ndk_dlsym(libHandle,
                                                           "_ZN3art3Dbg23SetAllocTrackingEnabledEb");//开启 alloc tracking 开启成功后才能执行后续操作


    if (artSetAllocTrackingEnable == nullptr) {
        LOGI("find artSetAllocTrackingEnable failed");
    }
    artGetRecentAllocations = (jbyteArray(*)()) ndk_dlsym(libHandle,
                                                          "_ZN3art3Dbg20GetRecentAllocationsEv");//重要方法，dump alloc 里的对象转换成 byte 数据
    if (artGetRecentAllocations == nullptr) {
        LOGI("find artGetRecentAllocations failed");
    }
    artAllocMapClear = (bool (*)(void *)) (ndk_dlsym(libHandle,
                                                     "_ZN3art2gc20AllocRecordObjectMap5ClearEv"));//清理 alloc 中已存在的对象
    if (artAllocMapClear == nullptr) {
        LOGI("find artAllocMapClear failed");
    }

    artAPILevel = apiLevel;
    return JNI_OK;
}

JNI_METHOD_DECL(void, unInitForArt)
(JNIEnv *env, jobject jref) {
    LOGI("unInitForArt, libHandle==NULL: %s",
         libHandle == NULL ? "true" : "false");
    if (libHandle != NULL) {
        dlclose(libHandle);
    }
    allocTrackerType = 0;
    libHandle = NULL;
    artSetAllocTrackingEnable = NULL;
    artGetRecentAllocations = NULL;
    artAPILevel = 0;
}

JNI_METHOD_DECL(void, setSaveDataDirectory)
(JNIEnv *env, jobject jref, jstring directory) {
    storeDataDirectory = env->GetStringUTFChars(directory, 0);
}


static void startARTAllocationTracker() {
    LOGI(ALLOC_TRACKER_TAG,
         "art, startAllocationTracker, func==NULL: %s, artEnvSetCheckJniEnabled==NULL: %s",
         artEnvSetCheckJniEnabled == NULL ? "true" : "false",
         artVmSetCheckJniEnabled == NULL ? "true" : "false");

    if (artSetAllocTrackingEnable != NULL) {
        artSetAllocTrackingEnable(true);
    }
    allocObjectCount = {0};
}

static void stopARTAllocationTracker() {
    LOGI("art, stopAllocationTracker, func==NULL: %s, objectCount: %d",
         artSetAllocTrackingEnable == NULL ? "true" : "false", allocObjectCount.load());
    if (artSetAllocTrackingEnable != NULL) {
        artSetAllocTrackingEnable(false);
    }
    allocObjectCount = {0};
}

static int getARTAllocRecordCount() {
    if (libHandle == NULL || allocTrackerType != ALLOC_TRACKER_ART) {
        return 0;
    }
    if (mallocRecordCount == NULL) {
        mallocRecordCount = (int *) dlsym(libHandle, "_ZN3art3Dbg19alloc_record_count_E");
    }
    if (mallocRecordCount != NULL) {
        return *mallocRecordCount;
    }
    return 0;
}
static int getARTAllocRecordMax() {
    if (libHandle == NULL || allocTrackerType != ALLOC_TRACKER_ART) {
        return 0;
    }
    if (mallocRecordMax == NULL) {
        mallocRecordMax = (int *) ndk_dlsym(libHandle, "_ZN3art3Dbg17alloc_record_max_E");
    }
    if (mallocRecordMax != NULL) {
        return *mallocRecordMax;
    } else {
        return 10 * 1024;
    }
    return 0;
}
/**
 * 22 23使用
 * @return
 */
static int getARTAllocRecordHead() {
    if (libHandle == NULL || allocTrackerType != ALLOC_TRACKER_ART) {
        return 0;
    }

    if (mallocRecordHead == NULL) {
        mallocRecordHead = (int *) ndk_dlsym(libHandle, "_ZN3art3Dbg18alloc_record_head_E");
    }
    if (mallocRecordHead != NULL) {
        return *mallocRecordHead;
    }
    return 0;
}
static void resetARTAllocRecord() {
    if (libHandle == NULL || allocTrackerType != ALLOC_TRACKER_ART) {
        return;
    }

    int a = getARTAllocRecordHead();
    int b = getARTAllocRecordCount();
    int c = getARTAllocRecordMax();

    if (mallocRecordHead != NULL && mallocRecordCount != NULL) {
        *mallocRecordHead = 0;
        *mallocRecordCount = 0;
    }
}
/**
 * 基本art 都适配 生成的 byte array 格式如下
 * Message header (all values big-endian):
 * (1b) message header len (to allow future expansion); includes itself
 * (1b) entry header len
 * (1b) stack frame len
 * (2b) number of entries
 * (4b) offset to string table from start of message
 * (2b) number of class name strings
 * (2b) number of method name strings
 * (2b) number of source file name strings
 * For each entry:
 *   (4b) total allocation size
 *   (2b) thread id
 *   (2b) allocated object's class name index
 *   (1b) stack depth
 *   For each stack frame:
 *     (2b) method's class name
 *     (2b) method name
 *     (2b) method source file
 *     (2b) line number, clipped to 32767; -2 if native; -1 if no source
 * (xb) class name strings
 * (xb) method name strings
 * (xb) source file strings
 */
jbyteArray getARTAllocationData() {
    if (artGetRecentAllocations != NULL) {

        jbyteArray data = artGetRecentAllocations();
        LOGI("artGetRecentAllocations finished");
        if (data != NULL) {
            LOGI("artGetRecentAllocations success");
            return data;
        } else {
            LOGI("artGetRecentAllocations failed");
        }
    }
    return NULL;
}

JNI_METHOD_DECL(void, setEnableARTRecordAllocation)
(JNIEnv *env, jobject jref, bool enable) {
    needStopRecord = !enable;
}

/// methods for dvm
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

JNI_METHOD_DECL(jint, initForDvm)
(JNIEnv *env, jobject jref) {
    allocTrackerType = ALLOC_TRACKER_DVM;
    libHandle = ndk_dlopen("libdvm.so", RTLD_LAZY);
    if (libHandle == nullptr) {
        LOGI("initForDvm, dlopen libdvm failed!");
        return -2;
    }

    dvmEnableAllocTracker = (void (*)()) dlsym(libHandle, "_Z21dvmEnableAllocTrackerv");
    dvmDisableAllocTracker = (void (*)()) dlsym(libHandle, "_Z22dvmDisableAllocTrackerv");
    dvmGenerateTrackedAllocationReport = (bool (*)(u1 **, size_t *)) dlsym(libHandle,
                                                                           "_Z34dvmGenerateTrackedAllocationReportPPhPj");
    dvmDumpTrackedAllocations = (void (*)(bool)) dlsym(libHandle, "_Z25dvmDumpTrackedAllocationsb");

    if (dvmEnableAllocTracker != nullptr && dvmDisableAllocTracker != nullptr &&
        dvmGenerateTrackedAllocationReport != nullptr && dvmDumpTrackedAllocations != nullptr) {
        LOGI("initForDvm success");
        return 0;
    } else {
        LOGI("initForDvm failed");
        return -2;
    }
}

JNI_METHOD_DECL(void, unInitForDvm)
(JNIEnv *env, jobject jref) {
    LOGI("unInitForDvm, libHandle==NULL: %s",
         libHandle == nullptr ? "true" : "false");
    if (libHandle != nullptr) {
        dlclose(libHandle);
    }
    free(dvmAllocationData);
    dvmAllocationData = nullptr;
    dvmAllocationDataLen = 0;
    allocTrackerType = 0;
    dvmEnableAllocTracker = nullptr;
    dvmDisableAllocTracker = nullptr;
    dvmGenerateTrackedAllocationReport = nullptr;
    dvmDumpTrackedAllocations = nullptr;
}

static void startDvmAllocationTracker() {
    LOGI("dvm, startAllocationTracker, func==NULL: %s",
         dvmEnableAllocTracker == nullptr ? "true" : "false");
    if (dvmEnableAllocTracker != nullptr) {
        dvmEnableAllocTracker();
    }
}

JNI_METHOD_DECL(void, dumpDvmAllocationData)
(JNIEnv *env, jobject jref) {
    if (dvmGenerateTrackedAllocationReport != nullptr) {
        LOGI("dumpDvmAllocationData, use dvm");
        bool result = dvmGenerateTrackedAllocationReport(&dvmAllocationData, &dvmAllocationDataLen);
        LOGI("dvmGenerateTrackedAllocationReport %s",
             result ? "success" : "failed");
        if (result) {
            LOGI("dvmGenerateTrackedAllocationReport result len: %d, data: 0x%x",
                 dvmAllocationDataLen, dvmAllocationData);
        }
    }
}

static void stopDvmAllocationTracker() {
    LOGI("dvm, stopAllocationTracker, func==NULL: %s",
         dvmDisableAllocTracker == NULL ? "true" : "false");
    if (dvmDisableAllocTracker != NULL) {
        dvmDisableAllocTracker();
    }
}
JNI_METHOD_DECL(jbyteArray, getDvmAllocationDataForJava)
(JNIEnv *env, jobject jref) {
    LOGI("getDvmAllocationDataForJava data: 0x%x, dataLen", dvmAllocationData,
         dvmAllocationDataLen);
    if (dvmAllocationData != nullptr && dvmAllocationDataLen > 0) {
        jbyteArray result = env->NewByteArray(dvmAllocationDataLen);
        if (result != nullptr) {
            env->SetByteArrayRegion(result, 0, dvmAllocationDataLen,
                                    reinterpret_cast<const jbyte *>(dvmAllocationData));
        }
        free(dvmAllocationData);
        dvmAllocationDataLen = 0;
        return result;
    }
    return nullptr;
}

JNI_METHOD_DECL(void, dumpAllocationDataInLog)
(JNIEnv *env, jobject jref) {
    LOGI("start dumpAllocationDataInLog, allocTrackerType: %d",
         allocTrackerType);
    if (allocTrackerType == ALLOC_TRACKER_ART) {
        if (artDbgDumpRecentAllocations != nullptr) {
            artDbgDumpRecentAllocations();
        }
    } else if (allocTrackerType == ALLOC_TRACKER_DVM) {
        if (dvmDumpTrackedAllocations != nullptr) {
            dvmDumpTrackedAllocations(false);
        }
    }
}
///////////////////////////end dalivk//////////////////////////////

bool saveARTAllocationData(SaveAllocationData saveData) {
    JNIEnv *env = facebook::jni::Environment::current();
    {
        snprintf(saveData.dataFileName, 1024, "%s/%d", storeDataDirectory,
                 static_cast<int>(time(0)));
        int fd = open(saveData.dataFileName, O_RDWR | O_CREAT | O_CLOEXEC, (mode_t) 0644);
        lseek(fd, 0, SEEK_SET);
        LOGI("saveARTAllocationData %s file, fd: %d", saveData.dataFileName, fd);
        size_t dataSize = env->GetArrayLength(saveData.data);
        jbyte *olddata = (jbyte *) env->GetByteArrayElements(saveData.data, 0);
        write(fd, olddata, dataSize);
        close(fd);
        LOGI("saveARTAllocationData write file to %s", saveData.dataFileName);
    }
    return true;
}


std::string a;
/**
 * @param type
 * @param byte_count
 * @return
 */
static bool newArtRecordAllocationDoing(Class *type, size_t byte_count) {

    allocObjectCount++;

    char *typeName = GetDescriptor(type, &a);
//    LOGI("=====class name:%s,allocbyte:%d", typeName,byte_count);// 如果只关心分配的对象大小的话，可以不用做alloc dump 的操作
    //达到 max
    int randret = randomInt(0, 100);
    if (randret == LUCKY) {
        LOGI("====current alloc count %d=====", allocObjectCount.load());
        return false;
    }
    if (allocObjectCount > setAllocRecordMax) {
        CMyLock lock(g_Lock);//此处需要 loc 因为对象分配的时候不知道在哪个线程，不 lock 会导致重复 dump
        allocObjectCount = 0;

        //write alloc data to file
        jbyteArray allocData = getARTAllocationData();
        SaveAllocationData saveData{allocData};
        saveARTAllocationData(saveData);
        resetARTAllocRecord();
        LOGI("===========CLEAR ALLOC MAPS=============");

        lock.Unlock();
    }
    return true;
}

/**
 * 24以上版本和以下版本上由于 alloc list 的结构不同导致调用方式也不同
 * @param _this
 * @param type
 * @param byte_count
 * @return
 */
static bool newArtRecordAllocationDoing24(void *_this, Class *type, size_t byte_count) {

    if (artAllocMapClear == nullptr) {//如果无法主动 clear 对象，那么下面的逻辑会导致 dump 下来的对象重复
        return false;
    }

    allocObjectCount++;

    char *typeName = GetDescriptor(type, &a);
//    LOGI("=====class name:%s,allocbyte:%d", typeName,byte_count);// 如果只关心分配的对象大小的话，可以不用做alloc dump 的操作
    //达到 max
    int randret = randomInt(0, 100);
    if (randret == LUCKY) {
        LOGI("====current alloc count %d=====", allocObjectCount.load());
        return false;
    }
//    int artAllocMax = getARTAllocRecordMax();
    if (allocObjectCount > setAllocRecordMax) {
        CMyLock lock(g_Lock);
        allocObjectCount = 0;

        //write alloc data to file
        jbyteArray allocData = getARTAllocationData();
        SaveAllocationData saveData{allocData};
        saveARTAllocationData(saveData);
        //TODO:clear alloc data
        if (artAllocMapClear != nullptr) {
            artAllocMapClear(_this);
            LOGI("===========CLEAR ALLOC MAPS=============");
        }

        lock.Unlock();
    }
    return true;
}

#ifdef __cplusplus
}
#endif
