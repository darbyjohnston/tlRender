set(HEADERS)

set(SOURCE
    main.cpp)

add_executable(usdrecord-gl ${SOURCE} ${HEADERS})
target_link_libraries(usdrecord-gl glfw tf usd usdGeom usdImagingGL usdAppUtils hd)
set_target_properties(usdrecord-gl PROPERTIES FOLDER examples)

