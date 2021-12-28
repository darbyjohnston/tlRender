set(HEADERS
    App.h)
set(SOURCE
    App.cpp
    main.cpp)

add_executable(tlrbake ${SOURCE} ${HEADERS})
target_link_libraries(tlrbake tlrApp)
set_target_properties(tlrbake PROPERTIES FOLDER bin)

install(
    TARGETS tlrbake
    RUNTIME DESTINATION bin)
