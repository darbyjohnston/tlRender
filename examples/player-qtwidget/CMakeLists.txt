add_executable(player-qtwidget MainWindow.h MainWindow.cpp main.cpp)
target_link_libraries(player-qtwidget tlQtWidget)
set_target_properties(player-qtwidget PROPERTIES FOLDER examples)
