set(HEADERS Resource.h)

set(SOURCE
    Resource.cpp
    ../../etc/Icons/Color.cpp
    ../../etc/Icons/CompareA.cpp
    ../../etc/Icons/CompareB.cpp
    ../../etc/Icons/CompareDifference.cpp
    ../../etc/Icons/CompareHorizontal.cpp
    ../../etc/Icons/CompareOverlay.cpp
    ../../etc/Icons/Compare.cpp
    ../../etc/Icons/CompareTile.cpp
    ../../etc/Icons/CompareVertical.cpp
    ../../etc/Icons/CompareWipe.cpp
    ../../etc/Icons/Devices.cpp
    ../../etc/Icons/FileOpenSeparateAudio.cpp
    ../../etc/Icons/Files.cpp
    ../../etc/Icons/Hidden.cpp
    ../../etc/Icons/Info.cpp
    ../../etc/Icons/Messages.cpp
    ../../etc/Icons/tlRender.cpp
    ../../etc/Icons/View.cpp
    ../../etc/Icons/Visible.cpp
    ../../etc/Icons/WindowFullScreen.cpp
    ../../etc/Icons/WindowSecondary.cpp)

add_library(tlResource ${HEADERS} ${SOURCE})
target_link_libraries(tlResource)
set_target_properties(tlResource PROPERTIES FOLDER lib)
set_target_properties(tlResource PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS tlResource
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlResource)
