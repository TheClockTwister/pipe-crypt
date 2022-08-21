cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(pipe-crypt)

add_subdirectory(lib/cryptopp)
add_subdirectory(lib/argparse)

include_directories(lib/argparse/include)

add_executable(pipe-crypt src/main.cpp)
target_link_libraries(pipe-crypt cryptopp pthread) ## Specifying cryptopp in uppercase gave me link error.
target_precompile_headers(pipe-crypt PRIVATE src/main.hpp)
target_compile_options(pipe-crypt PRIVATE -fpermissive)

add_executable(Test test.cpp)
target_link_libraries(Test cryptopp) ## Specifying cryptopp in uppercase gave me link error.

add_compile_options(-g)
