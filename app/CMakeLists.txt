cmake_minimum_required(VERSION 3.20.0)

# Get absolute (real path) to the module base folder
get_filename_component(
  LUA_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../ REALPATH )

get_filename_component(
  IRIDIUM_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/iridium/ REALPATH )

# Append module path to the extra modules list
list( APPEND ZEPHYR_EXTRA_MODULES ${LUA_MODULE_PATH} )
list( APPEND ZEPHYR_EXTRA_MODULES ${IRIDIUM_MODULE_PATH} )

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(lua_zepphyr_app)

target_sources(app PRIVATE
  ./src/main.c
)

target_link_libraries( app PRIVATE lua iridium )

# include( ${ZEPHYR_BASE}/samples/net/common/common.cmake )

# add_custom_command( 
#   OUTPUT lua/main.o
#   COMMAND "luac" -o lua/main.o -- lua/main.lua
#   DEPENDS "lua/main.lua"
#   WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
# )

# lua_compile_file( lua/main.o out )

generate_inc_file_for_target(
	app
	lua/main.lua
	${PROJECT_BINARY_DIR}/zephyr/include/generated/lua.main.inc
)

# target_include_directories(app PRIVATE
#   ../modules/hal/nxp/mcux/mcux-sdk/drivers/pwm/
#   )
