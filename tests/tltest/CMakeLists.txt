set(HEADERS)

set(SOURCE
    main.cpp)

set(LIBRARIES
    tlAppTest
    tlCoreTest
    tlIOTest
    tlTimelineTest)
if(TLRENDER_GL)
    list(APPEND LIBRARIES tlGLTest)
endif()
if(TLRENDER_QT6 OR TLRENDER_QT5)
    list(APPEND LIBRARIES tlQtTest)
endif()

add_executable(tltest ${SOURCE} ${HEADERS})
target_link_libraries(tltest ${LIBRARIES})
set_target_properties(tltest PROPERTIES FOLDER tests)

add_test(tltest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tltest${CMAKE_EXECUTABLE_SUFFIX})
