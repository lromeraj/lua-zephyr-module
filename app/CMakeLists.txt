cmake_minimum_required(VERSION 3.20.0)

# ┌─────────────────────────────────────────────┐
# │             Zephyr dependencies             │
# └─────────────────────────────────────────────┘

# Get absolute (real path) to the Lua module base folder
get_filename_component(
  LUA_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../ REALPATH )

# Get absolute (real path) to the Iiridium module base folder
get_filename_component(
  IRIDIUM_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/iridium/ REALPATH )

# Append module path to the extra modules list
list( APPEND ZEPHYR_EXTRA_MODULES ${LUA_MODULE_PATH} )
list( APPEND ZEPHYR_EXTRA_MODULES ${IRIDIUM_MODULE_PATH} )

if( "${DEBUG}" STREQUAL "yes" )
	list( APPEND OVERLAY_CONFIG "dbg.conf" )
endif()

if(DEFINED QEMU_TTY_PATH)
  set(
    ENV{QEMU_EXTRA_FLAGS}
    "-chardev serial,id=con1,path=${QEMU_TTY_PATH} -serial chardev:con1" )
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project( lua_zepphyr_app )

target_sources( app PRIVATE
  ./src/main.c
)

target_link_libraries( app PRIVATE lua iridium )

# This is a simple wrapper which compiles lua files
# and generates a header file with the binary content
# The output header name will be, for example:
# /my/path/main.lua -> main.lua.h
lua_generate_inc_file_for_target(
  app
  ${CMAKE_CURRENT_SOURCE_DIR}/lua/main.lua
)