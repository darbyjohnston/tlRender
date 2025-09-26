set(HEADERS
    Cineon.h
    DPX.h
    IO.h
    IOInline.h
    Init.h
    PPM.h
    Plugin.h
    Read.h
    SGI.h
    SequenceIO.h
    System.h
    SystemInline.h
    Write.h)
set(HEADERS_PRIVATE
    SequenceIOReadPrivate.h)

set(SOURCE
    CineonRead.cpp
    CineonWrite.cpp
    Cineon.cpp
    DPXRead.cpp
    DPXWrite.cpp
    DPX.cpp
    IO.cpp
    Init.cpp
    PPM.cpp
    PPMRead.cpp
    PPMWrite.cpp
    Plugin.cpp
    Read.cpp
    SGI.cpp
    SGIRead.cpp
    SGIWrite.cpp
    SequenceIO.cpp
    SequenceIORead.cpp
    SequenceIOWrite.cpp
    System.cpp
    Write.cpp)

set(LIBRARIES)
set(LIBRARIES_PRIVATE)
if(TLRENDER_JPEG)
    list(APPEND HEADERS JPEG.h)
    list(APPEND HEADERS_PRIVATE JPEGPrivate.h)
    list(APPEND SOURCE JPEG.cpp JPEGRead.cpp JPEGWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE libjpeg-turbo::turbojpeg-static)
endif()
if(TLRENDER_TIFF)
    list(APPEND HEADERS TIFF.h)
    list(APPEND SOURCE TIFF.cpp TIFFRead.cpp TIFFWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE TIFF::TIFF)
endif()
if(TLRENDER_STB)
    list(APPEND HEADERS STB.h)
    list(APPEND SOURCE STB.cpp STBRead.cpp STBWrite.cpp)
endif()
if(TLRENDER_PNG)
    list(APPEND HEADERS PNG.h)
    list(APPEND HEADERS_PRIVATE PNGPrivate.h)
    list(APPEND SOURCE PNG.cpp PNGRead.cpp PNGWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE PNG::PNG)
endif()
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
list(APPEND LIBRARIES tlCore ftk::feather-tk-gl)

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
