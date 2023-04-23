set(HEADERS
    ButtonGroup.h
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
    ITimelineItem.h
    IWidget.h
    IWidgetInline.h
    IWidgetOptions.h
    IconLibrary.h
    IntEdit.h
    IntModel.h
    IntSlider.h
    Label.h
    LineEdit.h
    ListButton.h
    PieChart.h
    PushButton.h
    RowLayout.h
    ScrollArea.h
    Spacer.h
    StackLayout.h
    Style.h
    TimelineAudioClipItem.h
    TimelineAudioGapItem.h
    TimelineIOManager.h
    TimelineItem.h
    TimelineTrackItem.h
    TimelineVideoClipItem.h
    TimelineVideoGapItem.h
    ToolButton.h)
set(HEADERS_PRIVATE)

set(SOURCE
    ButtonGroup.cpp
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
    ITimelineItem.cpp
    IWidget.cpp
    IWidgetOptions.cpp
    IconLibrary.cpp
    IntEdit.cpp
    IntModel.cpp
    IntSlider.cpp
    Label.cpp
    LineEdit.cpp
    ListButton.cpp
    PieChart.cpp
    PushButton.cpp
    RowLayout.cpp
    ScrollArea.cpp
    Spacer.cpp
    StackLayout.cpp
    Style.cpp
    TimelineAudioClipItem.cpp
    TimelineAudioGapItem.cpp
    TimelineIOManager.cpp
    TimelineItem.cpp
    TimelineTrackItem.cpp
    TimelineVideoClipItem.cpp
    TimelineVideoGapItem.cpp
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
