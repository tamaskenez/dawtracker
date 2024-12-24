cmake_minimum_required(VERSION 3.29)
project(DawTracker
	VERSION 0.1
)

if(NOT CMAKE_PREFIX_PATH)
	set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/id)
endif()

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
find_package(concurrentqueue REQUIRED)
find_package(readerwriterqueue REQUIRED)

# IMGUI backend

block()
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bd/IMGUI/s)
set(IMGUI_BACKENDS_DIR ${IMGUI_DIR}/backends)

set(sources
	${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp
	${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.h
	${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3_loader.h
	${IMGUI_BACKENDS_DIR}/imgui_impl_sdl3.cpp
	${IMGUI_BACKENDS_DIR}/imgui_impl_sdl3.h
)
source_group(TREE ${IMGUI_BACKENDS_DIR} FILES ${sources})

add_library(imgui_backend STATIC ${sources})

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

endblock()

# JUCE

add_library(jucemodules OBJECT)

target_link_libraries(jucemodules PRIVATE
	juce::juce_audio_formats
	juce::juce_audio_devices
	juce::juce_audio_utils
	juce::juce_events
)

target_compile_definitions(jucemodules
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        DONT_SET_USING_JUCE_NAMESPACE=1
        JUCE_MODAL_LOOPS_PERMITTED=1
    INTERFACE
		$<TARGET_PROPERTY:jucemodules,COMPILE_DEFINITIONS>
)
target_include_directories(jucemodules
    INTERFACE
        $<TARGET_PROPERTY:jucemodules,INCLUDE_DIRECTORIES>
)

# Local sources

add_subdirectory(src)
