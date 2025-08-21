if(CONFIG_LUA)

  zephyr_library_named(lua)

  zephyr_library_sources(src/onelua.c conf/luauser.c conf/stubs.c)

  zephyr_include_directories(src/)

  zephyr_library_compile_options(
    -DMAKE_LIB -DLUA_USER_H="${CMAKE_CURRENT_SOURCE_DIR}/conf/luauser.h")

	add_subdirectory(socket/)

endif()

set(LUA_SRC_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    CACHE INTERNAL "")

# set( LUA_PATH ${CMAKE_BINARY_DIR}/lua CACHE INTERNAL "")

set(LUAC_PATH
    ${CMAKE_BINARY_DIR}/luac
    CACHE INTERNAL "")

add_custom_command(
  OUTPUT ${LUAC_PATH}
  COMMAND gcc -O2 -Wall -Wextra -include ${AUTOCONF_H} -DLUA_COMPAT_5_3
          -D__ZEPHYR__=1 -DMAKE_LUAC onelua.c -lm -o ${LUAC_PATH}
  WORKING_DIRECTORY ${LUA_SRC_PATH}
  DEPENDS ${AUTOCONF_H})

# add_custom_command( OUTPUT ${LUA_PATH} COMMAND gcc -O2 -Wall -Wextra -include
# ${AUTOCONF_H} -DLUA_COMPAT_5_3 -D__ZEPHYR__=1 -DMAKE_LUA onelua.c -lm -o
# ${LUA_PATH} WORKING_DIRECTORY ${LUA_SRC_PATH} DEPENDS ${AUTOCONF_H} )

# add_custom_target(lua COMMAND ${LUA_PATH} DEPENDS ${LUA_PATH} USES_TERMINAL )

add_custom_target(luac DEPENDS ${LUAC_PATH})

function(generate_lua_byte_code_for_target target)

  set(srcFiles "${ARGN}")

  if(srcFiles)
    foreach(srcFile ${srcFiles})

      get_lua_byte_code_file_path(${srcFile} outFile)

      generate_lua_byte_code_file_for_target(${target} ${srcFile} ${outFile})

    endforeach()
  else()
    message(WARNING "No Lua source files provided")
  endif()

endfunction()

function(generate_lua_byte_code_file srcFile outFile)

  if(CONFIG_LUA_STRIP_DEBUG_INFORMATION)
    list(APPEND LUAC_FLAGS -s)
  endif()

  add_custom_command(
    OUTPUT ${outFile}
    COMMAND ${LUAC_PATH} ${LUAC_FLAGS} -o ${outFile} -- ${srcFile}
    DEPENDS luac ${srcFile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

endfunction()

function(generate_lua_byte_code_file_for_target target srcFile outFile)

  generate_lua_byte_code_file(${srcFile} ${outFile})

  # Generate a unique target name for the output file
  generate_unique_target_name_from_filename(${outFile} outFileTargetName)

  # Create custom target for the output file
  add_custom_target(${outFileTargetName} DEPENDS ${outFile})

  # Add output file target as a dependency for main target
  add_dependencies(${target} ${outFileTargetName})

endfunction()

function(generate_lua_inc_file_for_target target srcFile)

  get_lua_byte_code_file_path(${srcFile} outFile)

  generate_lua_byte_code_file(${srcFile} ${outFile})

  # Get source file directory
  get_filename_component(srcFileDir ${srcFile} DIRECTORY)

  # Get source file name without extension
  get_filename_component(srcFileNameWE ${srcFile} NAME_WE)

  generate_inc_file_for_target(
    ${target}
    ${outFile}
    ${PROJECT_BINARY_DIR}/zephyr/include/generated/${srcFileDir}/${srcFileNameWE}.lbc.h
  )

endfunction()

function(get_lua_byte_code_file_path srcFile outFile)

  # Get source file directory
  get_filename_component(srcFileDir ${srcFile} DIRECTORY)

  # Get source file name without extension
  get_filename_component(srcFileNameWE ${srcFile} NAME_WE)

  set(${outFile}
      ${PROJECT_BINARY_DIR}/${srcFileDir}/${srcFileNameWE}.lbc
      PARENT_SCOPE)

endfunction()
