set(HEADERS
    EventLoop.h
    IWidget.h
    Window.h
    RowLayout.h
    StackLayout.h
    Style.h
    TextLabel.h)
set(HEADERS_PRIVATE)

set(SOURCE
    EventLoop.cpp
    IWidget.cpp
    Window.cpp
    RowLayout.cpp
    StackLayout.cpp
    Style.cpp
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
