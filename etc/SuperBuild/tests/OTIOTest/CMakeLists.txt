find_package(OpenTimelineIO REQUIRED)

set(source main.cpp)

add_executable(OTIOTest ${header} ${source})
target_link_libraries(OTIOTest OTIO::opentimelineio)

add_test(OTIOTest OTIOTest)
