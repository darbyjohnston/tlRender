add_subdirectory(tlbake)
if(TLRENDER_BUILD_QT6 OR TLRENDER_BUILD_QT5)
    add_subdirectory(tlplay)
endif()
