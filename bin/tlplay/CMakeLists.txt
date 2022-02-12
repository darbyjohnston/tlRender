set(HEADERS)
set(SOURCE main.cpp)

add_executable(tlplay ${SOURCE} ${HEADERS})
target_link_libraries(tlplay tlPlay)
set_target_properties(tlplay PROPERTIES FOLDER bin)

install(
    TARGETS tlplay
    RUNTIME DESTINATION bin)
