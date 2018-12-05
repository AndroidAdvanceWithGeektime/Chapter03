# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -DART_DEFAULT_GC_TYPE_IS_CMS -DDISABLE_CPUCAP -DDISABLE_XPLAT -fexceptions  -DHAVE_POSIX_CLOCKS -Wno-unused -Wno-gnu-alignof-expression -Wno-missing-field-initializers -std=gnu++14 -fvisibility=hidden -fexceptions -Wextra -Wno-unused-parameter -Wno-format-security -Wno-unused-local-typedef")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpack-struct=4 -g -Os -Wextra -DMUSEUM_NOOP_CHECK_MACROS -DMUSEUM_NOOP_THREAD_MUTATORS")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

include_directories(.)
add_library( # Sets the name of the library.
        fbjni
        STATIC
        lyra/lyra_exceptions.cpp
        lyra/lyra_breakpad.cpp
        lyra/cxa_throw.cpp
        lyra/lyra.cpp
        fbjni/ByteBuffer.cpp
        fbjni/fbjni.cpp
#        fbjni/OnLoad.cpp
        fbjni/ReadableByteChannel.cpp
        fbjni/detail/Environment.cpp
        fbjni/detail/Exceptions.cpp
        fbjni/detail/Hybrid.cpp
        fbjni/detail/References.cpp
        fbjni/detail/utf8.cpp
        )


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
        fbjni

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})