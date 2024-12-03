#!/bin/bash

if [ ! -d "build" ]; then
  echo "Directory 'build' does not exist. Creating it..."
  mkdir build
fi

cd build || exit 1

if [ ! -f "CMakeCache.txt" ] || [ ! -f "Makefile" ]; then
  echo "CMakeCache.txt not found, configuring the project..."
  cmake -G "Unix Makefiles" ..
  if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
  fi
fi

echo "Preparing test files..."
cmake --build . --target PrepareTestFiles 
if [ $? -ne 0 ]; then
  echo "Build target PrepareTestFiles failed!"
  exit 1
fi

make -j4

find . > ../files_before.txt

echo "Running tests..."
ctest -R "^s_test_|^f_test_"

find . > ../files_after.txt
sort ../files_before.txt > ../files_before_sorted.txt
sort ../files_after.txt > ../files_after_sorted.txt
comm -13 ../files_before_sorted.txt ../files_after_sorted.txt | xargs rm -rf

rm ../files_before.txt ../files_after.txt ../files_before_sorted.txt ../files_after_sorted.txt

echo "Saving file list after tests..."



echo "Cleanup complete."
