include(ExternalProject)

set(nlohmann_json_GIT_REPOSITORY "https://github.com/nlohmann/json.git")
set(nlohmann_json_GIT_TAG "v3.10.5")

set(nlohmann_json_ARGS ${TLRENDER_EXTERNAL_ARGS})
if(BUILD_SHARED_LIBS)
    list(APPEND nlohmann_json_ARGS -DJSON_BuildTests=OFF)
endif()

ExternalProject_Add(
	nlohmann_json
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/nlohmann_json
	GIT_REPOSITORY ${nlohmann_json_GIT_REPOSITORY}
    GIT_TAG ${nlohmann_json_GIT_TAG}
    LIST_SEPARATOR |
	CMAKE_ARGS ${nlohmann_json_ARGS})
