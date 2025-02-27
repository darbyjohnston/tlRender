add_executable(panorama-qtwidget PanoramaViewport.h PanoramaViewport.cpp main.cpp)
target_link_libraries(panorama-qtwidget tlQtWidget)
set_target_properties(panorama-qtwidget PROPERTIES FOLDER examples)
