set(HEADERS
    DrawUtil.h
    Event.h
    EventLoop.h
    GroupBox.h
    IButton.h
    IWidget.h
    IconLibrary.h
    Label.h
    ListButton.h
    PushButton.h
    RowLayout.h
    ScrollArea.h
    Spacer.h
    StackLayout.h
    Style.h
    ToolButton.h
    Window.h)
set(HEADERS_PRIVATE)

set(SOURCE
    DrawUtil.cpp
    Event.cpp
    EventLoop.cpp
    GroupBox.cpp
    IButton.cpp
    IWidget.cpp
    IconLibrary.cpp
    Label.cpp
    ListButton.cpp
    PushButton.cpp
    RowLayout.cpp
    ScrollArea.cpp
    Spacer.cpp
    StackLayout.cpp
    Style.cpp
    ToolButton.cpp
    Window.cpp)

add_library(tlUI ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlUI PUBLIC tlTimeline PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlUI PROPERTIES FOLDER lib)
set_target_properties(tlUI PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlUI
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlUI)
