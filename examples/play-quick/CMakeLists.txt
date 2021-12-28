set(HEADERS
    App.h)
set(SOURCE
    App.cpp
    main.cpp)
qt5_add_resources(SOURCE play-quick.qrc)

add_executable(play-quick ${SOURCE} ${HEADERS})
target_link_libraries(play-quick tlrQuick tlrApp)
set_target_properties(play-quick PROPERTIES FOLDER examples)
