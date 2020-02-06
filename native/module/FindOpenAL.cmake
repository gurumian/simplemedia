# - Try to find OpenAL
#
# The following variables are optionally searched for defaults
#  OPENAL_ROOT_DIR:            Base directory where all GLOG components are found
#
# The following are set after configuration is done:
#  OPENAL_FOUND
#  OPENAL_INCLUDE_DIRS
#  OPENAL_LIBRARIES
#  OPENAL_LIBRARYRARY_DIRS

include(FindPackageHandleStandardArgs)

set(OPENAL_ROOT_DIR "" CACHE PATH "Folder contains OpenAL")

find_path(OPENAL_INCLUDE_DIR AL/al.h
    PATHS ${OPENAL_ROOT_DIR})

find_library(OPENAL_LIBRARY openal
    PATHS ${OPENAL_ROOT_DIR}
    PATH_SUFFIXES lib lib64)

find_package_handle_standard_args(OpenAL DEFAULT_MSG OPENAL_INCLUDE_DIR OPENAL_LIBRARY)

if(OPENAL_FOUND)
  set(OPENAL_INCLUDE_DIRS ${OPENAL_INCLUDE_DIR})
  set(OPENAL_LIBRARIES ${OPENAL_LIBRARY})
  message(STATUS "Found openal    (include: ${OPENAL_INCLUDE_DIR}, library: ${OPENAL_LIBRARY})")
  mark_as_advanced(OPENAL_ROOT_DIR OPENAL_LIBRARY_RELEASE OPENAL_LIBRARY_DEBUG
                                 OPENAL_LIBRARY OPENAL_INCLUDE_DIR)
endif()


