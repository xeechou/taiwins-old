cmake_minimum_required(VERSION 3.1.0)

project(taiwins
  VERSION 0.1
  LANGUAGES C CXX)

set(CMAKE_BINARY_DIR build)
set(CMAKE_C_FLAGS "-g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(
  -D_GNU_SOURCE
  )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall")
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/CMake")
include(GNUInstallDirs)

find_package(JsonC REQUIRED)
find_package(PCRE REQUIRED)
find_package(WLC REQUIRED)
find_package(Wayland REQUIRED)
find_package(XKBCommon REQUIRED)
find_package(Cairo)
find_package(Pango)
find_package(GdkPixbuf)
find_package(PAM)
find_package(LibInput REQUIRED)


#runtime output dir, but it ruined everything
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin) 

add_subdirectory(common)
include_directories(${common_INCLUDE_DIR})

include_directories(include)
add_subdirectory(protocols)
add_subdirectory(src)
add_subdirectory(clients)
