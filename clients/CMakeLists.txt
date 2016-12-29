include_directories(
  ${common_INCLUDE_DIRS}
  ${PROTOCOLS_INCLUDE_DIR}
  ${WLC_INCLUDE_DIRS}
  ${LIBINPUT_INCLUDE_DIRS}
  ${WAYLAND_CLIENT_INCLUDE_DIR}
)

include(GNUInstallDirs)

add_executable(desktop_shell
  desktop_shell.c
  buffer.c
  )

add_library(clients_lib SHARED
  wayland.c)

target_link_libraries(desktop_shell
  clients_lib
  proto_libraries
  ${WAYLAND_CLIENT_LIBRARIES}
  ${common_LIBRARY}
  )

install(TARGETS clients_lib DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(TARGETS desktop_shell DESTINATION "${CMAKE_INSTALL_BINDIR}")
