cmake_minimum_required(VERSION 3.5)

project(fflua LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC "fflua/null.cpp")

add_subdirectory("fflua/lua_lib")
target_link_libraries(${PROJECT_NAME} PUBLIC "lua")
add_dependencies(${PROJECT_NAME} "lua")

target_include_directories(
    ${PROJECT_NAME} INTERFACE
    "fflua/"
)

if (NOT CMAKE_BUILD_TYPE STREQUAL Release)
if (NOT NO_FFLUA_TEST STREQUAL "y")
    add_executable(${PROJECT_NAME}_test "example/main.cpp")
    target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND bash -c "cp ${CMAKE_SOURCE_DIR}/example/fflua_test.lua ${CMAKE_BINARY_DIR}/fflua_test.lua"
        COMMAND bash -c "cp ${CMAKE_SOURCE_DIR}/example/lua_tets_all.lua ${CMAKE_BINARY_DIR}/lua_tets_all.lua"
    )
endif()
endif()
