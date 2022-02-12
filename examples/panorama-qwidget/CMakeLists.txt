set(HEADERS
    PanoramaTimelineViewport.h)
set(SOURCE
    PanoramaTimelineViewport.cpp
    main.cpp)

add_executable(panorama-qwidget ${SOURCE} ${HEADERS})
target_link_libraries(panorama-qwidget tlQWidget)
set_target_properties(panorama-qwidget PROPERTIES FOLDER examples)
