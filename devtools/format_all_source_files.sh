#/bin/bash
exe=clang-format-3.6

$exe -i -style=WebKit ../mountainview/src/*.h
$exe -i -style=WebKit ../mountainview/src/*.cpp

$exe -i -style=WebKit ../mountainsort/src/*.h
$exe -i -style=WebKit ../mountainsort/src/*.cpp
