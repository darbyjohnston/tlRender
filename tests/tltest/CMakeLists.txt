set(HEADERS)
set(SOURCE
    main.cpp)

add_executable(tltest ${SOURCE} ${HEADERS})
set(LIBRARIES
    tlAppTest
    tlCoreTest
    tlIOTest
    tlTimelineTest)
if(TLRENDER_BUILD_GL)
    list(APPEND LIBRARIES tlRenderGLTest)
endif()
if(TLRENDER_BUILD_QT6 OR TLRENDER_BUILD_QT5)
    list(APPEND LIBRARIES tlQtTest)
endif()
target_link_libraries(tltest ${LIBRARIES})
set_target_properties(tltest PROPERTIES FOLDER tests)

add_test(tltest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tltest${CMAKE_EXECUTABLE_SUFFIX})
