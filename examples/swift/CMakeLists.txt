# A SwiftUI player on the engine's C interface: the native file panel,
# a view the GPU renderer draws into, and transport controls bound to
# the player. Built with CMake's Swift support, which wants the Ninja
# or Xcode generator.
set(SOURCE
    Player/App.swift
    Player/Engine.swift
    Player/PlayerView.swift
    Player/ContentView.swift)

# The same sources on macOS and iOS; the bundle's property list differs.
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(PLAYER_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info-iOS.plist.in)
else()
    set(PLAYER_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)
endif()

add_executable(player-swift MACOSX_BUNDLE ${SOURCE})
set_target_properties(player-swift PROPERTIES
    FOLDER examples
    OUTPUT_NAME "Player"
    Swift_MODULE_NAME "Player"
    MACOSX_BUNDLE_INFO_PLIST ${PLAYER_PLIST}
    MACOSX_BUNDLE_BUNDLE_NAME "Player"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.tlrender.player"
    MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    XCODE_ATTRIBUTE_SWIFT_VERSION 5
    # iPhone and iPad; Xcode's default is the iPhone alone, and it writes
    # that over the property list's UIDeviceFamily, which puts the
    # application in an iPhone-sized window on an iPad.
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
# The C headers reach Swift through the module map beside them.
target_compile_options(player-swift PRIVATE
    "$<$<COMPILE_LANGUAGE:Swift>:SHELL:-Xcc -I${PROJECT_SOURCE_DIR}/lib>"
    "$<$<COMPILE_LANGUAGE:Swift>:SHELL:-Xcc -I${PROJECT_SOURCE_DIR}/deps/ftk/lib>"
    "$<$<COMPILE_LANGUAGE:Swift>:SHELL:-Xcc -fmodule-map-file=${PROJECT_SOURCE_DIR}/lib/tlRender/C/module.modulemap>")
target_link_libraries(player-swift PRIVATE tlC "c++")
# An iOS device wants the bundle signed with a development team's
# certificate and a provisioning profile for its identifier, which the
# Xcode generator makes on its own when told the team; the toolchain file
# turns signing off for everything else, since static libraries need
# none, and the simulator takes an unsigned bundle.
if(CMAKE_SYSTEM_NAME STREQUAL "iOS" AND TLRENDER_IOS_TEAM)
    set_target_properties(player-swift PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED YES
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE Automatic
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${TLRENDER_IOS_TEAM}
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.tlrender.player")
endif()
