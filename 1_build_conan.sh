#!/bin/bash -e

# Installs to a subdirectory `id`

conan install conanfile.txt -b missing -pr:b default -of id/cmake -s build_type=Debug
conan install conanfile.txt -b missing -pr:b default -of id/cmake -s build_type=Release
