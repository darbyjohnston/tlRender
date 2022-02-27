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

add_executable(widgets-qtwidget ${HEADERS} ${SOURCE})
target_link_libraries(widgets-qtwidget tlQtWidget)
set_target_properties(widgets-qtwidget PROPERTIES FOLDER examples)
