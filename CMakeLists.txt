
cmake_minimum_required( VERSION 2.8 )
project( xrdcl-authz-plugin )

find_package( xrootd REQUIRED )

if( CMAKE_COMPILER_IS_GNUCXX )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -DLCMAPS_GSI_MODE=1" )
endif()

if( CMAKE_COMPILER_IS_GNUCC )
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -DLCMAPS_GSI_MODE=1" )
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

include_directories( "${XROOTD_INCLUDE_DIRS}" )
add_library(XrdClAuthzPlugin-${XROOTD_PLUGIN_VERSION} MODULE src/XrdClAuthzPlugin.cc)
target_link_libraries(XrdClAuthzPlugin-${XROOTD_PLUGIN_VERSION} ${XROOTD_LIBRARIES})
set_target_properties(XrdClAuthzPlugin-${XROOTD_PLUGIN_VERSION} PROPERTIES LINK_FLAGS "-Wl,--version-script=${PROJECT_SOURCE_DIR}/configs/export-symbols")

add_library(XrdSecunix-${XROOTD_PLUGIN_VERSION} MODULE src/XrdSecProtocolunix.cc)
set_target_properties(XrdSecunix-${XROOTD_PLUGIN_VERSION} PROPERTIES LINK_FLAGS "-Wl,--version-script=${PROJECT_SOURCE_DIR}/configs/export-symbols-anon")

if (NOT DEFINED LIB_INSTALL_DIR)
  SET(LIB_INSTALL_DIR "lib")
endif()

if (NOT DEFINED SYSCONF_INSTALL_DIR)
  SET(SYSCONF_INSTALL_DIR "etc")
endif()

configure_file(configs/client.plugins.d/authz.conf.in configs/client.plugins.d/authz.conf)

install(
  FILES
  ${CMAKE_BINARY_DIR}/configs/client.plugins.d/authz.conf
  DESTINATION ${SYSCONF_INSTALL_DIR}/xrootd/client.plugins.d/
  PERMISSIONS
  OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

install(
  TARGETS XrdClAuthzPlugin-${XROOTD_PLUGIN_VERSION}
  LIBRARY DESTINATION ${LIB_INSTALL_DIR} )

install(
  TARGETS XrdSecunix-${XROOTD_PLUGIN_VERSION}
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}/xrdcl-authz-plugin )

set_target_properties(
  XrdClAuthzPlugin-${XROOTD_PLUGIN_VERSION}
  PROPERTIES
  VERSION   ""
  SOVERSION ""
  LINK_INTERFACE_LIBRARIES "" )

set_target_properties(
  XrdSecunix-${XROOTD_PLUGIN_VERSION}
  PROPERTIES
  VERSION   ""
  SOVERSION ""
  LINK_INTERFACE_LIBRARIES "" )
