cmake_minimum_required (VERSION 2.8)

project(alt-voice)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/tools)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(cotire)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /GL /LTCG /Zi /O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

add_definitions(-DALT_VOICE_LIB)
add_definitions(-DAL_LIBTYPE_STATIC)

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

include_directories(
  ./src
  ./include
  ${PROJECT_SOURCE_DIR}/vendor/openal/include/
  ${PROJECT_SOURCE_DIR}/vendor/opus/include/
  ${PROJECT_SOURCE_DIR}/vendor/portaudio/include/
)

link_directories(
  ${PROJECT_SOURCE_DIR}/libs
)

add_library(
  ${PROJECT_NAME} SHARED
  ${PROJECT_SOURCE_FILES}
)

target_link_libraries(${PROJECT_NAME} WinMM.lib)
target_link_libraries(${PROJECT_NAME} OpenAL32.lib)
target_link_libraries(${PROJECT_NAME} opus.lib)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 17
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/client"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/client"
)

cotire(${PROJECT_NAME})
