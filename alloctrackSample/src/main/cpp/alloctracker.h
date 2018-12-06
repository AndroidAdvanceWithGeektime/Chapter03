#ifndef _ALLOC_TRACKER_H
#define _ALLOC_TRACKER_H

#include <dlfcn.h>
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/mman.h>
#include <fcntl.h>

#include <vector>
#include <string>
#include <atomic>
#include "lock.h"
#include "logger.h"

using std::vector;
using std::atomic;

#define ALLOC_TRACKER_ART 1
#define ALLOC_TRACKER_DVM 2


static unsigned int LUCKY = 50;

#ifdef HAVE_STDINT_H
#include <stdint.h>    /* C99 */
typedef uint8_t             u1;
#else
typedef unsigned char u1;
#endif


#define JNI_METHOD_DECL(ret_type, method_name) \
     extern "C" JNIEXPORT ret_type JNICALL Java_##com_dodola_alloctrack##_##AllocTracker##_##method_name

#ifdef __cplusplus
extern "C" {
#endif
class Class;

struct SaveAllocationData {
    jbyteArray data;
    char dataFileName[128];
};
static int allocTrackerType;
static void *libHandle = NULL;
static atomic<int> allocObjectCount(0);
static size_t setAllocRecordMax = 8000;

static bool needStopRecord = false;

//lock
static CMutex g_Lock;

class Thread;

class Object;

__attribute__((always_inline)) inline uint8_t Read1(uintptr_t addr) {
    return *reinterpret_cast<uint8_t *>(addr);
}

__attribute__((always_inline)) inline uint16_t Read2(uintptr_t addr) {
    return *reinterpret_cast<uint16_t *>(addr);
}

__attribute__((always_inline)) inline uint32_t Read4(uintptr_t addr) {
    return *reinterpret_cast<uint32_t *>(addr);
}

__attribute__((always_inline)) inline uint64_t Read8(uintptr_t addr) {
    return *reinterpret_cast<uint64_t *>(addr);
}

__attribute__((always_inline)) inline uintptr_t AccessField(
        uintptr_t addr,
        uint32_t offset) {
    return addr + offset;
}


////////////////////////////////
//22 23 使用 后面整理
static int *mallocRecordHead = NULL;
static int *mallocRecordCount = NULL;
static int *mallocRecordMax = NULL;

static char *(*GetDescriptor)(Class *, std::string *) = NULL;

static void (*artSetAllocTrackingEnable)(bool) = NULL;
static jbyteArray (*artGetRecentAllocations)() = NULL;
static void (*artEnvSetCheckJniEnabled)(bool) = NULL;
static bool (*artVmSetCheckJniEnabled)(bool) = NULL;
static bool (*artAllocMapClear)(void *) = NULL;
static int artAPILevel = 0;

// dump alloc log in art
static void (*artDbgDumpRecentAllocations)() = NULL;
static const char *storeDataDirectory;

static bool newArtRecordAllocationDoing(Class *type, size_t byte_count);
static bool newArtRecordAllocationDoing24(void *, Class *type, size_t byte_count);

//RecordAllocation(Thread* self,
//266                                            ObjPtr<mirror::Object>* obj,
//267                                            size_t byte_count)
static void (*oldArtRecordAllocation26)(void *_this, Thread *, void *obj, size_t);
static void newArtRecordAllocation26(void *_this, Thread *self, void *obj, size_t byte_count) {
    if (needStopRecord) {
        return;
    } else {
        int objptr = Read4(reinterpret_cast<uintptr_t>(obj));//此处获取的其实是一个 ref 对象
        int classRef = Read4(objptr);//根据 ref 获取真实的对象地址
        newArtRecordAllocationDoing24(_this, reinterpret_cast<Class *>(classRef), byte_count);
        oldArtRecordAllocation26(_this, self, obj, byte_count);
    }
}


static void (*oldArtRecordAllocation23)(Thread *, Class *, size_t);
static void newArtRecordAllocation23(Thread *self, Class *type, size_t byte_count) {
    if (needStopRecord) {
        return;
    } else {
        if (newArtRecordAllocationDoing(type, byte_count)) {
            oldArtRecordAllocation23(self, type, byte_count);
        }
    }
}
static void (*oldArtRecordAllocation22)(Class *, size_t);
static void newArtRecordAllocation22(Class *type, size_t byte_count) {
    if (needStopRecord) {
        return;
    } else {
        if (newArtRecordAllocationDoing(type, byte_count)) {
            oldArtRecordAllocation22(type, byte_count);
        }
    }
}

// the allocation tracker methods in Dalvik
static void (*dvmEnableAllocTracker)() = NULL;
static void (*dvmDisableAllocTracker)() = NULL;
static bool (*dvmGenerateTrackedAllocationReport)(u1 **, size_t *) = NULL;
static void (*dvmDumpTrackedAllocations)(bool) = NULL;
static u1 *dvmAllocationData;
static size_t dvmAllocationDataLen;
static void startDvmAllocationTracker();
static void stopDvmAllocationTracker();
static void dumpDvmAllocationData();
static jbyteArray getDvmAllocationDataForJava();

static void startARTAllocationTracker();
static void stopARTAllocationTracker();

static jbyteArray getAllocationData();
static jbyteArray getARTAllocationData();


static void dumpAllocationDataInLog();

void hookFunc();

#ifdef __cplusplus
}
#endif

#endif