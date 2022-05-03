set(HEADERS
    DeviceSystem.h
	OutputDevice.h
	Util.h)

set(SOURCE
    DeviceSystem.cpp
	OutputDevice.cpp
	Util.cpp)

set(DECKLINK_INCLUDE_DIR)
set(DECKLINK_SOURCE)
set(DECKLINK_LIBRARIES)
if(WIN32)
	set(DECKLINK_INCLUDE_DIR
	    ${CMAKE_CURRENT_SOURCE_DIR}/Win
		${CMAKE_CURRENT_BINARY_DIR})

    set(DECKLINK_SOURCE
        ${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI_i.c
        ${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Win/platform.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Win/platform.cpp)

    list(APPEND DECKLINK_LIBRARIES comsuppw.lib)

	set(MIDL_OUTPUT_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI_i.c")
    set(MIDL_OUTPUT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI.h")
    set(MIDL_OUTPUT_TLB "${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI.tlb")
	set(MIDL_OUTPUT
        ${MIDL_OUTPUT_SOURCE}
        ${MIDL_OUTPUT_HEADER}
        ${MIDL_OUTPUT_TLB})

    set(DECKLINK_API_IDL "${TLRENDER_BMD_SDK}/Win/include/DeckLinkAPI.idl")
	
	set(MIDL_MACHINE amd64)

    add_custom_command(
        OUTPUT ${MIDL_OUTPUT}
        DEPENDS ${DECKLINK_API_IDL}
        COMMAND midl /env ${MIDL_MACHINE} /header ${MIDL_OUTPUT_HEADER} ${DECKLINK_API_IDL} /tlb ${MIDL_OUTPUT_TLB}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_source_files_properties(${MIDL_OUTPUT} PROPERTIES GENERATED TRUE)
    add_custom_target(MIDL_Target DEPENDS ${MIDL_OUTPUT})
elseif(APPLE)
elseif(UNIX)
	set(DECKLINK_INCLUDE_DIR
	    ${CMAKE_CURRENT_SOURCE_DIR}/Linux
		${TLRENDER_BMD_SDK})

    set(DECKLINK_SOURCE
        ${TLRENDER_BMD_SDK}/DeckLinkAPI.h
        ${TLRENDER_BMD_SDK}/DeckLinkAPIDispatch.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Linux/platform.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Linux/platform.cpp)
endif()

add_library(tlBMD ${HEADERS} ${SOURCE} ${DECKLINK_SOURCE})
target_include_directories(tlBMD PUBLIC ${DECKLINK_INCLUDE_DIR})
target_link_libraries(tlBMD tlTimeline ${DECKLINK_LIBRARIES})
set_target_properties(tlBMD PROPERTIES FOLDER lib)
set_target_properties(tlBMD PROPERTIES PUBLIC_HEADER "${HEADERS}")
if(WIN32)
    add_dependencies(tlBMD MIDL_Target)
endif()

install(TARGETS tlBMD
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlBMD)
