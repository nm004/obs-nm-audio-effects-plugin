cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" OFF)
option(ENABLE_QT "Use Qt functionality" OFF)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

if (CMAKE_HOST_WIN32)
	set(DEFAULT_FAUST_DIR "C:/Program Files/Faust")
else()
	set(DEFAULT_FAUST_DIR "/usr")
endif()
set(FAUST_DIR ${DEFAULT_FAUST_DIR} CACHE PATH "Path to Faust installation directory")

add_library(faust INTERFACE)
target_include_directories(faust INTERFACE "${FAUST_DIR}/include")
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE faust)

list(APPEND faustInputs
	src/faust/basicfilter.dsp
	src/faust/echo.dsp
	src/faust/loudspeaker.dsp
	src/faust/radio.dsp
	src/faust/zitarev1.dsp)
list(APPEND faustOutputs ${faustInputs})
list(TRANSFORM faustOutputs REPLACE "[.]dsp$" ".h")
add_custom_command(
	OUTPUT
	${faustOutputs}
	COMMAND ${FAUST_DIR}/bin/faust
	ARGS -nvi -inpl -ns nm -cn BasicFilter -o src/faust/basicfilter.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/faust/basicfilter.dsp
	COMMAND ${FAUST_DIR}/bin/faust
	ARGS -nvi -inpl -ns nm -cn Echo -o src/faust/echo.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/faust/echo.dsp
	COMMAND ${FAUST_DIR}/bin/faust
	ARGS -nvi -inpl -ns nm -cn LoudSpeaker -o src/faust/loudspeaker.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/faust/loudspeaker.dsp
	COMMAND ${FAUST_DIR}/bin/faust
	ARGS -nvi -inpl -ns nm -cn Radio -o src/faust/radio.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/faust/radio.dsp
	COMMAND ${FAUST_DIR}/bin/faust
	ARGS -nvi -inpl -ns nm -cn ZitaRev1 -o src/faust/zitarev1.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/faust/zitarev1.dsp
	DEPENDS ${faustInputs})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src/faust)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${faustOutputs} src/plugin-main.cc)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
