cmake_minimum_required(VERSION 3.29)
project(dawtracker)

if(NOT CMAKE_PREFIX_PATH)
	set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/id)
endif()

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

if(BUILD_TESTING)
	enable_testing()
	find_package(GTest REQUIRED CONFIG)
endif()

find_package(fmt REQUIRED CONFIG)
find_package(absl REQUIRED CONFIG)
find_package(gtest REQUIRED CONFIG)
find_package(JUCE REQUIRED CONFIG)
find_package(imgui REQUIRED CONFIG)
find_package(SDL3 REQUIRED CONFIG)
find_package(freetype REQUIRED CONFIG)
find_package(OpenGL REQUIRED)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bd/IMGUI/s)
set(IMGUI_BACKENDS_DIR ${IMGUI_DIR}/backends)

add_library(imgui_backend STATIC
	${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp
	${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.h
	${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3_loader.h
	${IMGUI_BACKENDS_DIR}/imgui_impl_sdl3.cpp
	${IMGUI_BACKENDS_DIR}/imgui_impl_sdl3.h
)

target_link_libraries(imgui_backend
	PUBLIC
		SDL3::SDL3
		imgui::imgui
		OpenGL::GL
)

target_include_directories(imgui_backend
	PUBLIC
		${IMGUI_BACKENDS_DIR}
)

include(cmake/warnings_clang.cmake)
include(cmake/warnings_gcc.cmake)
include(cmake/warnings_msvc.cmake)

add_compile_definitions(DATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data\")
add_subdirectory(src)
