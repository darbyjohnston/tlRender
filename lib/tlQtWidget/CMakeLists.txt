set(HEADERS
    ContainerWidget.h
    Init.h
    SpeedLabel.h
    TimeLabel.h
    TimeSpinBox.h
    TimelineWidget.h
    Util.h
    Viewport.h)
set(PRIVATE_HEADERS)

set(SOURCE
    ContainerWidget.cpp
    Init.cpp
    SpeedLabel.cpp
    TimeLabel.cpp
    TimeSpinBox.cpp
    TimelineWidget.cpp
    Util.cpp
    Viewport.cpp)

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

install(
    TARGETS tlQtWidget
    EXPORT tlQtWidgetTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlQtWidget)
install(
    EXPORT tlQtWidgetTargets
    FILE tlQtWidgetTargets.cmake
    DESTINATION "lib/cmake/tlRender"
    NAMESPACE tlRender::)