project(roz_tools)
cmake_minimum_required(VERSION 2.8)

set(roz_tools_VERSION_MAJOR 0)
set(roz_tools_VERSION_MINOR 1)

set(CMAKE_CXX_FLAGS -Wall)
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" MATCHES "^2\\.6\\.4$")
    mark_as_advanced(FORCE CMAKE_BACKWARDS_COMPATIBILITY)
else("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" MATCHES "^2\\.6\\.4$")
    mark_as_advanced(CLEAR CMAKE_BACKWARDS_COMPATIBILITY)
endif("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" MATCHES "^2\\.6\\.4$")

aux_source_directory(. SRC_LIST)
#add_executable(${PROJECT_NAME} ${SRC_LIST})


find_package(ITK REQUIRED)
if(ITK_FOUND)
  message("Found ITK")
  include(${ITK_USE_FILE})
endif(ITK_FOUND)
set(ROZ_ITK_LIB ${ITK_LIBRARIES})


subdirs(IO)
subdirs(process)
subdirs(analysis)
subdirs(misc)

include_directories(
  ${CMAKE_SOURCE_DIR}/lib
  )

