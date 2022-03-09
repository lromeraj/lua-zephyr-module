#include <zephyr.h>
#include <device.h>
#include <sys/printk.h>
#include <usb/usb_device.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"

lua_State * L;

void main(void)
{


	if (usb_enable(NULL)) {
		return;
	}

    /* k_sleep(K_MSEC(5000)); */

    L = luaL_newstate();
	luaL_openlibs(L);
    fflush(stdout);

    printk("[INIT DONE]\n");
}
