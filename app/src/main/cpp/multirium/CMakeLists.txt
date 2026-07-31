cmake_minimum_required(VERSION 3.12)
project(multirium-client)

set(CMAKE_CXX_STANDARD 20)

if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    add_definitions(-DVER_x32=true)
elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
    add_definitions(-DVER_x32=false)
endif()

# Add source files
file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/**/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/**/**/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/**/**/**/**/**/*.c*
        ${CMAKE_CURRENT_LIST_DIR}/**/**/**/**/**/**/**/**/**/*.c*
        )
list(FILTER SOURCES EXCLUDE REGEX ".*TimeCycleDebugModule.h\\.cpp$")
# Add include directories
include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/util/
        ${CMAKE_CURRENT_LIST_DIR}/vendor/armhook
        ${CMAKE_CURRENT_LIST_DIR}/vendor/GlossHook/include/
        ${CMAKE_CURRENT_LIST_DIR}/vendor/imgui
        ${CMAKE_CURRENT_LIST_DIR}/vendor/
        ${CMAKE_CURRENT_LIST_DIR}/game/RW/
        ${CMAKE_CURRENT_LIST_DIR}/game/core/
        ${CMAKE_CURRENT_LIST_DIR}/cross_system/
)

set(glosshook-path "${CMAKE_CURRENT_SOURCE_DIR}/vendor/GlossHook/lib/${ANDROID_ABI}/libGlossHook.a")
add_library(glosshook STATIC IMPORTED)
set_target_properties(glosshook PROPERTIES IMPORTED_LOCATION ${glosshook-path})

set(bass-path "${CMAKE_CURRENT_SOURCE_DIR}/vendor/bass/libs/${ANDROID_ABI}/libbass.so")
add_library(bass SHARED IMPORTED)
set_target_properties(bass PROPERTIES IMPORTED_LOCATION ${bass-path})

# Opus
#add_library(libopus STATIC IMPORTED)
#set_target_properties(libopus PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/vendor/opus/libopus.a)

add_library(multirium-client SHARED
        ${SOURCES}
        ${CMAKE_CURRENT_LIST_DIR}/cross_system/CChooseSpawn.cpp
        ${CMAKE_CURRENT_LIST_DIR}/cross_system/CRegistration.cpp
        ${CMAKE_CURRENT_LIST_DIR}/cross_system/CCustomNotify.cpp
        ${CMAKE_CURRENT_LIST_DIR}/cross_system/CGtaDialog.cpp
        ${CMAKE_CURRENT_LIST_DIR}/cross_system/CMinerIndicator.cpp
        ${CMAKE_CURRENT_LIST_DIR}/game/custom/CVehicleVinyl.cpp
)


target_link_libraries(multirium-client log EGL GLESv3 opus glosshook bass)
#add_library(imgui STATIC imgui.cpp imgui_draw.cpp imgui_widgets.cpp imgui_tables.cpp)
# Set compilation flags
#
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -g -fvisibility=default -pthread -Wall -O3 -fexceptions")
