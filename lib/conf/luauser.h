#ifndef _LUA_USER_H_
#define _LUA_USER_H_

#include <zephyr/sys/printk.h>

#define luai_userstateopen(L) luauser_stateopen(L)
#define luai_userstateclose(L) luauser_stateclose(L)

#ifdef CONFIG_LUA_PRINTK
	#define lua_writestring(s,l) \
		printk("%s", (s)) 

	#define lua_writestringerror(s,p) \
		printk((s), (p))
#endif

#ifdef CONFIG_LUA_GLOCK
	#include <zephyr/sys/mutex.h>

	void luauser_lock(lua_State *L);
	void luauser_unlock(lua_State *L);

	#define lua_lock(L) luauser_lock(L)
	#define lua_unlock(L) luauser_unlock(L)
#endif

void luauser_stateopen(lua_State *L);
void luauser_stateclose(lua_State *L);

#endif
