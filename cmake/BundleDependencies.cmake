include(BundleUtilities)

if(NOT DEFINED APP_BUNDLE OR NOT EXISTS "${APP_BUNDLE}")
    message(FATAL_ERROR "APP_BUNDLE must name an existing .app bundle")
endif()
if(NOT DEFINED APP_EXECUTABLE OR NOT EXISTS "${APP_EXECUTABLE}")
    message(FATAL_ERROR "APP_EXECUTABLE must name the bundle executable")
endif()

fixup_bundle("${APP_EXECUTABLE}" "" "")

execute_process(
    COMMAND /usr/bin/codesign --force --deep --sign - "${APP_BUNDLE}"
    RESULT_VARIABLE CODESIGN_RESULT
)
if(NOT CODESIGN_RESULT EQUAL 0)
    message(FATAL_ERROR "Ad-hoc signing failed for ${APP_BUNDLE}")
endif()
