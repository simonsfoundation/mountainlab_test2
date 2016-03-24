#/bin/bash
clang-format -i -style=WebKit ../mountainview/src/*.h
clang-format -i -style=WebKit ../mountainview/src/*.cpp

clang-format -i -style=WebKit ../mountainsort/src/*.h
clang-format -i -style=WebKit ../mountainsort/src/*.cpp
