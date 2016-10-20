cmake_minimum_required(VERSION 3.1.0)

include(GNUInstallDirs)

project(common
  VERSION 0.1
  LANGUAGES C CXX)

add_library(common STATIC
  types.c)


install(TARGETS common DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES "types.h" DESTINATION "{CMAKE_INSTALL_INCLUDEDIR}")

