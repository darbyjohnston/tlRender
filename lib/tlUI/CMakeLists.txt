set(HEADERS
    Action.h
    ButtonGroup.h
    ComboBox.h
    Divider.h
    DoubleEdit.h
    DoubleModel.h
    DrawUtil.h
    Event.h
    EventLoop.h
    FloatEdit.h
    FloatEditSlider.h
    FloatModel.h
    FloatSlider.h
    GridLayout.h
    GroupBox.h
    IButton.h
    IClipboard.h
    IPopup.h
    IWidget.h
    IWidgetInline.h
    IWidgetOptions.h
    IconLibrary.h
    IncButtons.h
    Init.h
    IntEdit.h
    IntEditSlider.h
    IntModel.h
    IntSlider.h
    Label.h
    LayoutUtil.h
    LineEdit.h
    ListButton.h
    Menu.h
    MenuBar.h
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
    TimeLabel.h
    ToolButton.h)
set(HEADERS_PRIVATE)

set(SOURCE
    Action.cpp
    ButtonGroup.cpp
    ComboBox.cpp
    Divider.cpp
    DoubleEdit.cpp
    DoubleModel.cpp
    DrawUtil.cpp
    Event.cpp
    EventLoop.cpp
    FloatEdit.cpp
    FloatEditSlider.cpp
    FloatModel.cpp
    FloatSlider.cpp
    GridLayout.cpp
    GroupBox.cpp
    IButton.cpp
    IClipboard.cpp
    IPopup.cpp
    IWidget.cpp
    IWidgetOptions.cpp
    IconLibrary.cpp
    IncButtons.cpp
    Init.cpp
    IntEdit.cpp
    IntEditSlider.cpp
    IntModel.cpp
    IntSlider.cpp
    Label.cpp
    LayoutUtil.cpp
    LineEdit.cpp
    ListButton.cpp
    Menu.cpp
    MenuBar.cpp
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
    TimeLabel.cpp
    ToolButton.cpp)

set(LIBRARIES_PRIVATE)
if(TLRENDER_NFD)
    list(APPEND LIBRARIES_PRIVATE NFD)
endif()

add_library(tlUI ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlUI PUBLIC tlIO PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlUI PROPERTIES FOLDER lib)
set_target_properties(tlUI PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlUI
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlUI)
