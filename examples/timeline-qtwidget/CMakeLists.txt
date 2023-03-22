set(HEADERS
    App.h
    #ClipItem.h
    #GapItem.h
    IItem.h
    MainWindow.h
    TimelineItem.h
    TimelineScrollArea.h
    TimelineWidget.h
    #TrackItem.h
    )

set(SOURCE
    App.cpp
    #ClipItem.cpp
    #GapItem.cpp
    IItem.cpp
    MainWindow.cpp
    TimelineItem.cpp
    TimelineScrollArea.cpp
    TimelineWidget.cpp
    #TrackItem.cpp
    main.cpp)

add_executable(timeline-qtwidget ${SOURCE} ${HEADERS})
target_link_libraries(timeline-qtwidget tlQtWidget tlUI tlApp)
set_target_properties(timeline-qtwidget PROPERTIES FOLDER examples)
