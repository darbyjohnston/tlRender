set(HEADERS
    Bindings.h)
set(HEADERS_PRIVATE)
set(SOURCE
    Bindings.cpp
    OffscreenRender.cpp)

# Static. The bindings are linked by the tlrender module and nothing else,
# so there is no API here for anything to import -- the module absorbs them.
add_library(tlGLPy STATIC ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
# Hidden, as the module is: the binding classes hold nanobind's types,
# which are hidden, and GCC warns about a class more visible than its
# members. Nothing here is exported anyway.
set_target_properties(tlGLPy PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)

target_include_directories(tlGLPy
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:include>)
target_include_directories(tlGLPy
    PRIVATE
        ${NB_DIR}/include
        ${NB_DIR}/ext/robin_map/include)

target_link_libraries(tlGLPy PUBLIC tlGL PRIVATE ${TLRENDER_NANOBIND_LIBRARY})

set_target_properties(tlGLPy PROPERTIES FOLDER lib)
set_target_properties(tlGLPy PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlGLPy
    ARCHIVE DESTINATION lib COMPONENT dev ${TLRENDER_PYTHON_ARCHIVE_INSTALL}
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/GLPy COMPONENT dev)
