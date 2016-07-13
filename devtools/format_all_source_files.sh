#/bin/bash
exe=clang-format-3.6

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR/..
find . -iname *.h -o -iname *.cpp | xargs $exe -i -style=WebKit

#$exe -i -style=WebKit ../mountainview/src/*.h
#$exe -i -style=WebKit ../mountainview/src/*.cpp

#$exe -i -style=WebKit ../mountainsort/src/*.h
#$exe -i -style=WebKit ../mountainsort/src/*.cpp
