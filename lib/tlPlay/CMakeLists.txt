set(HEADERS
    AudioModel.h
    ColorConfigModel.h
    ColorModel.h
    FilesModel.h
    Util.h
    ViewportModel.h)

set(SOURCE
    AudioModel.cpp
    ColorConfigModel.cpp
    ColorModel.cpp
    FilesModel.cpp
    Util.cpp
    ViewportModel.cpp)

add_library(tlPlay ${HEADERS} ${SOURCE})
target_link_libraries(tlPlay tlTimeline)
set_target_properties(tlPlay PROPERTIES FOLDER lib)
set_target_properties(tlPlay PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlPlay
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlPlay)
