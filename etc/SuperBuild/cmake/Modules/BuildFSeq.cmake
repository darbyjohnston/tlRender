include(ExternalProject)

set(FSeq_GIT_REPOSITORY "https://github.com/darbyjohnston/FSeq.git")
set(FSeq_GIT_TAG "d88d5be1550ccc57806ca02f4caa9326c584348a")

set(FSeq_ARGS ${TLR_EXTERNAL_ARGS})
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
