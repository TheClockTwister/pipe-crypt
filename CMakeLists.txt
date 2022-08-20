cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(PipeCrypt)

add_subdirectory(lib/cryptopp)
add_subdirectory(lib/argparser)

include_directories(lib/argparser)

add_executable(PipeCrypt src/main.cpp)
target_link_libraries(PipeCrypt cryptopp pthread) ## Specifying cryptopp in uppercase gave me link error.
target_precompile_headers(PipeCrypt PRIVATE src/main.hpp)
target_compile_options(PipeCrypt PRIVATE -fpermissive -O3)

add_executable(Test test.cpp)
target_link_libraries(Test cryptopp) ## Specifying cryptopp in uppercase gave me link error.