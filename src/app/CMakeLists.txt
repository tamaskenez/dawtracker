file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${sources})

add_library(app STATIC
	${sources}
)

target_precompile_headers(app PRIVATE src/pch.h)

target_include_directories(app PUBLIC .)

target_link_libraries(app
	PUBLIC
		common
		ui
)

