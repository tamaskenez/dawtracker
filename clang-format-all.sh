#!/bin/bash -e
git ls-files -- \*.cpp \*.h \*.hpp | xargs clang-format -i
