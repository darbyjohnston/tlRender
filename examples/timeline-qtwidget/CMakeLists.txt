set(HEADERS
    App.h
    BaseItem.h
    ClipItem.h
    GapItem.h
    MainWindow.h
    TimelineItem.h
    TimelineScrollArea.h
    TimelineWidget.h
    TrackItem.h)

set(SOURCE
    App.cpp
    BaseItem.cpp
    ClipItem.cpp
    GapItem.cpp
    MainWindow.cpp
    TimelineItem.cpp
    TimelineScrollArea.cpp
    TimelineWidget.cpp
    TrackItem.cpp
    main.cpp)

add_executable(timeline-qtwidget ${SOURCE} ${HEADERS})
target_link_libraries(timeline-qtwidget tlQtWidget tlApp)
set_target_properties(timeline-qtwidget PROPERTIES FOLDER examples)
