find_package(freetype REQUIRED)

set(source main.cpp)

add_executable(freetypeTest ${header} ${source})
target_link_libraries(freetypeTest Freetype::Freetype ZLIB)

add_test(freetypeTest freetypeTest)
