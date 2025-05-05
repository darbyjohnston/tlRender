if(WIN32)
    set(FFmpeg_FOUND TRUE)
    add_library(FFmpeg::avdevice STATIC IMPORTED)
    set_property(TARGET FFmpeg::avdevice PROPERTY
         IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/avdevice.lib")
    add_library(FFmpeg::avformat STATIC IMPORTED)
    set_property(TARGET FFmpeg::avformat PROPERTY
         IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/avformat.lib")
    add_library(FFmpeg::avcodec STATIC IMPORTED)
    set_property(TARGET FFmpeg::avcodec PROPERTY
         IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/avcodec.lib")
    add_library(FFmpeg::swresample STATIC IMPORTED)
    set_property(TARGET FFmpeg::swresample PROPERTY
         IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/swresample.lib")
    add_library(FFmpeg::swscale STATIC IMPORTED)
    set_property(TARGET FFmpeg::swscale PROPERTY
         IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/swscale.lib")
    add_library(FFmpeg::avutil STATIC IMPORTED)
    set_property(TARGET FFmpeg::avutil PROPERTY
         IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/avutil.lib")
    add_library(FFmpeg::FFmpeg INTERFACE IMPORTED)
    set_property(TARGET FFmpeg::FFmpeg PROPERTY
        INTERFACE_LINK_LIBRARIES
        FFmpeg::avdevice
        FFmpeg::avformat
        FFmpeg::avcodec
        FFmpeg::swresample
        FFmpeg::swscale
        FFmpeg::avutil)
    target_include_directories(FFmpeg::FFmpeg INTERFACE "${CMAKE_INSTALL_PREFIX}/include")
elseif(APPLE)
    set(FFmpeg_FOUND TRUE)
    add_library(FFmpeg::avdevice STATIC IMPORTED)
    set_property(TARGET FFmpeg::avdevice PROPERTY
         IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/libavdevice.dylib")
    add_library(FFmpeg::avformat STATIC IMPORTED)
    set_property(TARGET FFmpeg::avformat PROPERTY
         IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/libavformat.dylib")
    add_library(FFmpeg::avcodec STATIC IMPORTED)
    set_property(TARGET FFmpeg::avcodec PROPERTY
         IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/libavcodec.dylib")
    add_library(FFmpeg::swresample STATIC IMPORTED)
    set_property(TARGET FFmpeg::swresample PROPERTY
         IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/libswresample.dylib")
    add_library(FFmpeg::swscale STATIC IMPORTED)
    set_property(TARGET FFmpeg::swscale PROPERTY
         IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/libswscale.dylib")
    add_library(FFmpeg::avutil STATIC IMPORTED)
    set_property(TARGET FFmpeg::avutil PROPERTY
         IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/lib/libavutil.dylib")
    add_library(FFmpeg::FFmpeg INTERFACE IMPORTED)
    set_property(TARGET FFmpeg::FFmpeg PROPERTY
        INTERFACE_LINK_LIBRARIES
        FFmpeg::avdevice
        FFmpeg::avformat
        FFmpeg::avcodec
        FFmpeg::swresample
        FFmpeg::swscale
        FFmpeg::avutil)
    target_include_directories(FFmpeg::FFmpeg INTERFACE "${CMAKE_INSTALL_PREFIX}/include")
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(
        FFmpeg REQUIRED IMPORTED_TARGET
        libavdevice
        libavformat
        libavcodec
        libswresample
        libswscale
        libavutil)
    add_library(FFmpeg::FFmpeg ALIAS PkgConfig::FFmpeg)
endif()

set(source main.cpp)

add_executable(FFmpegTest ${header} ${source})
target_link_libraries(FFmpegTest FFmpeg::FFmpeg)

add_test(FFmpegTest FFmpegTest)
