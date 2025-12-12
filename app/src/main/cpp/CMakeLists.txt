cmake_minimum_required(VERSION 3.22.1)
project("cnative")

# OpenCV headers (CHÍNH XÁC)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link OpenCV .so
add_library(opencv_java4 SHARED IMPORTED)
set_target_properties(opencv_java4 PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libopencv_java4.so)

add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME}
        android
        log
        jnigraphics
        opencv_java4)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE "-Wl,-z,max-page-size=16384")
