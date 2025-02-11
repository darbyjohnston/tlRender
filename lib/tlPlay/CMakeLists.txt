set(HEADERS
    App.h
    AudioModel.h
    ColorModel.h
    FilesModel.h
    Info.h
    Init.h
    OCIOModel.h
    RecentFilesModel.h
    RenderModel.h
    SettingsModel.h
    TimeUnitsModel.h
    Util.h
    Viewport.h
    ViewportModel.h)
set(PRIVATE_HEADERS
    ViewportPrivate.h)
if(TLRENDER_BMD)
    list(APPEND HEADERS BMDDevicesModel.h)
endif()

set(SOURCE
    App.cpp
    AudioModel.cpp
    ColorModel.cpp
    FilesModel.cpp
    Info.cpp
    Init.cpp
    OCIOModel.cpp
    RecentFilesModel.cpp
    RenderModel.cpp
    SettingsModel.cpp
    TimeUnitsModel.cpp
    Util.cpp
    Viewport.cpp
    ViewportColorWidget.cpp
    ViewportModel.cpp)
if(TLRENDER_BMD)
    list(APPEND SOURCE BMDDevicesModel.cpp)
endif()

add_library(tlPlay ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlPlay tlTimelineUI tlDevice)
set_target_properties(tlPlay PROPERTIES FOLDER lib)
set_target_properties(tlPlay PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlPlay
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlPlay)
