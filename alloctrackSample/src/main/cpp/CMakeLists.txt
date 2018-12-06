# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -Wno-unused -Wno-gnu-alignof-expression -Wno-missing-field-initializers -std=c++11 -fvisibility=hidden -fexceptions -Wextra -Wno-unused-parameter -Wno-format-security -Wno-unused-local-typedef")
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_subdirectory(fb)
add_subdirectory(Substrate)
#add_subdirectory(HookZz)

include_directories(fb)
add_library( # Sets the name of the library.
        alloc-lib
        SHARED
        dlopen.c
        allocTracker.cpp
        lock.cpp
        native-lib.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        alloc-lib
        ${log-lib} substrate fbjni)