set(HEADERS
    DrawUtil.h
    Event.h
    EventLoop.h
    FloatEdit.h
    FloatModel.h
    FloatSlider.h
    GeometryUtil.h
    GridLayout.h
    GroupBox.h
    IButton.h
    IntEdit.h
    IntModel.h
    IntSlider.h
    IWidget.h
    IWidgetOptions.h
    IconLibrary.h
    Label.h
    LineEdit.h
    ListButton.h
    PushButton.h
    RowLayout.h
    ScrollArea.h
    Spacer.h
    StackLayout.h
    Style.h
    ToolButton.h)
set(HEADERS_PRIVATE)

set(SOURCE
    DrawUtil.cpp
    Event.cpp
    EventLoop.cpp
    FloatEdit.cpp
    FloatModel.cpp
    FloatSlider.cpp
    GeometryUtil.cpp
    GridLayout.cpp
    GroupBox.cpp
    IButton.cpp
    IntEdit.cpp
    IntModel.cpp
    IntSlider.cpp
    IWidget.cpp
    IWidgetOptions.cpp
    IconLibrary.cpp
    Label.cpp
    LineEdit.cpp
    ListButton.cpp
    PushButton.cpp
    RowLayout.cpp
    ScrollArea.cpp
    Spacer.cpp
    StackLayout.cpp
    Style.cpp
    ToolButton.cpp)

add_library(tlUI ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlUI PUBLIC tlTimeline PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlUI PROPERTIES FOLDER lib)
set_target_properties(tlUI PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlUI
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlUI)
