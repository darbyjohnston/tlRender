include(ExternalProject)

set(FSeq_GIT_REPOSITORY "https://github.com/darbyjohnston/FSeq.git")
set(FSeq_GIT_TAG "f27bb2e45390f55376a5706f9a9c3af22add21ce")

set(FSeq_ARGS ${TLRENDER_EXTERNAL_ARGS})
if(BUILD_SHARED_LIBS)
    list(APPEND FSeq_ARGS -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE)
endif()

ExternalProject_Add(
	FSeq
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/FSeq
	GIT_REPOSITORY ${FSeq_GIT_REPOSITORY}
    GIT_TAG ${FSeq_GIT_TAG}
    LIST_SEPARATOR |
	CMAKE_ARGS ${FSeq_ARGS})
