add_executable(tlplay main.cpp)
target_link_libraries(tlplay tlQtPlayApp)
set_target_properties(tlplay PROPERTIES FOLDER bin)

install(
    TARGETS tlplay
    RUNTIME DESTINATION bin)
