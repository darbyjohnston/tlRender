set(HEADERS)

set(SOURCE
    main.cpp)

add_executable(usdrecord-glfw ${SOURCE} ${HEADERS})
target_link_libraries(usdrecord-glfw GLFW tf usd usdGeom usdImagingGL usdAppUtils hd)
set_target_properties(usdrecord-glfw PROPERTIES FOLDER examples)

