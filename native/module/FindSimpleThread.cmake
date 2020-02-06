# - Try to find SimpleThread
#
# The following variables are optionally searched for defaults
#  SIMPLETHREAD_ROOT_DIR:            Base directory where all GLOG components are found
#
# The following are set after configuration is done:
#  SIMPLETHREAD_FOUND
#  SIMPLETHREAD_INCLUDE_DIRS
#  SIMPLETHREAD_LIBRARIES
#  SIMPLETHREAD_LIBRARYRARY_DIRS

include(FindPackageHandleStandardArgs)

set(SIMPLETHREAD_ROOT_DIR "" CACHE PATH "Folder contains Gurum simplethread")

if(WIN32)
    find_path(SIMPLETHREAD_INCLUDE_DIR simpleio/simple_thread.h
        PATHS ${SIMPLETHREAD_ROOT_DIR}/src/simplethread)
else()
    find_path(SIMPLETHREAD_INCLUDE_DIR simplethread/simple_thread.h
        PATHS ${SIMPLETHREAD_ROOT_DIR})
endif()

if(MSVC)
    find_library(SIMPLETHREAD_LIBRARY_RELEASE libsimplethread_static
        PATHS ${SIMPLETHREAD_ROOT_DIR}
        PATH_SUFFIXES Release)

    find_library(SIMPLETHREAD_LIBRARY_DEBUG libsimplethread_static
        PATHS ${SIMPLETHREAD_ROOT_DIR}
        PATH_SUFFIXES Debug)

    set(SIMPLETHREAD_LIBRARY optimized ${SIMPLETHREAD_LIBRARY_RELEASE} debug ${SIMPLETHREAD_LIBRARY_DEBUG})
else()
    find_library(SIMPLETHREAD_LIBRARY simplethread
        PATHS ${SIMPLETHREAD_ROOT_DIR}
        PATH_SUFFIXES lib lib64)
endif()

find_package_handle_standard_args(SimpleThread DEFAULT_MSG SIMPLETHREAD_INCLUDE_DIR SIMPLETHREAD_LIBRARY)

if(SIMPLETHREAD_FOUND)
  set(SIMPLETHREAD_INCLUDE_DIRS ${SIMPLETHREAD_INCLUDE_DIR})
  set(SIMPLETHREAD_LIBRARIES ${SIMPLETHREAD_LIBRARY})
  message(STATUS "Found simplethread    (include: ${SIMPLETHREAD_INCLUDE_DIR}, library: ${SIMPLETHREAD_LIBRARY})")
  mark_as_advanced(SIMPLETHREAD_ROOT_DIR SIMPLETHREAD_LIBRARY_RELEASE SIMPLETHREAD_LIBRARY_DEBUG
                                 SIMPLETHREAD_LIBRARY SIMPLETHREAD_INCLUDE_DIR)
endif()

