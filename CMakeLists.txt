cmake_minimum_required (VERSION 2.8)

project(alt-voice)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /GL /Zi /O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
endif()

add_definitions(-DALT_VOICE_LIB)
add_definitions(-D_NO_LOGS)

file(GLOB_RECURSE PROJECT_SOURCE_FILES "src/*.h" "src/*.cpp" "include/*.h")

macro(GroupSources curdir groupindex)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)

  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      GroupSources(${curdir}/${child} ${groupindex}/${child})
    else()

      string(REPLACE "/" "\\" groupname ${groupindex})

      source_group(${groupname} FILES ${curdir}/${child})
    endif()
  endforeach()
endmacro()

GroupSources(${PROJECT_SOURCE_DIR}/src "Source Files")
GroupSources(${PROJECT_SOURCE_DIR}/include "Include")
GroupSources(${PROJECT_SOURCE_DIR}/vendor/rnnoise/src "rnnoise")

include_directories(
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/vendor/opus/include/
  ${PROJECT_SOURCE_DIR}/vendor/rnnoise/include/
  ${PROJECT_SOURCE_DIR}/vendor/rtaudio
)

add_library(${PROJECT_NAME} SHARED 
    ${PROJECT_SOURCE_FILES} 
    "vendor/rnnoise/src/rnn.c" 
    "vendor/rnnoise/src/rnn_data.c" 
    "vendor/rnnoise/src/rnn_reader.c" 
    "vendor/rnnoise/src/denoise.c"
)

#rnnoise bullshit
#add_definitions(-DCOMPILE_OPUS)

#opus bullshit
set(OPUS_BUILD_SHARED_LIBRARY OFF)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
set(AVX_SUPPORTED OFF)
set(OPUS_X86_MAY_HAVE_AVX OFF)
set(OPUS_CUSTOM_MODES ON)
set(LIBTYPE STATIC)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/opus ${PROJECT_SOURCE_DIR}/libs/${CMAKE_BUILD_TYPE}/opus)

#rtaudio bullshit
set(RTAUDIO_API_DS ON CACHE BOOL "Build DirectSound API")
set(RTAUDIO_API_WASAPI ON CACHE BOOL "Build WASAPI API")
set(RTAUDIO_BUILD_TESTING OFF)
set(RTAUDIO_BUILD_SHARED_LIBS OFF)
set(RTAUDIO_BUILD_STATIC_LIBS ON)
set(RTAUDIO_STATIC_MSVCRT OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/rtaudio rtaudio)

target_link_libraries(${PROJECT_NAME} opus rtaudio)

#Tests
if(BUILD_TESTS)
  add_executable(
    3d-two-sources
    ${PROJECT_SOURCE_DIR}/examples/3d-two-sources.cpp
  )
  target_link_libraries(3d-two-sources alt-voice)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 17
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin/Debug/"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin/Release/"
)
