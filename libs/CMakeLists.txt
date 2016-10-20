############################DEAL WITH EXXTERNAL PROJECTS############################
###USE CMAKE_SHARED_LIBRARY_PREFIX, CMAKE_SHARED_LIBRARY_SUFFIX,
###CMAKE_STATIC_LIBRARY_PREFIX, CMAKE_STATIC_LIBARY_SUFFIX

include(GNUInstallDirs)
include(ExternalProject)
#we all want to generate project_include_dir and project_libraries variable for
#a given project, but it is like you need to write a findpackage like script 




####Deal with common
ExternalProject_Add(common
  PREFIX common
  UPDATE_COMMAND ""
  PATCH_COMMAND  ""
  INSTALL_DIR "${CMAKE_SOURCE_DIR}/libs/common-bin"
  CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/libs/common"
  #build flags
  )
ExternalProject_Get_Property(common
  install_dir)
#get the include dirs 
set(common_INCLUDE_DIRS "${install_dir}/${CMAKE_INSTALL_INCLUDEDIR}")
#setup the libraries, or maybe you shouldn't use find library???, change it later
add_library(_common STATIC IMPORTED)
set_property(TARGET _common PROPERTY IMPORTED_LOCATION
  "${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}common${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(common_LIBRARY "${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}common${CMAKE_STATIC_LIBRARY_SUFFIX}")

add_dependencies(_common common)

#set(common_LIBRARY "${_common}")
message("Found common project: include_dir ${common_INCLUDE_DIRS}")
message("libraies: ${common_LIBRARY}")
