add_executable(
    test-patterns
    App.h
    TestPatterns.h
    App.cpp
    TestPatterns.cpp
    main.cpp)
target_link_libraries(test-patterns tlTimelineGL)
set_target_properties(test-patterns PROPERTIES FOLDER examples)
