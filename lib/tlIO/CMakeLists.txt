set(HEADERS
    Cineon.h
    DPX.h
    IO.h
    IOInline.h
    IOSystem.h
    IOSystemInline.h
    PPM.h
    SequenceIO.h
    SGI.h
    Util.h)
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
    IOSystem.cpp
    PPM.cpp
    PPMRead.cpp
    PPMWrite.cpp
    SequenceIORead.cpp
    SequenceIOWrite.cpp
    SGI.cpp
    SGIRead.cpp
    SGIWrite.cpp
    Util.cpp)

set(LIBRARIES tlCore)
set(LIBRARIES_PRIVATE)
if(TLRENDER_JPEG)
    list(APPEND HEADERS_PRIVATE JPEG.h)
    list(APPEND SOURCE JPEG.cpp JPEGRead.cpp JPEGWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE JPEG)
endif()
if(TLRENDER_TIFF)
    list(APPEND HEADERS_PRIVATE TIFF.h)
    list(APPEND SOURCE TIFF.cpp TIFFRead.cpp TIFFWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE TIFF)
endif()
if(TLRENDER_PNG)
    list(APPEND HEADERS_PRIVATE PNG.h)
    list(APPEND SOURCE PNG.cpp PNGRead.cpp PNGWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE PNG)
endif()
if(TLRENDER_EXR)
    # \todo Make this library private.
    list(APPEND HEADERS OpenEXR.h)
    list(APPEND SOURCE OpenEXR.cpp OpenEXRRead.cpp OpenEXRWrite.cpp)
    list(APPEND LIBRARIES OpenEXR::OpenEXR)
endif()
if(TLRENDER_FFMPEG)
    list(APPEND HEADERS_PRIVATE FFmpeg.h FFmpegReadPrivate.h)
    list(APPEND SOURCE FFmpeg.cpp FFmpegRead.cpp FFmpegReadAudio.cpp
        FFmpegReadVideo.cpp FFmpegWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE FFmpeg)
endif()

add_library(tlIO ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlIO PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlIO PROPERTIES FOLDER lib)
set_target_properties(tlIO PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlIO
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlIO)
