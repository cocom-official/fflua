#pragma once

#include <stdbool.h>

#include "lua/lua.h"

extern int lua_dump_stack(lua_State *L, char *str, int buff_size);
