#!/bin/bash

# make sure the submodules are pulled
mkdir -p lib
git submodule update --init --recursive

# Build static library for Crypto++
cd lib/cryptopp
export CXXFLAGS="-std=c++20 -DNDEBUG -O3 -s -flto -march=native -mtune=native"
make -j $(grep -c 'cpu[0-9]' /proc/stat)
make install
cd ../..

# Build project
mkdir -p build
cd build
cmake ..
cmake --build . --target pipe-crypt
cd ..
mv build/pipe-crypt* ./
