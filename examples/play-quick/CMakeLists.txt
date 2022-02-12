set(HEADERS
    App.h)
set(SOURCE
    App.cpp
    main.cpp)
if(TLRENDER_BUILD_QT6)
	qt6_add_resources(SOURCE play-quick.qrc)
elseif(TLRENDER_BUILD_QT5)
	qt5_add_resources(SOURCE play-quick.qrc)
endif()

add_executable(play-quick ${SOURCE} ${HEADERS})
target_link_libraries(play-quick tlQuick tlApp)
set_target_properties(play-quick PROPERTIES FOLDER examples)
