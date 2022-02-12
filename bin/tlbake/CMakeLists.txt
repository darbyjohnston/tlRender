set(HEADERS
    App.h)
set(SOURCE
    App.cpp
    main.cpp)

add_executable(tlbake ${SOURCE} ${HEADERS})
target_link_libraries(tlbake tlApp)
set_target_properties(tlbake PROPERTIES FOLDER bin)

install(
    TARGETS tlbake
    RUNTIME DESTINATION bin)
