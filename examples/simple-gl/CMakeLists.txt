add_executable(simple-gl simple-gl.h simple-gl.cpp)
target_link_libraries(simple-gl tlGLApp tlTimelineUI)
set_target_properties(simple-gl PROPERTIES FOLDER examples)
