find_package(libsamplerate REQUIRED)

set(source main.cpp)

add_executable(libsamplerateTest ${header} ${source})
target_link_libraries(libsamplerateTest libsamplerate)

add_test(libsamplerateTest libsamplerateTest)
