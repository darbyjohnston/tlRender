set(HEADERS
	DeviceData.h
    IDeviceSystem.h
	IOutputDevice.h
	Util.h)

set(SOURCE
	DeviceData.cpp
    IDeviceSystem.cpp
	IOutputDevice.cpp
	Util.cpp)

set(DEVICE_INCLUDE_DIRECTORIES)
set(DEVICE_LINK_LIBRARIES tlTimeline)

if(TLRENDER_BUILD_BMD)

	list(APPEND HEADERS
		BMDDeviceSystem.h
		BMDOutputDevice.h
		BMDUtil.h)
	list(APPEND SOURCE
		BMDDeviceSystem.cpp
		BMDOutputDevice.cpp
		BMDUtil.cpp)

	if(WIN32)
		list(APPEND HEADERS
			${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI.h
			${CMAKE_CURRENT_SOURCE_DIR}/BMD/Win/platform.h)
		list(APPEND SOURCE
			${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI_i.c
			${CMAKE_CURRENT_SOURCE_DIR}/BMD/Win/platform.cpp)
		list(APPEND DEVICE_INCLUDE_DIRECTORIES
			${CMAKE_CURRENT_SOURCE_DIR}/BMD/Win
			${CMAKE_CURRENT_BINARY_DIR})
		list(APPEND DEVICE_LINK_LIBRARIES comsuppw.lib)

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
		list(APPEND HEADERS
			${TLRENDER_BMD_SDK}/Linux/include/DeckLinkAPI.h
			${CMAKE_CURRENT_SOURCE_DIR}/BMD/Linux/platform.h)
		list(APPEND SOURCE
			${TLRENDER_BMD_SDK}/Linux/include/DeckLinkAPIDispatch.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/BMD/Linux/platform.cpp)
		list(APPEND DEVICE_INCLUDE_DIRECTORIES
			${CMAKE_CURRENT_SOURCE_DIR}/BMD/Linux
			${TLRENDER_BMD_SDK}/Linux/include)
	endif()
endif()

add_library(tlDevice ${HEADERS} ${SOURCE})
target_include_directories(tlDevice PUBLIC ${DEVICE_INCLUDE_DIRECTORIES})
target_link_libraries(tlDevice ${DEVICE_LINK_LIBRARIES})
set_target_properties(tlDevice PROPERTIES FOLDER lib)
set_target_properties(tlDevice PROPERTIES PUBLIC_HEADER "${HEADERS}")
if(TLRENDER_BUILD_BMD)
	if(WIN32)
		add_dependencies(tlDevice MIDL_Target)
	endif()
endif()

install(TARGETS tlDevice
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlDevice)
