cmake_minimum_required(VERSION 3.10)
project(automaton_simulator CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(automaton_simulator
    src/main.cpp
    src/LambdaNfa.cpp
    src/Pda.cpp
    src/Cfg.cpp
)