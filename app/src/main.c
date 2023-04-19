#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/fs/fs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "luac.h"

// For reference
// https://www.lua.org/pil/24.1.html
// https://docs.zephyrproject.org/latest/contribute/bin_blobs.html

lua_State * L;

/*
static FATFS fat_fs;
static struct fs_mount_t mp = {
    .type = FS_FATFS,
    .fs_data = &fat_fs,
    .mnt_point = "/SD:",
};
*/

// typedef int (*lua_CFunction) (lua_State *L);

static int l_sin (lua_State *L) {
    double d = luaL_checknumber(L, 1);
    lua_pushnumber(L, sin(d));
    return 1;  /* number of results */
}

void main(void) {

    /*
    if (disk_access_init("USDHC_1") != 0) {

        printk("[SD] Storage init ERROR!\n");

        int res = fs_mount(&mp);

        if (res == FR_OK) {
            printk("[SD] Mounted\n");
        } else {
            printk("[SD] Error mounting disk.\n");
        }
    }
    */

    L = luaL_newstate();
    luaL_openlibs( L );
    
    lua_pushcfunction( L, l_sin );
    lua_setglobal( L, "mysin" );

    luaL_dostring( L, lua_main );

    fflush(stdout);

    printk("[INIT DONE]\n");
}

