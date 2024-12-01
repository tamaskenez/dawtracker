#!/bin/bash -e

cmake -D CMAKE_INSTALL_PREFIX=${PWD}/id \
	-D BUILD_DIR=${PWD}/bd \
	-P thirdparty/thirdparty.cmake
