cmake_minimum_required(VERSION 3.5)

project(fflua LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

add_library(${PROJECT_NAME} STATIC "fflua/null.cpp")

add_subdirectory("fflua/lua_lib")
target_link_libraries(${PROJECT_NAME} PUBLIC "lua")
add_dependencies(${PROJECT_NAME} "lua")

target_include_directories(
    ${PROJECT_NAME} INTERFACE
    "fflua/"
    )

add_executable(${PROJECT_NAME}_test "example/main.cpp")
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})
