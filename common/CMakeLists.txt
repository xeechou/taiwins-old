cmake_minimum_required(VERSION 3.1.0)

include(GNUInstallDirs)


project(commoncutils
  VERSION 0.1
  LANGUAGES C CXX)

add_library(common STATIC
  types.c)
set(common_LIBRARY common
  CACHE STRING "")
set(common_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  CACHE STRING "")

#install(TARGETS common DESTINATION "${CMAKE_INSTALL_LIBDIR}")
#install(FILES "types.h" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

