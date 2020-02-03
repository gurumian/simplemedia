# - Try to find SimpleIO
#
# The following variables are optionally searched for defaults
#  SIMPLEIO_ROOT_DIR:            Base directory where all GLOG components are found
#
# The following are set after configuration is done:
#  SIMPLEIO_FOUND
#  SIMPLEIO_INCLUDE_DIRS
#  SIMPLEIO_LIBRARIES
#  SIMPLEIO_LIBRARYRARY_DIRS

include(FindPackageHandleStandardArgs)

set(SIMPLEIO_ROOT_DIR "" CACHE PATH "Folder contains Gurum simpleio")

if(WIN32)
    find_path(SIMPLEIO_INCLUDE_DIR simpleio/simple_client.h
        PATHS ${SIMPLEIO_ROOT_DIR}/src/simpleio)
else()
    find_path(SIMPLEIO_INCLUDE_DIR simpleio/simple_client.h
        PATHS ${SIMPLEIO_ROOT_DIR})
endif()

if(MSVC)
    find_library(SIMPLEIO_LIBRARY_RELEASE libsimpleio_static
        PATHS ${SIMPLEIO_ROOT_DIR}
        PATH_SUFFIXES Release)

    find_library(SIMPLEIO_LIBRARY_DEBUG libsimpleio_static
        PATHS ${SIMPLEIO_ROOT_DIR}
        PATH_SUFFIXES Debug)

    set(SIMPLEIO_LIBRARY optimized ${SIMPLEIO_LIBRARY_RELEASE} debug ${SIMPLEIO_LIBRARY_DEBUG})
else()
    find_library(SIMPLEIO_LIBRARY simpleio
        PATHS ${SIMPLEIO_ROOT_DIR}
        PATH_SUFFIXES lib lib64)
endif()

find_package_handle_standard_args(Simpleio DEFAULT_MSG SIMPLEIO_INCLUDE_DIR SIMPLEIO_LIBRARY)

if(SIMPLEIO_FOUND)
  set(SIMPLEIO_INCLUDE_DIRS ${SIMPLEIO_INCLUDE_DIR})
  set(SIMPLEIO_LIBRARIES ${SIMPLEIO_LIBRARY})
  message(STATUS "Found simpleio    (include: ${SIMPLEIO_INCLUDE_DIR}, library: ${SIMPLEIO_LIBRARY})")
  mark_as_advanced(SIMPLEIO_ROOT_DIR SIMPLEIO_LIBRARY_RELEASE SIMPLEIO_LIBRARY_DEBUG
                                 SIMPLEIO_LIBRARY SIMPLEIO_INCLUDE_DIR)
endif()

