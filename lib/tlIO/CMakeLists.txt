set(HEADERS
    Cineon.h
    DPX.h
    IO.h
    IOInline.h
    IOSystem.h
    IOSystemInline.h
    PPM.h
    SequenceIO.h
	SGI.h)
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
    SequenceIO.cpp
	SGI.cpp
	SGIRead.cpp
	SGIWrite.cpp)

set(tlIO_LIBRARIES tlCore)
if(JPEG_FOUND)
    list(APPEND HEADERS JPEG.h)
    list(APPEND SOURCE JPEG.cpp JPEGRead.cpp JPEGWrite.cpp)
    list(APPEND tlIO_LIBRARIES JPEG)
endif()
if(TIFF_FOUND)
    list(APPEND HEADERS TIFF.h)
    list(APPEND SOURCE TIFF.cpp TIFFRead.cpp TIFFWrite.cpp)
    list(APPEND tlIO_LIBRARIES TIFF)
endif()
if(PNG_FOUND)
    list(APPEND HEADERS PNG.h)
    list(APPEND SOURCE PNG.cpp PNGRead.cpp PNGWrite.cpp)
    list(APPEND tlIO_LIBRARIES PNG)
endif()
if(OpenEXR_FOUND)
    list(APPEND HEADERS OpenEXR.h)
    list(APPEND SOURCE OpenEXR.cpp OpenEXRRead.cpp OpenEXRWrite.cpp)
    list(APPEND tlIO_LIBRARIES OpenEXR)
endif()
if(FFmpeg_FOUND)
    list(APPEND HEADERS FFmpeg.h)
    list(APPEND SOURCE FFmpeg.cpp FFmpegRead.cpp FFmpegWrite.cpp)
    list(APPEND tlIO_LIBRARIES FFmpeg)
endif()

add_library(tlIO ${HEADERS} ${SOURCE})
target_link_libraries(tlIO ${tlIO_LIBRARIES})
set_target_properties(tlIO PROPERTIES FOLDER lib)
set_target_properties(tlIO PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlIO
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlRender/tlIO)
