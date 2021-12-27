add_executable(swrender swrender.h swrender.cpp)
target_link_libraries(swrender tlrGL tlrApp GLFW)
set_target_properties(swrender PROPERTIES FOLDER examples)
