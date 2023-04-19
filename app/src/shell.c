#include <zephyr/device.h>
#include <zephyr/shell/shell.h>
#include <zephyr/fs/fs.h>
#include <ff.h>

#include "lua/lauxlib.h"
#include "lua/lua.h"

extern lua_State* L;

void cmd_lua_loadfile(const struct shell * shell, size_t argc, char ** argv) {
    if (argc == 2) {
        char* path = argv[1];

        // Load init.lua
        struct fs_file_t init;
        fs_file_t_init(&init);
        if(0 == fs_open(&init, path, 1)) {
            fs_seek(&init, 0, FS_SEEK_END);
            size_t size = fs_tell(&init);
            char * str = malloc(size+1);
            str[size] = 0;
            fs_seek(&init, 0, FS_SEEK_SET);
            fs_read(&init, str, size);
            int err = luaL_dostring(L, str);
            if (err) {shell_print(shell, "Lua file executed with errors: %i", err);} else {
                shell_print(shell, "Lua file executed successfully.");
            }
            free(str);
        } else {
            shell_print(shell, "Could not open the file %s", path);
        }
        fs_close(&init);
    } else {
        shell_print(shell, "Invalid # of arguments %i", argc);
    }
}

void cmd_lua(const struct shell * shell, size_t argc, char ** argv) {
    if (argc == 2) {
        char* arg = argv[1];
        luaL_loadstring(L, arg);
        int err = lua_pcall(L, 0, 0, 0);
        if (err) {
            const char* str = luaL_checkstring(L, 1);
            if (str) {
                shell_print(shell, "<ERR %i: %s>", err, str);
                lua_pop(L, 1);
            } else {
                shell_print(shell, "<ERR %i>", err);
            }
        }
        fflush(stdout);
    } else if (argc == 1) {
        shell_print(shell, "Lua 5.4.4 %i", L);
    } else {
        shell_print(shell, "Invalid # of arguments %i", argc);
    }
}

SHELL_CMD_REGISTER(lua, NULL, "Execute lua code", cmd_lua);
SHELL_CMD_REGISTER(lua_lf, NULL, "Run the given lua file", cmd_lua_loadfile);
