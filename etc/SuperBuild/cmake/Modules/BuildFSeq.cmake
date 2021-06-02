include(ExternalProject)

set(FSeq_ARGS ${TLR_EXTERNAL_ARGS})
if(BUILD_SHARED_LIBS)
    set(FSeq_ARGS ${FSeq_ARGS} -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE)
endif()
if(CMAKE_CXX_STANDARD)
    set(FSeq_ARGS ${FSeq_ARGS} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD})
endif()

ExternalProject_Add(
	FSeq
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/FSeq
	GIT_REPOSITORY https://github.com/darbyjohnston/FSeq.git
	CMAKE_ARGS ${FSeq_ARGS})
