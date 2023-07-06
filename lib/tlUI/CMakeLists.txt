set(HEADERS
    ButtonGroup.h
    ComboBox.h
    Divider.h
    DoubleEdit.h
    DoubleModel.h
    DrawUtil.h
    DrivesModel.h
    Event.h
    EventLoop.h
    FileBrowser.h
    FloatEdit.h
    FloatEditSlider.h
    FloatModel.h
    FloatSlider.h
    GridLayout.h
    GroupBox.h
    IButton.h
    IClipboard.h
    IDialog.h
    IMenuPopup.h
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
    RecentFilesModel.h
    RowLayout.h
    ScrollArea.h
    ScrollWidget.h
    Spacer.h
    ScrollBar.h
    Splitter.h
    StackLayout.h
    Style.h
    StyleInline.h
    TimeEdit.h
    TimeLabel.h
    ToolButton.h)
set(HEADERS_PRIVATE
    FileBrowserPrivate.h)

set(SOURCE
    ButtonGroup.cpp
    ComboBox.cpp
    Divider.cpp
    DoubleEdit.cpp
    DoubleModel.cpp
    DrawUtil.cpp
    DrivesModel.cpp
    Event.cpp
    EventLoop.cpp
    FileBrowser.cpp
    FileBrowserButton.cpp
    FileBrowserDirectoryWidget.cpp
    FileBrowserPathsWidget.cpp
    FileBrowserWidget.cpp
    FloatEdit.cpp
    FloatEditSlider.cpp
    FloatModel.cpp
    FloatSlider.cpp
    GridLayout.cpp
    GroupBox.cpp
    IButton.cpp
    IClipboard.cpp
    IDialog.cpp
    IMenuPopup.cpp
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
    RecentFilesModel.cpp
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
