set(HEADERS
    App.h
    AudioModel.h
    ColorModel.h
    FilesModel.h
    Info.h
    Init.h
    OCIOModel.h
    Settings.h
    SettingsInline.h
    Util.h
    ViewportModel.h)

set(SOURCE
    App.cpp
    AudioModel.cpp
    ColorModel.cpp
    FilesModel.cpp
    Info.cpp
    Init.cpp
    OCIOModel.cpp
    Settings.cpp
    Util.cpp
    ViewportModel.cpp)

if(TLRENDER_BMD)
    list(APPEND HEADERS
        BMDDevicesModel.h)
    list(APPEND SOURCE
        BMDDevicesModel.cpp)
endif()

add_library(tlPlay ${HEADERS} ${SOURCE})
target_link_libraries(tlPlay tlDevice)
set_target_properties(tlPlay PROPERTIES FOLDER lib)
set_target_properties(tlPlay PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlPlay
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlPlay)
