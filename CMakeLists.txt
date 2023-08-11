cmake_minimum_required(VERSION 3.5)

include(FetchContent)
include(GenerateExportHeader)

project(NatsMq LANGUAGES CXX)

option(NATSMQ_BUILD_TESTS "Build unit tests" OFF)
option(NATSMQ_BUILD_EXAMPLES "Build examples" OFF)
#option(NATSMQ_USE_LIBEVENT "Use libevent library" OFF)
#option(NATSMQ_USE_LIBUV "Use libuv library" OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

FetchContent_Declare(
  cnats
  GIT_REPOSITORY https://github.com/nats-io/nats.c.git
  GIT_TAG        v3.6.0
  GIT_SHALLOW    ON
)

set(NATS_BUILD_WITH_TLS OFF)
set(NATS_BUILD_EXAMPLES OFF)
set(NATS_BUILD_STREAMING OFF)
set(NATS_BUILD_LIB_SHARED OFF)
set(BUILD_TESTING OFF)

FetchContent_MakeAvailable(cnats)

file(GLOB sources
    "include/*.cpp"
    "src/*.cpp"
    "src/core/*.cpp"
    "src/jetstream/*.cpp")

file(GLOB headers
    "include/*.h"
    "src/*.h"
    "src/core/*.h"
    "src/jetstream/*.h"
    "private/*.h")

add_library(${PROJECT_NAME} SHARED ${sources} ${headers})

target_link_libraries(${PROJECT_NAME} PRIVATE nats_static)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

GENERATE_EXPORT_HEADER(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/natsmq_export.h)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/core>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/jetstream>
)

if(NATSMQ_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
if(NATSMQ_BUILD_EXAMPLES)
  add_subdirectory(tests)
endif()

