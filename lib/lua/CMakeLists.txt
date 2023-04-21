

if(CONFIG_LUA)
  zephyr_library_named( lua )
  zephyr_library_sources( stubs.c )
  zephyr_library_sources( src/onelua.c )
  zephyr_include_directories( src/ )
  zephyr_library_link_libraries( zephyr )
endif()

set(
  LUAC_PATH 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/luac 
  CACHE INTERNAL "" )

# LIST( APPEND CMAKE_PROGRAM_PATH  ${LUAC_PATH} )

add_custom_command( OUTPUT ${LUAC_PATH}
                    COMMAND make all
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(  luac
                    DEPENDS ${LUAC_PATH}
)

function( lua_generate_inc_file_for_target target srcPath )

  get_filename_component( srcName ${srcPath} NAME)

  set(destPath ${CMAKE_CURRENT_BINARY_DIR}/lua/${srcName}.o)

  add_custom_command(
    OUTPUT ${destPath}
    COMMAND ${LUAC_PATH} -o ${destPath} -- ${srcPath}
    DEPENDS luac ${srcPath}
  )

  generate_inc_file_for_target(
	  ${target}
	  ${destPath}
	  ${PROJECT_BINARY_DIR}/zephyr/include/generated/${srcName}.h
  )

endfunction()

# add_custom_target( luac DEPENDS ./src/luac )
