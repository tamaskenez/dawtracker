set(PROJECTS
    IMGUI
    JUCE
    SDL3
)

set(JUCE_GIT_REPOSITORY https://github.com/juce-framework/JUCE.git)
set(JUCE_GIT_BRANCH master)
set(IMGUI_GIT_REPOSITORY https://github.com/ocornut/imgui.git)
set(SDL3_GIT_REPOSITORY https://github.com/libsdl-org/SDL.git)

if(NOT CMAKE_INSTALL_PREFIX)
    message(FATAL_ERROR "Missing CMAKE_INSTALL_PREFIX")
endif()
if(NOT BUILD_DIR)
    message(FATAL_ERROR "Missing CMAKE_INSTALL_PREFIX")
endif()
if(EXISTS ${BUILD_DIR})
    if(NOT IS_DIRECTORY ${BUILD_DIR})
        message(FATAL_ERROR "BUILD_DIR is not a directory: ${BUILD_DIR}")
    endif()
else()
    file(MAKE_DIRECTORY ${BUILD_DIR})
endif()

find_package(Git REQUIRED)

# Helper macros to execute cmake commands.
macro(cmake)
    execute_process(COMMAND ${CMAKE_COMMAND} ${ARGV}
        COMMAND_ERROR_IS_FATAL ANY)
endmacro()

foreach(project IN LISTS PROJECTS)
    set(s ${BUILD_DIR}/${project}/s)
    if(NOT IS_DIRECTORY ${s})
        if(DEFINED ${project}_GIT_BRANCH)
            set(git_branch_option --branch ${${project}_GIT_BRANCH})
        else()
            unset(git_branch_option)
        endif()
        execute_process(
            COMMAND ${GIT_EXECUTABLE}
                clone --depth 1 ${git_branch_option}
                    ${${project}_GIT_REPOSITORY} ${s}
            COMMAND_ECHO STDOUT
            COMMAND_ERROR_IS_FATAL ANY
        )
    else()
        message(STATUS "Not cloning ${project}, directory exists")
    endif()
endforeach()

file(COPY_FILE ${CMAKE_CURRENT_LIST_DIR}/imgui/CMakeLists.txt ${BUILD_DIR}/IMGUI/s/CMakeLists.txt)

set(build_types Debug Release)
if(DEFINED ENV{BUILD_TYPES})
    set(build_types $ENV{BUILD_TYPES})
endif()
foreach(project IN LISTS PROJECTS)
    foreach(config IN LISTS build_types)
        set(s ${BUILD_DIR}/${project}/s)
        set(b ${BUILD_DIR}/${project}/${config})
        message(STATUS "[${project}]: calling cmake-config ${config}")
        cmake(-S ${s} -B ${b}
            -D CMAKE_BUILD_TYPE=${config}
            -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -D CMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
            -D BUILD_SHARED_LIBS=0
            -D CMAKE_DEBUG_POSTFIX=_d
            -D CMAKE_CXX_STANDARD=23
            -D CMAKE_CXX_STANDARD_REQUIRED=1
            -D CMAKE_FIND_PACKAGE_PREFER_CONFIG=1
            -D BUILD_TESTING=0
            -D JUCE_USE_CURL=0
            -D JUCE_USE_FONTCONFIG=0
            -D JUCE_USE_FREETYPE=0
            -D JUCE_USE_XCURSOR=0
            -D JUCE_USE_XINERAMA=0
            -D JUCE_USE_XRANDR=0
            -D JUCE_USE_XRENDER=0
            -D JUCE_WEB_BROWSER=0
            -D SDL_AUDIO=0
            -D SDL_CAMERA=0
            -D SDL_DIRECTX=0
            -D SDL_HAPTIC=0
            -D SDL_HIDAPI=0
            -D SDL_JOYSTICK=0
            -D SDL_OPENGLES=0
            -D SDL_POWER=0
            -D SDL_SENSOR=0
            -D SDL_VULKAN=0
            -D SDL_WASAPI=0
            -D SDL_XINPUT=0
        )
        message(STATUS "[${project}]: calling cmake-build ${config}")
        cmake(--build ${b} --target install --config ${config} -j)
    endforeach()
endforeach()
