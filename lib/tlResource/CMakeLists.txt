set(HEADERS)
set(SOURCE)
set(RESOURCES
    etc/Icons/ColorControls.svg
    etc/Icons/ColorPicker.svg
    etc/Icons/CompareA.svg
    etc/Icons/CompareB.svg
    etc/Icons/CompareDifference.svg
    etc/Icons/CompareHorizontal.svg
    etc/Icons/CompareOverlay.svg
    etc/Icons/Compare.svg
    etc/Icons/CompareTile.svg
    etc/Icons/CompareVertical.svg
    etc/Icons/CompareWipe.svg
    etc/Icons/Devices.svg
    etc/Icons/Export.svg
    etc/Icons/FileOpenSeparateAudio.svg
    etc/Icons/Files.svg
    etc/Icons/Hidden.svg
    etc/Icons/Info.svg
    etc/Icons/Messages.svg
    etc/Icons/tlRender.svg
    etc/Icons/View.svg
    etc/Icons/Visible.svg
    etc/Icons/WindowFullScreen.svg
    etc/Icons/WindowSecondary.svg)
foreach(RESOURCE ${RESOURCES})
    get_filename_component(RESOURCE_BASE ${RESOURCE} NAME_WE)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_BASE}.cpp
        COMMAND dtkres ${CMAKE_SOURCE_DIR}/${RESOURCE} ${RESOURCE_BASE} tl_resource
        DEPENDS ${CMAKE_SOURCE_DIR}/${RESOURCE})
    list(APPEND SOURCE ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_BASE}.cpp)
endforeach()

add_library(tlResource ${HEADERS} ${SOURCE})
target_link_libraries(tlResource)
set_target_properties(tlResource PROPERTIES FOLDER lib)
set_target_properties(tlResource PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS tlResource
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlResource)
