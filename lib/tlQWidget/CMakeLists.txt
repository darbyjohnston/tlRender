set(HEADERS
	BellowsPrivate.h
	BellowsWidget.h
    FilmstripWidget.h
	RadioButtonGroup.h
	Separator.h
    SpeedLabel.h
    TimeLabel.h
    TimeSpinBox.h
    TimelineControls.h
    TimelineSlider.h
    TimelineViewport.h
    TimelineWidget.h
    Util.h)
set(SOURCE
	BellowsPrivate.cpp
	BellowsWidget.cpp
    FilmstripWidget.cpp
	RadioButtonGroup.cpp
	Separator.cpp
    SpeedLabel.cpp
    TimeLabel.cpp
    TimeSpinBox.cpp
    TimelineControls.cpp
    TimelineSlider.cpp
    TimelineViewport.cpp
    TimelineWidget.cpp
    Util.cpp)
if(TLRENDER_BUILD_QT6)
	qt6_add_resources(SOURCE tlQWidget.qrc)
elseif(TLRENDER_BUILD_QT5)
	qt5_add_resources(SOURCE tlQWidget.qrc)
endif()

add_library(tlQWidget ${HEADERS} ${SOURCE})
if(TLRENDER_BUILD_QT6)
	target_link_libraries(tlQWidget Qt6::Widgets Qt6::OpenGLWidgets Qt6::Svg tlQt)
elseif(TLRENDER_BUILD_QT5)
	target_link_libraries(tlQWidget Qt5::Widgets Qt5::Svg tlQt)
endif()
set_target_properties(tlQWidget PROPERTIES FOLDER lib)
set_target_properties(tlQWidget PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlQWidget
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlQWidget)
