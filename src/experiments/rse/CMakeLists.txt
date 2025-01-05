file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${sources})

add_executable(rse EXCLUDE_FROM_ALL
	${sources}
)
target_include_directories(rse PUBLIC .)
target_link_libraries(rse
    PRIVATE
		common
)

