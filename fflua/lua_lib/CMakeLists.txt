cmake_minimum_required(VERSION 3.5)

project(lua LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -O2 -Wstringop-overflow=0)

set(lua_core_list
    "lua/lapi.c"
    "lua/lcode.c"
    "lua/lctype.c"
    "lua/ldebug.c"
    "lua/ldo.c"
    "lua/ldump.c"
    "lua/lfunc.c"
    "lua/lgc.c"
    "lua/llex.c"
    "lua/lmem.c"
    "lua/lobject.c"
    "lua/lopcodes.c"
    "lua/lparser.c"
    "lua/lstate.c"
    "lua/lstring.c"
    "lua/ltable.c"
    "lua/ltm.c"
    "lua/lundump.c"
    "lua/lvm.c"
    "lua/lzio.c"
    "lua/ltests.c"
)

set(lua_auxlib_list
    "lua/lauxlib.c"
)

set(lua_lib_list
    "lua/lbaselib.c"
    "lua/ldblib.c"
    "lua/liolib.c"
    "lua/lmathlib.c"
    "lua/loslib.c"
    "lua/ltablib.c"
    "lua/lstrlib.c"
    "lua/lutf8lib.c"
    "lua/loadlib.c"
    "lua/lcorolib.c"
    "lua/linit.c"
)

set(lua_ext_list
    "lua_ext.c"
    "lua_repl.c"
)

add_library(${PROJECT_NAME} STATIC ${lua_core_list} ${lua_auxlib_list} ${lua_lib_list} ${lua_ext_list})

target_include_directories(
    ${PROJECT_NAME} PRIVATE
    "lua"
)
target_include_directories(
    ${PROJECT_NAME} INTERFACE
    "."
)

add_executable(${PROJECT_NAME}_interpreter ${lua_core_list} ${lua_auxlib_list} ${lua_lib_list} lua/lua.c)
