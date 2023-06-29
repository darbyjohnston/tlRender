add_executable(tlplay-qt main.cpp)
target_link_libraries(tlplay-qt tlPlayQtApp)
set_target_properties(tlplay-qt PROPERTIES FOLDER bin)

install(
    TARGETS tlplay-qt
    RUNTIME DESTINATION bin)
