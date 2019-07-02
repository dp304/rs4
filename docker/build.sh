#!/bin/bash
set +ex

cd $HOME

git clone https://github.com/dp304/rs4.git || true
cd rs4
git clean -ffdx
git pull
git submodule update --init --recursive

mkdir build
cd build
conan install .. --build=missing

cmake ..
cmake --build .
