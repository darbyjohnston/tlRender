set(HEADERS
    Event.h
    EventLoop.h
    IWidget.h
    Window.h
    RowLayout.h
    Spacer.h
    StackLayout.h
    Style.h
    TextButton.h
    TextLabel.h)
set(HEADERS_PRIVATE)

set(SOURCE
    Event.cpp
    EventLoop.cpp
    IWidget.cpp
    Window.cpp
    RowLayout.cpp
    Spacer.cpp
    StackLayout.cpp
    Style.cpp
    TextButton.cpp
    TextLabel.cpp)

add_library(tlUI ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlUI PUBLIC tlTimeline PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlUI PROPERTIES FOLDER lib)
set_target_properties(tlUI PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlUI
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlUI)
