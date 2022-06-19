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

#include <fsl_pwm.h>
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

    // Manually setup the PWM
    pwm_config_t pwmConfig;
    PWM_GetDefaultConfig(&pwmConfig);

    pwmConfig.reloadLogic = kPWM_ReloadPwmFullCycle;
    pwmConfig.clockSource = kPWM_BusClock;
    pwmConfig.enableDebugMode = true;

    if (PWM_Init((void*)0x403E0000, 0, &pwmConfig) == kStatus_Fail) {
        printk("[PWM] PWM Init Fail\n");
    } else {
        *(uint16_t*)0x403E002C = 0;
        *(uint16_t*)0x403E008C = 0;
        *(uint16_t*)0x403E00EC = 0;
        *(uint16_t*)0x403E014C = 0;

        pwm_signal_param_t pwmSignal;
        pwmSignal.pwmChannel = kPWM_PwmA;
        pwmSignal.level = kPWM_HighTrue;
        pwmSignal.dutyCyclePercent = 75;
        PWM_SetupPwm((void*)0x403E0000, kPWM_Module_0, &pwmSignal, 1, kPWM_EdgeAligned, 2500000, 250000000);

        PWM_SetPwmLdok((void*)0x403E0000, kPWM_Control_Module_0, true);
        PWM_StartTimer((void*)0x403E0000, kPWM_Control_Module_0);
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
