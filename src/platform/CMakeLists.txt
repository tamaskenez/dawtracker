file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${sources})

add_library(platform STATIC
	${sources}
)
target_include_directories(platform PUBLIC .)
target_link_libraries(platform
	PUBLIC
		common
    PRIVATE
        SDL3::SDL3
        concurrentqueue::concurrentqueue
)

