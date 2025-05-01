set(FFmpeg_LIBS)
if(WIN32)
    set(FFmpeg_LIBS
        ${CMAKE_INSTALL_PREFIX}/lib/avdevice.lib
        ${CMAKE_INSTALL_PREFIX}/lib/avformat.lib
        ${CMAKE_INSTALL_PREFIX}/lib/avcodec.lib
        ${CMAKE_INSTALL_PREFIX}/lib/swresample.lib
        ${CMAKE_INSTALL_PREFIX}/lib/swscale.lib
        ${CMAKE_INSTALL_PREFIX}/lib/avutil.lib)
    include_directories(${CMAKE_INSTALL_PREFIX}/include)
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
    set(FFmpeg_LIBS PkgConfig::FFmpeg)
endif()

set(source main.cpp)

add_executable(FFmpegTest ${header} ${source})
target_link_libraries(FFmpegTest ${FFmpeg_LIBS})

add_test(FFmpegTest FFmpegTest)
