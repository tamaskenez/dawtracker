cmake_minimum_required(VERSION 3.29)
project(imgui)

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED)

find_package(freetype REQUIRED CONFIG)

add_library(imgui STATIC
    imgui.cpp
    imgui.h
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_internal.h
    imgui_tables.cpp
    imgui_widgets.cpp
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    misc/freetype/imgui_freetype.cpp
    misc/freetype/imgui_freetype.h
)
target_include_directories(imgui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<INSTALL_INTERFACE:include>)

target_compile_definitions(imgui PUBLIC
    IMGUI_ENABLE_FREETYPE
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    IMGUI_DEFINE_MATH_OPERATORS
    IMGUI_ENABLE_WIN32_DEFAULT_IME_FUNCTIONS
    IMGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS
)

target_link_libraries(imgui
    PRIVATE
        Freetype::Freetype
)

if(APPLE)
target_link_libraries(imgui
    PRIVATE
        $<LINK_LIBRARY:FRAMEWORK,ApplicationServices>
)
endif()
    
install(TARGETS imgui
    EXPORT imgui-targets DESTINATION lib)
install(EXPORT imgui-targets
    FILE imgui-config.cmake
    NAMESPACE imgui::
    DESTINATION lib/cmake/imgui)
install(FILES imgui.h imconfig.h
    DESTINATION include)
