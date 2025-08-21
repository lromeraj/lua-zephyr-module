#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/fs/fs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lauxlib.h"
#include "lualib.h"

#include "luac.h"

LOG_MODULE_REGISTER(app);

// For reference
// https://www.lua.org/pil/24.1.html
// https://docs.zephyrproject.org/latest/contribute/bin_blobs.html

static int l_z_sleep(lua_State *L);

static const struct luaL_Reg l_zephyr[] = {
  {"sleep", l_z_sleep },
  {NULL, NULL}  /* sentinel */
};

static int l_z_sleep(lua_State *L) {
  lua_Number sleep_ms = luaL_checknumber(L, 1);
  
  k_sleep(K_MSEC(sleep_ms));
  return 0;
}

int luaopen_zephyr(lua_State *L) {

  lua_newtable(L);
  luaL_setfuncs(L, l_zephyr, 0);

  lua_setglobal(L, "z");

  return 1;
}

int main(void) {

  static lua_State * L;

  L = luaL_newstate();

  luaL_openlibs(L);
  luaopen_zephyr(L);
  
  // lua_pushcfunction( L, l_sin );
  // lua_setglobal( L, "mysin" );
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

