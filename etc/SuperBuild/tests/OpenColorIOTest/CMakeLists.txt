find_package(OpenColorIO REQUIRED)

set(source main.cpp)

add_executable(OpenColorIOTest ${header} ${source})
target_link_libraries(OpenColorIOTest OpenColorIO::OpenColorIO)

add_test(OpenColorIOTest OpenColorIOTest)
