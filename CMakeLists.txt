cmake_minimum_required(VERSION 3.29)
project(LangLawyer)

set(CMAKE_CXX_STANDARD 20)

add_executable(concept_auto type-constraint-auto.cpp)
add_executable(decl-auto decl-auto.cpp)
add_executable(auto-deduce auto-deduce.cpp)