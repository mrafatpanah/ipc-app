cmake_minimum_required(VERSION 3.10)
project(CppIPCExample CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(ipc_app main.cpp)

message(STATUS "Configuring CppIPCExample project")