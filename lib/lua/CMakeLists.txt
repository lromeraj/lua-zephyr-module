if(CONFIG_LUA)

  zephyr_library_named( lua )
  zephyr_library_sources( stubs.c )
  zephyr_library_sources( onelua.c )

  zephyr_include_directories( ./ )
  zephyr_library_link_libraries( zephyr )

endif()
