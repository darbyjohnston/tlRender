set(HEADERS
	App.h
	FloatSlider.h
	IntSlider.h
	MainWindow.h)
set(SOURCE
	App.cpp
	FloatSlider.cpp
	IntSlider.cpp
	MainWindow.cpp
	main.cpp)

add_executable(widgets-qwidget ${HEADERS} ${SOURCE})
target_link_libraries(widgets-qwidget tlQWidget)
set_target_properties(widgets-qwidget PROPERTIES FOLDER examples)
