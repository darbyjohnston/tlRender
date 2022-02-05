set(HEADERS)
set(SOURCE
    main.cpp)

add_executable(tlrtest ${SOURCE} ${HEADERS})
set(LIBRARIES
    tlrAppTest
    tlrCoreTest)
if(TLR_BUILD_GL)
    list(APPEND LIBRARIES tlrGLTest)
endif()
if(TLR_BUILD_QT6 OR TLR_BUILD_QT5)
    list(APPEND LIBRARIES tlrQtTest)
endif()
target_link_libraries(tlrtest ${LIBRARIES})
set_target_properties(tlrtest PROPERTIES FOLDER tests)

add_test(tlrtest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tlrtest${CMAKE_EXECUTABLE_SUFFIX})
