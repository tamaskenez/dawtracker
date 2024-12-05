file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${sources})

add_executable(app_main MACOSX_BUNDLE WIN32
	${sources}
)

target_link_libraries(app_main PRIVATE
	imgui_backend
	common
	app
	ui
	platform
)
