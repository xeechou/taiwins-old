include(Wayland)

WAYLAND_ADD_PROTOCOL_CLIENT(proto_twshell_client
  taiwins_shell.xml
  taiwins_shell
  )

WAYLAND_ADD_PROTOCOL_SERVER(proto_twshell_server
  taiwins_shell.xml
  taiwins_shell
  )

#You always need the dummy protocol
WAYLAND_ADD_PROTOCOL_SERVER(proto_dummy_server
  dummy.xml
  dummy)

WAYLAND_ADD_PROTOCOL_CLIENT(proto_dummy_client
  dummy.xml
  dummy)

add_library(proto_libraries STATIC
  ${proto_twshell_client}
  ${proto_twshell_server}
  ${proto_dummy_client}
  ${proto_dummy_server}
  )
set(PROTOCOLS_INCLUDE_DIR ${CMAKE_BINARY_DIR}/protocols PARENT_SCOPE)
