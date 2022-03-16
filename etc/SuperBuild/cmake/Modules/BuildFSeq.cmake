include(ExternalProject)

set(FSeq_GIT_REPOSITORY "https://github.com/darbyjohnston/FSeq.git")
set(FSeq_GIT_TAG "cda02f1793fe7f58b1b76aa6d427a3190226769a")

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
