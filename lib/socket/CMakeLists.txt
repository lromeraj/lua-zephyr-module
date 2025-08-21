if(CONFIG_LUA_SOCKET)

	zephyr_library_named(luasocket)
	
	zephyr_library_sources(
		src/compat.c
		src/auxiliar.c
		src/buffer.c
		src/except.c
		src/inet.c
		src/io.c
		src/mime.c
		src/options.c
		src/select.c
		#src/serial.c
		src/tcp.c
		src/timeout.c
		src/udp.c
		src/unix.c
		src/usocket.c
		src/luasocket.c
	)
  	
	zephyr_include_directories(src/)

	zephyr_library_compile_options(
		-DLUASOCKET_INET_ATON
	)

endif()