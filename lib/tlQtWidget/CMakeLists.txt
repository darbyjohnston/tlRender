set(HEADERS
    ContainerWidget.h
    FloatEditSlider.h
    Init.h
    IntEditSlider.h
    SpeedLabel.h
    TimeLabel.h
    TimeSpinBox.h
    TimelineWidget.h
    Util.h
    Viewport.h)
set(PRIVATE_HEADERS)

set(SOURCE
    ContainerWidget.cpp
    FloatEditSlider.cpp
    Init.cpp
    IntEditSlider.cpp
    SpeedLabel.cpp
    TimeLabel.cpp
    TimeSpinBox.cpp
    TimelineWidget.cpp
    Util.cpp
    Viewport.cpp)

if(TLRENDER_QT6)
    qt6_add_resources(SOURCE tlQtWidget.qrc)
elseif(TLRENDER_QT5)
    qt5_add_resources(SOURCE tlQtWidget.qrc)
endif()

set(LIBRARIES tlQt tlTimelineUI)
if(TLRENDER_QT6)
    list(APPEND LIBRARIES Qt6::Widgets Qt6::OpenGLWidgets Qt6::Svg)
elseif(TLRENDER_QT5)
    list(APPEND LIBRARIES Qt5::Widgets Qt5::Svg)
endif()

add_library(tlQtWidget ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlQtWidget ${LIBRARIES})
set_target_properties(tlQtWidget PROPERTIES FOLDER lib)
set_target_properties(tlQtWidget PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlQtWidget
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlQtWidget)
