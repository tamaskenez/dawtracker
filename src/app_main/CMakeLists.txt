file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${sources})

add_executable(${CMAKE_PROJECT_NAME} MACOSX_BUNDLE WIN32
	${sources}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
	imgui_backend
	common
	app
	ui
	platform
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
      XCODE_ATTRIBUTE_PRODUCT_NAME                               "${CMAKE_PROJECT_NAME}"
      XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER                  "com.dawtracker"
      XCODE_ATTRIBUTE_MARKETING_VERSION                          "${PROJECT_VERSION}"
      # XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION                  "${BUILD_VERSION}"
      XCODE_ATTRIBUTE_GENERATE_INFOPLIST_FILE                    YES
      XCODE_ATTRIBUTE_INFOPLIST_FILE                             ""
      XCODE_ATTRIBUTE_INFOPLIST_KEY_CFBundleDisplayName          "${CMAKE_PROJECT_NAME}"
      XCODE_ATTRIBUTE_INFOPLIST_KEY_LSApplicationCategoryType    "public.app-category.music"
      # XCODE_ATTRIBUTE_INFOPLIST_KEY_NSPrincipalClass           "NSApplication"
      # XCODE_ATTRIBUTE_INFOPLIST_KEY_NSMainStoryboardFile       "Main"
      # XCODE_ATTRIBUTE_INFOPLIST_KEY_NSHumanReadableCopyright   "(c)2023 Example Co."
	  XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS                     "${CMAKE_CURRENT_LIST_DIR}/app_main.entitlements"
	  XCODE_ATTRIBUTE_INFOPLIST_KEY_NSMicrophoneUsageDescription "To enable audio input for recording in DawTracker, click OK (recommended). You can also enable audio input later in System Preferences > Security & Privacy > Privacy > Microphone."
)
