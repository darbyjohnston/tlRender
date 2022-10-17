set(HEADERS
    App.h
    ClipItem.h
    GapItem.h
    IGraphicsItem.h
	MainWindow.h
    SceneView.h
    StackItem.h
	TimelineItem.h
    TimelineScene.h
    TrackItem.h
    TransitionItem.h
    Util.h)

set(SOURCE
    App.cpp
    ClipItem.cpp
    GapItem.cpp
    IGraphicsItem.cpp
	MainWindow.cpp
    SceneView.cpp
    StackItem.cpp
	TimelineItem.cpp
    TimelineScene.cpp
    TrackItem.cpp
    TransitionItem.cpp
    Util.cpp)

if(TLRENDER_BUILD_QT6)
	qt6_add_resources(SOURCE tlViewApp.qrc)
elseif(TLRENDER_BUILD_QT5)
	qt5_add_resources(SOURCE tlViewApp.qrc)
endif()

add_library(tlViewApp ${HEADERS} ${SOURCE})
target_link_libraries(tlViewApp tlQtWidget tlApp)
set_target_properties(tlViewApp PROPERTIES FOLDER lib)
set_target_properties(tlViewApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlViewApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlViewApp)
