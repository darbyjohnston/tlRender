set(HEADERS
    ITest.h
    TimelineTest.h)
set(SOURCE
    ITest.cpp
    TimelineTest.cpp
    main.cpp)
if(FFmpeg_FOUND)
    set(HEADERS ${HEADERS} FFmpegTest.h)
    set(SOURCE ${SOURCE} FFmpegTest.cpp)
endif()

add_executable(tlrtest ${SOURCE} ${HEADERS})
target_link_libraries(tlrtest tlrCore)

install(
    TARGETS tlrtest
    RUNTIME DESTINATION bin)
set_target_properties(tlrtest PROPERTIES FOLDER tests)

add_test(tlrtest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tlrtest${CMAKE_EXECUTABLE_SUFFIX})
