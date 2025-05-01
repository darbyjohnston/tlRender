find_package(PkgConfig REQUIRED)
pkg_check_modules(
    FFmpeg REQUIRED IMPORTED_TARGET
    libavdevice
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil)

set(source main.cpp)

add_executable(FFmpegTest ${header} ${source})
target_link_libraries(FFmpegTest PkgConfig::FFmpeg)

add_test(FFmpegTest FFmpegTest)
