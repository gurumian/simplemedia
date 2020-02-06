cmake_minimum_required(VERSION 3.0)


if (POLICY CMP0042)
  cmake_policy (SET CMP0042 NEW)
endif (POLICY CMP0042)

if (POLICY CMP0063)
  cmake_policy (SET CMP0063 NEW)
endif (POLICY CMP0063)

project (simplemedia)

FIND_PACKAGE (Threads)
set(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT})

INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckCXXSourceCompiles.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)

set (CMAKE_CXX_STANDARD 11)

add_definitions(-D__STDC_CONSTANT_MACROS)


set(CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/native/module
    ${CMAKE_MODULE_PATH})

add_subdirectory(native)

add_definitions(${FFMPEG_DEFINITIONS})
include_directories(${FFMPEG_INCLUDE_DIRS})
set(LIBS ${LIBS} ${FFMPEG_LIBRARIES})


set(simplemedia_base_dir ${CMAKE_CURRENT_LIST_DIR})
set(simplemedia_src_dir ${simplemedia_base_dir}/src)
set(simplemedia_inc_dir ${simplemedia_base_dir}/native/include)


find_package(SDL2)

find_package(Glog)

if (GLOG_FOUND)
	add_definitions(-DUSE_GLOG)
endif (GLOG_FOUND)

set(simplemedia_source_files
  ${simplemedia_src_dir}/addon.cc
  ${simplemedia_src_dir}/audio_decoder_wrap.cc
  ${simplemedia_src_dir}/video_decoder_wrap.cc
  ${simplemedia_src_dir}/frame_wrap.cc
  ${simplemedia_src_dir}/source_wrap.cc
)

if (SDL2_FOUND)
set(simplemedia_source_files
  ${simplemedia_source_files}
  ${simplemedia_src_dir}/addon.cc
  ${simplemedia_src_dir}/audio_renderer_wrap.cc
  ${simplemedia_src_dir}/video_renderer_wrap.cc
  ${simplemedia_src_dir}/window_wrap.cc
)
endif (SDL2_FOUND)

execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
#target_include_directories(simplemedia PRIVATE ${NODE_ADDON_API_DIR})
include_directories(${NODE_ADDON_API_DIR})

include_directories(${simplemedia_inc_dir})
include_directories(${simplemedia_base_dir}/native/include/simplemedia)
include_directories(${simplemedia_base_dir}/native/src)

include_directories(${CMAKE_JS_INC})
add_library(simplemedia SHARED ${simplemedia_source_files} ${CMAKE_JS_SRC})
set_target_properties(simplemedia PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(simplemedia ${CMAKE_JS_LIB} simplemedia_native)
