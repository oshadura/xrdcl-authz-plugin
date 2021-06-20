
cmake_minimum_required( VERSION 2.8...3.13 )
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

message("Plugin version:" ${XROOTD_PLUGIN_VERSION})
message("Include dirs:" ${XROOTD_INCLUDE_DIRS})

include_directories( "${XROOTD_INCLUDE_DIRS}" )
add_library(XrdClAuthzPlugin-${XROOTD_PLUGIN_VERSION} MODULE src/XrdClAuthzPlugin.cc)
target_link_libraries(XrdClAuthzPlugin-${XROOTD_PLUGIN_VERSION} ${XROOTD_LIBRARIES})
set_target_properties(XrdClAuthzPlugin-${XROOTD_PLUGIN_VERSION} PROPERTIES LINK_FLAGS "-Wl,--version-script=${PROJECT_SOURCE_DIR}/configs/export-symbols")

