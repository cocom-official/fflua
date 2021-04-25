#include <stdbool.h>
#include <string.h>

#include "lua_ext.h"

inline static int append_str(char *buff, int buff_size, const char *str)
{
    size_t len = strlen(buff);
    size_t remain_len = buff_size - len - 1;

    if (remain_len <= 0)
    {
        return len;
    }

    strncpy(buff + len, str, remain_len);

    if (strlen(str) >= buff_size - len - 1)
    {
        buff[buff_size -1] = '0';
    }

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
