if(CONFIG_LUA)

  zephyr_library_named( lua )
  zephyr_library_sources( stubs.c )
  zephyr_library_sources( src/onelua.c )
  zephyr_include_directories( src/ )
  zephyr_library_link_libraries( zephyr )

endif()