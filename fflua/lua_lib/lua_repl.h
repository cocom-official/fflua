#pragma once

#include <stdbool.h>

#include "lua/lua.h"

#ifndef LUA_REPL_STATE_MAX_BUFF_SIZE
#define LUA_REPL_STATE_MAX_BUFF_SIZE 1024
#endif

typedef struct
{
    lua_State *L;
    int status;
    bool init;
    bool complete;
    bool multiline;
    char str[LUA_REPL_STATE_MAX_BUFF_SIZE];
} lua_repl_instance;

extern bool lua_repl_instance_init(lua_repl_instance *instance);
extern void lua_repl_instance_deinit(lua_repl_instance *instance); /* instance should free by caller after deinit */
extern bool lua_repl_push_str(lua_repl_instance *instance, const char *b);
