set(HEADERS
	BellowsPrivate.h
	BellowsWidget.h
	FileWidget.h
    FilmstripWidget.h
	FloatSlider.h
	IntSlider.h
	SearchWidget.h
	Separator.h
	Spacer.h
    SpeedLabel.h
	Style.h
    TimeLabel.h
    TimeSpinBox.h
    TimelineSlider.h
    TimelineViewport.h
    Util.h)

set(SOURCE
	BellowsPrivate.cpp
	BellowsWidget.cpp
	FileWidget.cpp
    FilmstripWidget.cpp
	FloatSlider.cpp
	IntSlider.cpp
	SearchWidget.cpp
	Separator.cpp
	Spacer.cpp
    SpeedLabel.cpp
	Style.cpp
    TimeLabel.cpp
    TimeSpinBox.cpp
    TimelineSlider.cpp
    TimelineViewport.cpp
    Util.cpp)

if(TLRENDER_BUILD_QT6)
	qt6_add_resources(SOURCE tlQtWidget.qrc)
elseif(TLRENDER_BUILD_QT5)
	qt5_add_resources(SOURCE tlQtWidget.qrc)
endif()

set(LIBRARIES)
if(TLRENDER_BUILD_QT6)
	list(APPEND LIBRARIES Qt6::Widgets Qt6::OpenGLWidgets Qt6::Svg)
elseif(TLRENDER_BUILD_QT5)
	list(APPEND LIBRARIES Qt5::Widgets Qt5::Svg)
endif()
list(APPEND LIBRARIES tlQt)

add_library(tlQtWidget ${HEADERS} ${SOURCE})
target_link_libraries(tlQtWidget ${LIBRARIES})
set_target_properties(tlQtWidget PROPERTIES FOLDER lib)
set_target_properties(tlQtWidget PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlQtWidget
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlQtWidget)
