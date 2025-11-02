set(HEADERS
    IO.h
    IOInline.h
    Init.h
    Plugin.h
    Read.h
    SequenceIO.h
    System.h
    SystemInline.h
    Write.h)
set(HEADERS_PRIVATE
    SequenceIOReadPrivate.h)

set(SOURCE
    IO.cpp
    Init.cpp
    Plugin.cpp
    Read.cpp
    SequenceIO.cpp
    SequenceIORead.cpp
    SequenceIOWrite.cpp
    System.cpp
    Write.cpp)

set(LIBRARIES)
set(LIBRARIES_PRIVATE)
if(TLRENDER_EXR)
    list(APPEND HEADERS OpenEXR.h)
    list(APPEND HEADERS_PRIVATE OpenEXRPrivate.h)
    list(APPEND SOURCE OpenEXR.cpp OpenEXRRead.cpp OpenEXRWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE OpenEXR::OpenEXR)
endif()
if(TLRENDER_FFMPEG)
    list(APPEND HEADERS FFmpeg.h)
    list(APPEND HEADERS_PRIVATE FFmpegPrivate.h FFmpegReadPrivate.h)
    list(APPEND SOURCE FFmpeg.cpp FFmpegRead.cpp FFmpegReadAudio.cpp
        FFmpegReadVideo.cpp FFmpegWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE FFmpeg::FFmpeg)
endif()
if(TLRENDER_OIIO)
    list(APPEND HEADERS OIIO.h)
    list(APPEND SOURCE OIIO.cpp OIIORead.cpp OIIOWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE OpenImageIO::OpenImageIO)
endif()
if(TLRENDER_USD)
    list(APPEND HEADERS USD.h)
    list(APPEND HEADERS_PRIVATE USDPrivate.h)
    list(APPEND SOURCE USD.cpp USDRead.cpp USDRender.cpp)
    list(APPEND LIBRARIES_PRIVATE tf usd usdGeom usdImagingGL usdAppUtils hd)
endif()
if(TLRENDER_WMF)
    list(APPEND HEADERS WMF.h)
    list(APPEND SOURCE WMF.cpp WMFRead.cpp)
    list(APPEND LIBRARIES_PRIVATE mf mfplat mfreadwrite mfuuid propsys wmcodecdspuuid)
endif()
list(APPEND LIBRARIES tlCore ftk::ftkGL)

add_library(tlIO ${HEADERS})
target_sources(tlIO PRIVATE ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlIO PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlIO PROPERTIES FOLDER lib)
set_target_properties(tlIO PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS tlIO
    EXPORT tlIOTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlIO)
install(
    EXPORT tlIOTargets
    FILE tlIOTargets.cmake
    DESTINATION "lib/cmake/tlRender"
    NAMESPACE tlRender::)
