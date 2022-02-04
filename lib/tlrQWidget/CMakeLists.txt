set(HEADERS
	BellowsWidget.h
    FilmstripWidget.h
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
	BellowsWidget.cpp
    FilmstripWidget.cpp
	Separator.cpp
    SpeedLabel.cpp
    TimeLabel.cpp
    TimeSpinBox.cpp
    TimelineControls.cpp
    TimelineSlider.cpp
    TimelineViewport.cpp
    TimelineWidget.cpp
    Util.cpp)
if(TLR_BUILD_QT6)
	qt6_add_resources(SOURCE tlrQWidget.qrc)
elseif(TLR_BUILD_QT5)
	qt5_add_resources(SOURCE tlrQWidget.qrc)
endif()

add_library(tlrQWidget ${HEADERS} ${SOURCE})
if(TLR_BUILD_QT6)
	target_link_libraries(tlrQWidget Qt6::Widgets Qt6::OpenGLWidgets Qt6::Svg tlrQt)
elseif(TLR_BUILD_QT5)
	target_link_libraries(tlrQWidget Qt5::Widgets Qt5::Svg tlrQt)
endif()
set_target_properties(tlrQWidget PROPERTIES FOLDER lib)
set_target_properties(tlrQWidget PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlrQWidget
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlrQWidget)
