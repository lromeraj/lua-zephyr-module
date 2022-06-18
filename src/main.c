#include <zephyr.h>
#include <device.h>
#include <sys/printk.h>
#include <usb/usb_device.h>
#include <storage/disk_access.h>
#include <fs/fs.h>
#include <ff.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"

lua_State * L;


static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
    .type = FS_FATFS,
    .fs_data = &fat_fs,
    .mnt_point = "/SD:",
};


void main(void)
{
    if (usb_enable(NULL)) {
        return;
    }

    if (disk_access_init("USDHC_1") != 0) {
        printk("[SD] Storage init ERROR!");

        int res = fs_mount(&mp);
        if (res == FR_OK) {
            printk("[SD] Mounted\n");
        } else {
            printk("[SD] Error mounting disk.\n");
        }
    }

    /* k_sleep(K_MSEC(5000)); */

    L = luaL_newstate();
    luaL_openlibs(L);
    fflush(stdout);

    printk("[INIT DONE]\n");
}
