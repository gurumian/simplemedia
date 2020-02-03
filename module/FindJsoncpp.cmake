# - Try to find Jsoncpp
#
# The following variables are optionally searched for defaults
# JSONCPP_ROOT_DIR:            Base directory where all JSONCPP components are found
#
# The following are set after configuration is done:
#  JSONCPP_FOUND
#  JSONCPP_INCLUDE_DIRS
#  JSONCPP_LIBRARIES
#  JSONCPP_LIBRARYRARY_DIRS

include(FindPackageHandleStandardArgs)

set(JSONCPP_ROOT_DIR "" CACHE PATH "Folder contains Jsoncpp")

find_path(JSONCPP_INCLUDE_DIR json/json.h
    PATHS ${JSONCPP_ROOT_DIR})

find_library(JSONCPP_LIBRARY jsoncpp
    PATHS ${JSONCPP_ROOT_DIR}
    PATH_SUFFIXES lib lib64)

find_package_handle_standard_args(Jsoncpp DEFAULT_MSG JSONCPP_INCLUDE_DIR JSONCPP_LIBRARY)

if(JSONCPP_FOUND)
  set(JSONCPP_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIR})
  set(JSONCPP_LIBRARIES ${JSONCPP_LIBRARY})
  message(STATUS "Found jsoncpp    (include: ${JSONCPP_INCLUDE_DIRS}, library: ${JSONCPP_LIBRARIES})")
  mark_as_advanced(JSONCPP_ROOT_DIR JSONCPP_LIBRARY_RELEASE JSONCPP_LIBRARY_DEBUG
                                 JSONCPP_LIBRARY JSONCPP_INCLUDE_DIR)
endif()
