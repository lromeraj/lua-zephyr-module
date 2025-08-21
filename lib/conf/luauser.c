#include "lua.h"
#include "luauser.h"

#define _UNUSED(...) (void)(__VA_ARGS__)


#ifdef CONFIG_LUA_GLOCK

static struct {
	struct k_mutex m;
	int init;
} _lock;

#endif

void luauser_stateopen(lua_State *L)
{
	_UNUSED(L);

#ifdef CONFIG_LUA_GLOCK
	if (!_lock.init) {
		k_mutex_init(&_lock.m);
		_lock.init = 1;
	}
#endif

}

void luauser_stateclose(lua_State *L)
{
	_UNUSED(L);
#ifdef CONFIG_LUA_GLOCK
	if (_lock.init) {
		_lock.init = 0;
	}
#endif
}

#ifdef CONFIG_LUA_GLOCK
void luauser_lock(lua_State *L)
{
	_UNUSED(L);
	k_mutex_lock(&_lock.m, K_FOREVER);
}

void luauser_unlock(lua_State *L)
{
	_UNUSED(L);
	k_mutex_unlock(&_lock.m);
}
#endif