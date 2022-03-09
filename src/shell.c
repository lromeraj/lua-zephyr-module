
#include <zephyr.h>
#include <device.h>
#include <shell/shell.h>

#include "lua/lauxlib.h"
#include "lua/lua.h"

extern lua_State* L;
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
