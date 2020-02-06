cmake_minimum_required(VERSION 3.0)


if (POLICY CMP0042)
  cmake_policy (SET CMP0042 NEW)
endif (POLICY CMP0042)

if (POLICY CMP0063)
  cmake_policy (SET CMP0063 NEW)
endif (POLICY CMP0063)

project (simplemedia)


# CMakeLists for libsimplemedia
# Run with -DTEST=ON to build unit tests
#option(TEST "Built unit tests" OFF)

#set(USE_OPENAL 1)
option(USE_OPENAL "Use OpenAL" OFF)

#find_package(PkgConfig)

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
    ${CMAKE_SOURCE_DIR}/module
    ${CMAKE_MODULE_PATH})

set(simplemedia_base_dir ${CMAKE_CURRENT_LIST_DIR})
set(simplemedia_src_dir ${simplemedia_base_dir}/src/)
set(simplemedia_inc_dir ${simplemedia_base_dir}/include)
set(simplemedia_script_dir ${simplemedia_base_dir}/script)
set(simplemedia_lib_name "simplemedia_native")

set(simplemedia_source_files
  ${simplemedia_src_dir}/media_player.cc
  ${simplemedia_src_dir}/sdl/sdl_video_renderer.cc
  ${simplemedia_src_dir}/video_decoder.cc
  ${simplemedia_src_dir}/timer.cc
  ${simplemedia_src_dir}/source.cc
  ${simplemedia_src_dir}/pid_channel.cc
  ${simplemedia_src_dir}/packet_pool.cc
  ${simplemedia_src_dir}/packet_pool.h
  ${simplemedia_src_dir}/decoder.cc
  ${simplemedia_src_dir}/sdl/sdl_audio_renderer.cc
  ${simplemedia_src_dir}/audio_decoder.cc
  ${simplemedia_src_dir}/encoder.cc
  ${simplemedia_src_dir}/audio_encoder.cc
  ${simplemedia_src_dir}/frame_decoder.cc
  ${simplemedia_src_dir}/subtitle_decoder.cc
  ${simplemedia_src_dir}/sdl/sdl_subtitle_renderer.cc
  ${simplemedia_src_dir}/simple_thread.cc
)

set(simplemedia_header_files
  ${simplemedia_inc_dir}/simplemedia/media_player.h
  ${simplemedia_inc_dir}/simplemedia/video_renderer.h
  ${simplemedia_inc_dir}/simplemedia/video_decoder.h
  ${simplemedia_inc_dir}/simplemedia/timer.h
  ${simplemedia_inc_dir}/simplemedia/source.h
  ${simplemedia_inc_dir}/simplemedia/pid_channel.h
  ${simplemedia_inc_dir}/simplemedia/decoder.h
  ${simplemedia_inc_dir}/simplemedia/audio_renderer.h
  ${simplemedia_inc_dir}/simplemedia/audio_decoder.h
  ${simplemedia_inc_dir}/simplemedia/encoder.h
  ${simplemedia_inc_dir}/simplemedia/audio_encoder.h
  ${simplemedia_inc_dir}/simplemedia/renderer.h
  ${simplemedia_inc_dir}/simplemedia/config.h
  ${simplemedia_inc_dir}/simplemedia/sdl/sdl_video_renderer.h
  ${simplemedia_inc_dir}/simplemedia/sdl/sdl_audio_renderer.h
  ${simplemedia_inc_dir}/simplemedia/frame_decoder.h
  ${simplemedia_inc_dir}/simplemedia/sdl/sdl_subtitle_renderer.h
  ${simplemedia_inc_dir}/simplemedia/subtitle_decoder.h
  ${simplemedia_inc_dir}/simplemedia/subtitle_renderer.h
)

set(simplemedia_script_files
  ${simplemedia_script_dir}/start
)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
set(LIBS ${LIBS} ${SDL2_LIBRARY})

find_package(FFmpeg REQUIRED)
add_definitions(${FFMPEG_DEFINITIONS})
include_directories(${FFMPEG_INCLUDE_DIRS})
set(LIBS ${LIBS} ${FFMPEG_LIBRARIES})

if (SWSCALE_FOUND)
add_definitions(${SWSCALE_DEFINITIONS})
include_directories(${SWSCALE_INCLUDE_DIRS})
set(LIBS ${LIBS} ${SWSCALE_LIBRARIES})
endif (SWSCALE_FOUND)


if (SWRESAMPLE_FOUND)
add_definitions(${SWRESAMPLE_DEFINITIONS})
include_directories(${SWRESAMPLE_INCLUDE_DIRS})
set(LIBS ${LIBS} ${SWRESAMPLE_LIBRARIES})
#set(USE_SWRESAMPLE 1)

#option(USE_SWRESAMPLE "Use swresample" ON)
endif (SWRESAMPLE_FOUND)

if (POSTPROC_FOUND)
add_definitions(${POSTPROC_DEFINITIONS})
include_directories(${POSTPROC_INCLUDE_DIRS})
set(LIBS ${LIBS} ${POSTPROC_LIBRARIES})
endif (POSTPROC_FOUND)


find_package(Glog)
include_directories(${GLOG_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GLOG_LIBRARIES})

if (GLOG_FOUND)
	add_definitions(-DUSE_GLOG)
endif (GLOG_FOUND)

include_directories(${simplemedia_inc_dir})
include_directories(${simplemedia_inc_dir}/simplemedia)
include_directories(${simplemedia_src_dir})

configure_file ("${simplemedia_inc_dir}/simplemedia/config.h.in"
                "${simplemedia_inc_dir}/simplemedia/config.h" )
               
               
#add_library(${simplemedia_lib_name} STATIC SHARED ${simplemedia_source_files})
add_library(${simplemedia_lib_name} SHARED ${simplemedia_source_files})
target_link_libraries(${simplemedia_lib_name} ${LIBS})

add_executable(mediaplayer ${simplemedia_base_dir}/example/example_player.cc)
target_link_libraries(mediaplayer ${simplemedia_lib_name} ${LIBS} ${CMAKE_DL_LIBS})

add_executable(audiodecoder ${simplemedia_base_dir}/example/example_audio_decoder.cc)
target_link_libraries(audiodecoder ${simplemedia_lib_name} ${LIBS} ${CMAKE_DL_LIBS})

add_executable(audioplayer ${simplemedia_base_dir}/example/example_audio_player.cc)
target_link_libraries(audioplayer ${simplemedia_lib_name} ${LIBS} ${CMAKE_DL_LIBS})

install(TARGETS ${simplemedia_lib_name} DESTINATION lib)
install(DIRECTORY ${simplemedia_inc_dir}/simplemedia DESTINATION include)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/{mediaplayer,audioplayer,audiodecoder} DESTINATION bin/simplemedia)
install(PROGRAMS ${simplemedia_script_files} DESTINATION bin/simplemedia)

