add_subdirectory(Core)
add_subdirectory(GL)
if(TLRENDER_GPU AND TARGET ftk::ftkGPU)
    add_subdirectory(GPU)
    # The C interface draws with the GPU renderer, so it is built with it.
    add_subdirectory(C)
endif()
add_subdirectory(IO)
add_subdirectory(Resource)
add_subdirectory(Timeline)
add_subdirectory(UI)

if(TLRENDER_PROGRAMS)
    add_subdirectory(BakeApp)
endif()

if(TLRENDER_TESTS)
    add_subdirectory(CoreTest)
    add_subdirectory(GLTest)
    add_subdirectory(IOTest)
    add_subdirectory(TimelineTest)
    add_subdirectory(UITest)
endif()

if(TLRENDER_PYTHON)
    # A wheel leaves out the binding archives: nothing links them but the
    # module, which already has. Their headers stay, for the modules built
    # on this one.
    set(TLRENDER_PYTHON_ARCHIVE_INSTALL)
    if(SKBUILD)
        set(TLRENDER_PYTHON_ARCHIVE_INSTALL EXCLUDE_FROM_ALL)
    endif()

    add_subdirectory(CorePy)
    add_subdirectory(IOPy)
    add_subdirectory(TimelinePy)
    add_subdirectory(GLPy)
    add_subdirectory(UIPy)
    add_subdirectory(tlRenderPy)
endif()
