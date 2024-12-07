file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${sources})

add_executable(audiodevicemanager
	${sources}
)
target_precompile_headers(audiodevicemanager PRIVATE pch.h)
target_include_directories(audiodevicemanager PUBLIC .)
target_link_libraries(audiodevicemanager
    PRIVATE
		common
		jucemodules
)

