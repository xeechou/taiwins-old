cmake_minimum_required(VERSION 3.1.0)

project(taiwins C)
set(CMAKE_C_FLAGS "-g")

add_definitions(
  -D_GNU_SOURCE
  )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/CMake")

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


include_directories(include)

add_subdirectory(src)
