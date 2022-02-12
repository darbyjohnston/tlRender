set(HEADERS
    AVIO.h
    AVIOInline.h
    AVIOSystem.h
    AVIOSystemInline.h
    Assert.h
    Audio.h
    AudioInline.h
    AudioSystem.h
    BBox.h
    BBoxInline.h
    Cineon.h
    Color.h
    ColorInline.h
    Context.h
    ContextInline.h
    DPX.h
    Error.h
    File.h
    FileIO.h
    FontSystem.h
    ICoreSystem.h
    ICoreSystemInline.h
	IRender.h
	IRenderInline.h
    ISystem.h
    Image.h
    ImageInline.h
    LRUCache.h
    LRUCacheInline.h
    ListObserver.h
    ListObserverInline.h
    LogSystem.h
    MapObserver.h
    MapObserverInline.h
    Math.h
    MathInline.h
    Matrix.h
    MatrixInline.h	
    Memory.h
    MemoryInline.h
	Mesh.h
	MeshInline.h
	OCIO.h
	OCIOInline.h
	OS.h
    Observer.h
    PPM.h
    Path.h
    PathInline.h
    Range.h
    RangeInline.h
    SequenceIO.h
	SoftwareRender.h
    String.h
    StringFormat.h
    StringFormatInline.h
    Time.h
    Timeline.h
    TimelineInline.h
    TimelinePlayer.h
    TimelinePlayerInline.h
	TimelineUtil.h
    Util.h
    ValueObserver.h
    ValueObserverInline.h
	Vector.h
	VectorInline.h)
set(SOURCE
    AVIO.cpp
    AVIOSystem.cpp
    Assert.cpp
    Audio.cpp
    AudioSystem.cpp
	BBox.cpp
    CineonRead.cpp
    CineonWrite.cpp
    Cineon.cpp
    Context.cpp
    DPXRead.cpp
    DPXWrite.cpp
    DPX.cpp
    Error.cpp
    FileIO.cpp
    FontSystem.cpp
    ICoreSystem.cpp
	IRender.cpp
    ISystem.cpp
    Image.cpp
    LogSystem.cpp
	Matrix.cpp
    Memory.cpp
	Mesh.cpp
	OCIO.cpp
	OS.cpp
    PPM.cpp
    PPMRead.cpp
    PPMWrite.cpp
    Path.cpp
    SequenceIO.cpp
	SoftwareRender.cpp
    String.cpp
    StringFormat.cpp
    Time.cpp
    Timeline.cpp
    TimelinePlayer.cpp
	TimelineUtil.cpp
	Vector.cpp)
if (WIN32)
    set(SOURCE
        ${SOURCE}
        ErrorWin32.cpp
        FileIOWin32.cpp
        FileWin32.cpp
		OSWin32.cpp
        TimeWin32.cpp)
else()
    set(SOURCE
        ${SOURCE}
        FileIOUnix.cpp
        FileUnix.cpp
		OSUnix.cpp
        TimeUnix.cpp)
endif()

set(tlCore_LIBRARIES OTIO OCIO FreeType FSeq GLM RtAudio libsamplerate)
if(TLRENDER_ENABLE_PYTHON)
    list(APPEND tlCore_LIBRARIES Python3::Python)
endif()
if(JPEG_FOUND)
    list(APPEND HEADERS JPEG.h)
    list(APPEND SOURCE JPEG.cpp JPEGRead.cpp JPEGWrite.cpp)
    list(APPEND tlCore_LIBRARIES JPEG)
endif()
if(TIFF_FOUND)
    list(APPEND HEADERS TIFF.h)
    list(APPEND SOURCE TIFF.cpp TIFFRead.cpp TIFFWrite.cpp)
    list(APPEND tlCore_LIBRARIES TIFF)
endif()
if(PNG_FOUND)
    list(APPEND HEADERS PNG.h)
    list(APPEND SOURCE PNG.cpp PNGRead.cpp PNGWrite.cpp)
    list(APPEND tlCore_LIBRARIES PNG)
endif()
if(OpenEXR_FOUND)
    list(APPEND HEADERS OpenEXR.h)
    list(APPEND SOURCE OpenEXR.cpp OpenEXRRead.cpp OpenEXRWrite.cpp)
    list(APPEND tlCore_LIBRARIES OpenEXR)
endif()
if(FFmpeg_FOUND)
    list(APPEND HEADERS FFmpeg.h)
    list(APPEND SOURCE FFmpeg.cpp FFmpegRead.cpp FFmpegWrite.cpp)
    list(APPEND tlCore_LIBRARIES FFmpeg)
endif()
list(APPEND tlCore_LIBRARIES Imath Threads::Threads)

add_library(tlCore ${HEADERS} ${SOURCE})
target_link_libraries(tlCore ${tlCore_LIBRARIES})
set_target_properties(tlCore PROPERTIES FOLDER lib)
set_target_properties(tlCore PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlCore
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlRender/tlCore)
