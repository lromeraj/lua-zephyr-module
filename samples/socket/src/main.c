#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/fs/fs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <lauxlib.h>
#include <lualib.h>
#include <luasocket.h>

#include "luac.h"

LOG_MODULE_REGISTER(app);

// For reference
// https://www.lua.org/pil/24.1.html
// https://docs.zephyrproject.org/latest/contribute/bin_blobs.html

int main(void) {

  static lua_State * L;

  L = luaL_newstate();

  luaL_openlibs(L);

  luaopen_socket_core(L);
  lua_setglobal(L, "socket");
  
  int error = 
    luaL_loadbuffer(L, g_lua_main, sizeof(g_lua_main), "main") 
    || lua_pcall(L, 0, 0, 0);

  if (error) {
    // lua_tostring() returns and appends 
    // at the same time the value in the stack
    LOG_ERR("%s", lua_tostring(L, -1));
    lua_pop(L, 1); 
  }

  lua_close(L);
  
  return 0;
}

