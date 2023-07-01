set(HEADERS
    App.h
    AudioMenu.h
    CompareMenu.h
    CompareToolBar.h
    FileMenu.h
    FileToolBar.h
    FrameMenu.h
    MainWindow.h
    PlaybackMenu.h
    RenderMenu.h
    ToolsMenu.h
    ToolsToolBar.h
    ViewMenu.h
    ViewToolBar.h
    WindowMenu.h
    WindowToolBar.h)

set(SOURCE
    App.cpp
    AudioMenu.cpp
    CompareMenu.cpp
    CompareToolBar.cpp
    FileMenu.cpp
    FileToolBar.cpp
    FrameMenu.cpp
    MainWindow.cpp
    PlaybackMenu.cpp
    RenderMenu.cpp
    ViewMenu.cpp
    ViewToolBar.cpp
    ToolsMenu.cpp
    ToolsToolBar.cpp
    WindowMenu.cpp
    WindowToolBar.cpp)

add_library(tlPlayGLApp ${HEADERS} ${SOURCE})
target_link_libraries(tlPlayGLApp tlPlay tlTimelineUI tlGLApp)
set_target_properties(tlPlayGLApp PROPERTIES FOLDER lib)
set_target_properties(tlPlayGLApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlPlayGLApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlPlayGLApp)
