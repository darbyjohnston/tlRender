add_executable(render App.h App.cpp main.cpp)
target_link_libraries(render tlTimelineGL)
set_target_properties(render PROPERTIES FOLDER examples)
