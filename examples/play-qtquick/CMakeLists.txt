set(HEADERS
    App.h)

set(SOURCE
    App.cpp
    main.cpp)

if(TLRENDER_QT6)
    qt6_add_resources(SOURCE play-qtquick.qrc)
elseif(TLRENDER_QT5)
    qt5_add_resources(SOURCE play-qtquick.qrc)
endif()

add_executable(play-qtquick ${SOURCE} ${HEADERS})
target_link_libraries(play-qtquick tlQtQuick tlApp)
set_target_properties(play-qtquick PROPERTIES FOLDER examples)
