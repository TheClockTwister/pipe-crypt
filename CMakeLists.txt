cmake_minimum_required(VERSION 3.16)
project(PipeCrypt)

# all the optimizations for maximum speed and all the errors
add_compile_options(-O3 -Wall -Wextra -pedantic -Werror)

add_executable(PipeCrypt main.cpp)
TARGET_LINK_LIBRARIES(PipeCrypt cryptopp ) ## Specifying cryptopp in uppercase gave me link error.

target_precompile_headers(PipeCrypt PRIVATE headers.hpp)
