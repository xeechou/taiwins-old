include_directories(
	${PROTOCOLS_INCLUDE_DIR}
	${WLC_INCLUDE_DIRS}
	${PCRE_INCLUDE_DIRS}
	${JSONC_INCLUDE_DIRS}
	${XKBCOMMON_INCLUDE_DIRS}
	${LIBINPUT_INCLUDE_DIRS}
	${CAIRO_INCLUDE_DIRS}
	${PANGO_INCLUDE_DIRS}
	${common_INCLUDE_DIR}
)


add_executable(taiwins
  main.c
  utils.c
  layout.cpp
  wm.cpp
  handlers.cpp
  protocols.c
  ###protocols
  taiwins_shell.cpp
  dummy_shell.c
  )

target_link_libraries(taiwins
  proto_libraries
  ${common_LIBRARY}
  ${WLC_LIBRARIES}
  ${XKBCOMMON_LIBRARIES}
  ${WAYLAND_SERVER_LIBRARIES}
  ${LIBINPUT_LIBRARIES}
  )
