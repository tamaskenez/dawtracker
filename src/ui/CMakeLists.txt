file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${sources})

add_library(ui STATIC
	${sources}
)
target_precompile_headers(ui PRIVATE src/pch.h)

target_include_directories(ui PUBLIC .)
target_link_libraries(ui
	PUBLIC
		common
    PRIVATE
        imgui::imgui
        platform
)

