include_directories(
	${WLC_INCLUDE_DIRS}
	${LIBINPUT_INCLUDE_DIRS}
	${WAYLAND_CLIENT_INCLUDE_DIR}
)


add_executable(desktop_clients
  desktop_shell_client.c
  )

target_link_libraries(desktop_clients
  ${WAYLAND_CLIENT_LIBRARIES}
  )
