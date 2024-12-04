file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${sources})

add_executable(main MACOSX_BUNDLE WIN32
	${sources}
)

target_link_libraries(main PRIVATE
	imgui_backend
	common
	app
	ui
)
