set(HEADERS
    ButtonGroup.h
    ComboBox.h
    DrawUtil.h
    Event.h
    EventLoop.h
    FloatEdit.h
    FloatEditSlider.h
    FloatModel.h
    FloatSlider.h
    GeometryUtil.h
    GridLayout.h
    GroupBox.h
    IButton.h
    IClipboard.h
    IWidget.h
    IWidgetInline.h
    IWidgetOptions.h
    IconLibrary.h
    IncButtons.h
    IntEdit.h
    IntEditSlider.h
    IntModel.h
    IntSlider.h
    Label.h
    LineEdit.h
    ListButton.h
    PieChart.h
    PushButton.h
    RowLayout.h
    ScrollArea.h
    ScrollWidget.h
    Spacer.h
    ScrollBar.h
    Splitter.h
    StackLayout.h
    Style.h
    TimeEdit.h
    TimeUnitsModel.h
    ToolButton.h)
set(HEADERS_PRIVATE)

set(SOURCE
    ButtonGroup.cpp
    ComboBox.cpp
    DrawUtil.cpp
    Event.cpp
    EventLoop.cpp
    FloatEdit.cpp
    FloatEditSlider.cpp
    FloatModel.cpp
    FloatSlider.cpp
    GeometryUtil.cpp
    GridLayout.cpp
    GroupBox.cpp
    IButton.cpp
    IClipboard.cpp
    IWidget.cpp
    IWidgetOptions.cpp
    IconLibrary.cpp
    IncButtons.cpp
    IntEdit.cpp
    IntEditSlider.cpp
    IntModel.cpp
    IntSlider.cpp
    Label.cpp
    LineEdit.cpp
    ListButton.cpp
    PieChart.cpp
    PushButton.cpp
    RowLayout.cpp
    ScrollArea.cpp
    ScrollWidget.cpp
    Spacer.cpp
    ScrollBar.cpp
    Splitter.cpp
    StackLayout.cpp
    Style.cpp
    TimeEdit.cpp
    TimeUnitsModel.cpp
    ToolButton.cpp)

add_library(tlUI ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlUI PUBLIC tlIO PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlUI PROPERTIES FOLDER lib)
set_target_properties(tlUI PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlUI
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlUI)
