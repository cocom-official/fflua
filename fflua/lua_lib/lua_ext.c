#include <stdbool.h>
#include <string.h>

#include "lua_ext.h"

inline static int append_str(char *buff, int buff_size, const char *str)
{
    strncpy_s(buff + strlen(buff), buff_size, str, buff_size);

    return strlen(buff);
}

int lua_dump_stack(lua_State *L, char *str, int buff_size)
{
    memset(str, 0, buff_size);

    int top = lua_gettop(L);
    lua_getglobal(L, "tostring");

    for (int i = 1; i <= top; i++)
    {
        const char *s;
        size_t l;
        lua_pushvalue(L, -1); /* function to be called */
        lua_pushvalue(L, i);  /* value to print */
        lua_call(L, 1, 1);
        s = lua_tolstring(L, -1, &l); /* get result */

        if (s == NULL)
        {
            return 0;
        }

        if (i > 1)
        {
            append_str(str, buff_size, "\t");
        }

        append_str(str, buff_size, s);
        lua_pop(L, 1); /* pop result */
    }
    append_str(str, buff_size, "\n");

    return strlen(str);
}
