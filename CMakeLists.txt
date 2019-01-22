cmake_minimum_required (VERSION 2.8)

project(alt-voice)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/tools)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(cotire)

if (MSVC)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /GL /Zi /O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
endif()

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
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/vendor/openal/include/
  ${PROJECT_SOURCE_DIR}/vendor/opus/include/
)

add_library(
  ${PROJECT_NAME} SHARED
  ${PROJECT_SOURCE_FILES}
)

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/opus ${PROJECT_SOURCE_DIR}/libs/${CMAKE_BUILD_TYPE}/opus)
set(LIBTYPE STATIC)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/openal ${PROJECT_SOURCE_DIR}/libs/${CMAKE_BUILD_TYPE}/openal)

if (WIN32)
target_link_libraries(${PROJECT_NAME} WinMM.lib)
endif()

target_link_libraries(${PROJECT_NAME} OpenAL)
target_link_libraries(${PROJECT_NAME} opus)

#Tests
if(BUILD_TESTS)
  add_executable(
    3d-two-sources
    ${PROJECT_SOURCE_DIR}/examples/3d-two-sources.cpp
  )
  if (WIN32)
  target_link_libraries(3d-two-sources WinMM.lib)
  endif()
  target_link_libraries(3d-two-sources alt-voice)
enif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 17
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin/Debug/"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin/Release/"
)

cotire(${PROJECT_NAME})
