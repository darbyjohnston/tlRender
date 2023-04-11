set(HEADERS
    App.h
    ITimelineItem.h
    MainWindow.h
    TimelineAudioClipItem.h
    TimelineAudioGapItem.h
    TimelineIOManager.h
    TimelineItem.h
    TimelineScrollArea.h
    TimelineWidget.h
    TimelineTrackItem.h
    TimelineVideoClipItem.h
    TimelineVideoGapItem.h)

set(SOURCE
    App.cpp
    ITimelineItem.cpp
    MainWindow.cpp
    TimelineAudioClipItem.cpp
    TimelineAudioGapItem.cpp
    TimelineIOManager.cpp
    TimelineItem.cpp
    TimelineScrollArea.cpp
    TimelineWidget.cpp
    TimelineTrackItem.cpp
    TimelineVideoClipItem.cpp
    TimelineVideoGapItem.cpp
    main.cpp)

add_executable(timeline-qtwidget ${SOURCE} ${HEADERS})
target_link_libraries(timeline-qtwidget tlQtWidget tlUI tlApp)
set_target_properties(timeline-qtwidget PROPERTIES FOLDER examples)
