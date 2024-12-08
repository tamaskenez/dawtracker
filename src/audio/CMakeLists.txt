file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${sources})

add_library(audio STATIC
	${sources}
)
target_include_directories(audio PUBLIC . src)
target_link_libraries(audio
	PUBLIC
		common
    PRIVATE
        platform
        jucemodules
        readerwriterqueue::readerwriterqueue
)

