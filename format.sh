#!/bin/bash

find . -name "*.h" | xargs clang-format -i
find . -name "*.c" | xargs clang-format -i
find . -name "*.hpp" | xargs clang-format -i
find . -name "*.cpp" | xargs clang-format -i
