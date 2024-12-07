set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

include(${CMAKE_SOURCE_DIR}/cmake/warnings_clang.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/warnings_gcc.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/warnings_msvc.cmake)

add_subdirectory(common)
add_subdirectory(platform) # knows common
add_subdirectory(audio) # knows common
add_subdirectory(ui) # knows common, platform
add_subdirectory(app) # knows common, audio, ui, platform
add_subdirectory(app_main) # knows common, app, platform
add_subdirectory(experiments)