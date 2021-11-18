include(ExternalProject)

set(GLM_ZIP http://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip)

set(GLM_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DGLM_TEST_ENABLE=FALSE)

ExternalProject_Add(
    GLM
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/GLM
    URL ${GLM_ZIP}
    LIST_SEPARATOR |
    CMAKE_ARGS ${GLM_ARGS}
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/glm ${CMAKE_INSTALL_PREFIX}/include/glm)
