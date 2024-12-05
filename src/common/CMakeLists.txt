file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${sources})

add_library(common STATIC
	${sources}
)

target_include_directories(common PUBLIC .)
target_link_libraries(common
	PUBLIC
		absl::log
		fmt::fmt
)
