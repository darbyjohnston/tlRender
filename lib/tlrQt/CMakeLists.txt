set(HEADERS
    TimeSpinBox.h
    TimelineObject.h
    TimelineViewport.h
    TimelineWidget.h)
set(SOURCE
    TimeSpinBox.cpp
    TimelineObject.cpp
    TimelineViewport.cpp
    TimelineWidget.cpp)

add_library(tlrQt ${HEADERS} ${SOURCE})
target_link_libraries(tlrQt tlrCore Qt5::Widgets OpenGL::GL)
set_target_properties(tlrQt PROPERTIES FOLDER lib)
