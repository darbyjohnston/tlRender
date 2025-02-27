add_executable(player App.h MainWindow.h App.cpp MainWindow.cpp main.cpp)
target_link_libraries(player tlTimelineUI)
set_target_properties(player PROPERTIES FOLDER examples)
