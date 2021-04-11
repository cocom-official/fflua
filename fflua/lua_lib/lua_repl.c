#include <stdbool.h>
#include <string.h>

#include "lauxlib.h"
#include "lualib.h"

#include "lua_repl.h"

#ifndef LUA_REPL_MAX_INSTANCE
#define LUA_REPL_MAX_INSTANCE 10
#endif

#define MULTI_LINE_INCOMPLATE (-1)

#if !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT 512
#endif

#if !defined(LUA_BUFF_NAME)
#define LUA_BUFF_NAME "=stdin"
#endif

/* mark in error messages for incomplete statements */
#define EOFMARK "<eof>"
#define marklen (sizeof(EOFMARK) / sizeof(char) - 1)

static lua_repl_instance *repl_instance[LUA_REPL_MAX_INSTANCE] = {NULL};

static bool try_add_instance(lua_repl_instance *instance)
{
    for (int i = 0; i < LUA_REPL_MAX_INSTANCE; i++)
    {
        if (repl_instance[i] != NULL)
        {
            if (repl_instance[i] == instance)
            {
                return true;
            }
            else
            {
                continue;
            }
        }
        else
        {
            printf("new instance added %d 0x%p\n", i, instance);
            repl_instance[i] = instance;
            return true;
        }
    }

    fprintf(stderr, "repl instance number reach max %d\n", LUA_REPL_MAX_INSTANCE);
    return false;
}

static void set_instance_str(lua_repl_instance *instance, const char *str)
{
    strncpy_s(instance->str, LUA_REPL_STATE_MAX_BUFF_SIZE, str, LUA_REPL_STATE_MAX_BUFF_SIZE);
}

static int append_instance_str(lua_repl_instance *instance, const char *str)
{
    strncpy_s(instance->str + strlen(instance->str), LUA_REPL_STATE_MAX_BUFF_SIZE, str, LUA_REPL_STATE_MAX_BUFF_SIZE);

    return strlen(instance->str);
}

static lua_repl_instance *find_instance_form_lua_State(lua_State *L)
{
    for (int i = 0; i < LUA_REPL_MAX_INSTANCE; i++)
    {
        if (repl_instance[i] != NULL && repl_instance[i]->L == L)
        {
            return repl_instance[i];
        }
    }

    return NULL;
}

static bool pushline(lua_State *L, const char *b, bool firstline)
{
    char buffer[LUA_MAXINPUT] = {0};
    strncpy_s(buffer, LUA_MAXINPUT, b, LUA_MAXINPUT);

    size_t l = strlen(buffer);
    if (l > 0 && buffer[l - 1] == '\n')
    {
        buffer[--l] = '\0';
    }

    if (firstline && b[0] == '=')
    {
        /* for compatibility with 5.2, ... */
        /* change '=' to 'return' */
        lua_pushfstring(L, "return %s", b + 1);
    }
    else
    {
        lua_pushlstring(L, b, l);
    }

    return true;
}

static bool incomplete(lua_State *L, int status)
{
    if (status == LUA_ERRSYNTAX)
    {
        size_t lm_sg;
        const char *msg = lua_tolstring(L, -1, &lm_sg);

        if (lm_sg >= marklen && strcmp(msg + lm_sg - marklen, EOFMARK) == 0)
        {
            lua_pop(L, 1);
            return true;
        }
    }

    return false;
}

static int multiline(lua_repl_instance *state, const char *b)
{
    lua_State *L = state->L;
    size_t len = 0;
    const char *line = lua_tolstring(L, 1, &len);
    int status = luaL_loadbuffer(L, line, len, LUA_BUFF_NAME);

    if (!incomplete(L, status) || !pushline(L, b, false))
    {
        state->complete = true;
        return status;
    }
    else
    {
        state->complete = false;
        lua_pushliteral(L, "\n"); /* add newline... */
        lua_insert(L, -2);        /* ...between the two lines */
        lua_concat(L, 3);         /* join them */
        return MULTI_LINE_INCOMPLATE;
    }
}

static int addreturn(lua_State *L)
{
    const char *line = lua_tostring(L, -1);
    const char *retline = lua_pushfstring(L, "return %s;", line);
    int status = luaL_loadbuffer(L, retline, strlen(retline), LUA_BUFF_NAME);

    if (status == LUA_OK)
    {
        /* remove modified line */
        lua_remove(L, -2);
    }
    else
    {
        /* pop result from 'luaL_loadbuffer' and modified line */
        lua_pop(L, 2);
    }

    return status;
}

static int loadline(lua_repl_instance *state, const char *b)
{
    lua_State *L = state->L;
    int status = LUA_OK;

    if (!state->multiline)
    {
        lua_settop(L, 0);
        pushline(L, b, true);

        if ((status = addreturn(L)) != LUA_OK)
        {
            if (status == LUA_ERRSYNTAX)
            {
                /* try one more time */
                status = multiline(state, "");
            }
            state->multiline = true;
        }
        else
        {
            state->complete = true;
        }
    }
    else
    {
        /* try as command, maybe with continuation lines */
        status = multiline(state, b);
        if (status == MULTI_LINE_INCOMPLATE)
        {
            /* try one more time */
            status = multiline(state, "");
        }
    }

    if (state->complete == true)
    {
        lua_remove(L, 1); /* remove line from the stack */
        lua_assert(lua_gettop(L) == 1);
    }

    return status;
}

static void report_stack(lua_State *L)
{
    lua_repl_instance *instance = find_instance_form_lua_State(L);

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
            return;
        }
        if (i > 1)
        {
            append_instance_str(instance, "\t");
        }

        append_instance_str(instance, s);
        lua_pop(L, 1); /* pop result */
    }
    append_instance_str(instance, "\n");
}

static int report_error(lua_State *L, int status)
{
    if (status != LUA_OK)
    {
        lua_repl_instance *instance = find_instance_form_lua_State(L);
        const char *msg = lua_tostring(L, -1);
        if (instance != NULL && msg != NULL)
        {
            set_instance_str(find_instance_form_lua_State(L), msg);
        }
        lua_pop(L, 1);
    }

    return status;
}

static int msg_handler(lua_State *L)
{
    lua_repl_instance *instance = find_instance_form_lua_State(L);
    const char *msg = lua_tostring(L, 1);
    if (msg == NULL)
    {
        if (luaL_callmeta(L, 1, "__tostring") &&
            lua_type(L, -1) == LUA_TSTRING)
        {
            return 1;
        }
        msg = lua_pushfstring(L, "(error object is a %s value)",
                              luaL_typename(L, 1));
    }
    luaL_traceback(L, L, msg, 1);

    msg = lua_tostring(L, -1);
    if (instance != NULL && msg != NULL)
    {
        set_instance_str(find_instance_form_lua_State(L), msg);
    }

    return 1; /* return the traceback */
}

static int docall(lua_State *L, int nargs, int res)
{
    int base = lua_gettop(L) - nargs;             /* function index */
    lua_pushcfunction(L, msg_handler);           /* push message handler */
    lua_insert(L, base);                         /* put it under function and args */
    int status = lua_pcall(L, nargs, res, base); /* call */
    lua_remove(L, base);                         /* remove message handler from the stack */
    return status;
}

bool lua_repl_push_str(lua_repl_instance *state, const char *b)
{
    if (state == NULL || b == NULL)
    {
        return false;
    }

    if (state->complete)
    {
        state->complete = false;
        state->multiline = false;
        memset(state->str, 0, LUA_REPL_STATE_MAX_BUFF_SIZE);
    }

    int status = loadline(state, b);

    if (state->complete)
    {
        lua_State *L = state->L;

        if (status == LUA_OK)
        {
            status = docall(L, 0, LUA_MULTRET);
        }

        if (status == LUA_OK)
        {
            report_stack(L);
        }
        else
        {
            report_error(L, status);
        }

        state->status = status;

        return true;
    }
    else
    {
        return false;
    }
}

bool lua_repl_instance_init(lua_repl_instance *instance)
{
    if (instance == NULL)
    {
        return false;
    }

    if (!try_add_instance(instance))
    {
        instance->status = LUA_ERRMEM;
        set_instance_str(instance, "no repl instance mem");
        return false;
    }

    instance->complete = false;
    instance->multiline = false;
    instance->status = LUA_OK;
    memset(instance->str, 0, LUA_REPL_STATE_MAX_BUFF_SIZE);
    instance->init = true;

    return true;
}

void lua_repl_instance_deinit(lua_repl_instance *instance)
{
    if (instance == NULL || instance->init == false)
    {
        return;
    }

    for (int i = 0; i < LUA_REPL_MAX_INSTANCE; i++)
    {
        if (repl_instance[i] != NULL)
        {
            if (repl_instance[i] == instance)
            {
                repl_instance[i] = NULL;
                instance->init = false;
                return;
            }
        }
    }
}
