# A SwiftUI player on the engine's C interface: the native file panel,
# a view the GPU renderer draws into, and transport controls bound to
# the player. Built with CMake's Swift support, which wants the Ninja
# or Xcode generator.
set(SOURCE
    Player/App.swift
    Player/Engine.swift
    Player/PlayerView.swift
    Player/ContentView.swift)

add_executable(player-swift MACOSX_BUNDLE ${SOURCE})
set_target_properties(player-swift PROPERTIES
    FOLDER examples
    OUTPUT_NAME "Player"
    Swift_MODULE_NAME "Player"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME "Player"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.tlrender.player"
    MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    XCODE_ATTRIBUTE_SWIFT_VERSION 5)
# The C headers reach Swift through the module map beside them.
target_compile_options(player-swift PRIVATE
    "$<$<COMPILE_LANGUAGE:Swift>:SHELL:-Xcc -I${PROJECT_SOURCE_DIR}/lib>"
    "$<$<COMPILE_LANGUAGE:Swift>:SHELL:-Xcc -I${PROJECT_SOURCE_DIR}/deps/ftk/lib>"
    "$<$<COMPILE_LANGUAGE:Swift>:SHELL:-Xcc -fmodule-map-file=${PROJECT_SOURCE_DIR}/lib/tlRender/C/module.modulemap>")
target_link_libraries(player-swift PRIVATE tlC "c++")
