set(HEADERS
    App.h)
set(SOURCE
    App.cpp
    main.cpp)
qt5_add_resources(SOURCE tlrplay-quick.qrc)

add_executable(tlrplay-quick ${SOURCE} ${HEADERS})
target_link_libraries(tlrplay-quick tlrQuick tlrApp)
set_target_properties(tlrplay-quick PROPERTIES FOLDER bin)

install(
    TARGETS tlrplay-quick
    RUNTIME DESTINATION bin)
