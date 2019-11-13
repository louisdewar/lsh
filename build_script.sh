#!/bin/bash

BUILD_DIR=build

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ..
cmake --build .

echo Built to $BUILD_DIR, run ./$BUILD_DIR/lsh to start the program
