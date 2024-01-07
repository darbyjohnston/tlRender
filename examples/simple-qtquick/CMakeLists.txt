set(HEADERS
    App.h)

set(SOURCE
    App.cpp
    main.cpp)

if(TLRENDER_QT6)
    qt6_add_resources(SOURCE simple-qtquick.qrc)
elseif(TLRENDER_QT5)
    qt5_add_resources(SOURCE simple-qtquick.qrc)
endif()

add_executable(simple-qtquick ${SOURCE} ${HEADERS})
target_link_libraries(simple-qtquick tlQtQuick tlBaseApp)
set_target_properties(simple-qtquick PROPERTIES FOLDER examples)
