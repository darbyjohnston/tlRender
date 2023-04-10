set(HEADERS
    App.h
    AudioClipItem.h
    AudioGapItem.h
    IItem.h
    IOManager.h
    MainWindow.h
    TimelineItem.h
    TimelineScrollArea.h
    TimelineWidget.h
    TrackItem.h
    VideoClipItem.h
    VideoGapItem.h)

set(SOURCE
    App.cpp
    AudioClipItem.cpp
    AudioGapItem.cpp
    IItem.cpp
    IOManager.cpp
    MainWindow.cpp
    TimelineItem.cpp
    TimelineScrollArea.cpp
    TimelineWidget.cpp
    TrackItem.cpp
    VideoClipItem.cpp
    VideoGapItem.cpp
    main.cpp)

add_executable(timeline-qtwidget ${SOURCE} ${HEADERS})
target_link_libraries(timeline-qtwidget tlQtWidget tlUI tlApp)
set_target_properties(timeline-qtwidget PROPERTIES FOLDER examples)
