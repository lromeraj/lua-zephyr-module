# Get absolute (real path) to the module base folder
get_filename_component(
  IRIDIUM_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../ REALPATH )

# Append module path to the extra modules list
list( APPEND ZEPHYR_EXTRA_MODULES ${IRIDIUM_MODULE_PATH} )

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(lua_zepphyr_app)

target_sources(app PRIVATE
  ./src/main.c
)

target_link_libraries( app PRIVATE lua )

include( ${ZEPHYR_BASE}/samples/net/common/common.cmake )

generate_inc_file_for_target(
	app
	lua/main.lua
	${PROJECT_BINARY_DIR}/zephyr/include/generated/lua.main.inc
)

# target_include_directories(app PRIVATE
#   ../modules/hal/nxp/mcux/mcux-sdk/drivers/pwm/
#   )
