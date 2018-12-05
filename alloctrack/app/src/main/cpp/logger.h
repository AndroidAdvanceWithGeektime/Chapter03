//
// Created by baidu on 2018-11-15.
//

#ifndef ALLOCTRACK_LOGGER_H
#define ALLOCTRACK_LOGGER_H
#include <android/log.h>

#define ALLOC_TRACKER_TAG "AllocTracker"

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, ALLOC_TRACKER_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, ALLOC_TRACKER_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, ALLOC_TRACKER_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, ALLOC_TRACKER_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, ALLOC_TRACKER_TAG, __VA_ARGS__)

#endif //ALLOCTRACK_LOGGER_H
