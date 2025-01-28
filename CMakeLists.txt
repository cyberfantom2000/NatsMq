cmake_minimum_required(VERSION 3.5)

include(FetchContent)
include(GenerateExportHeader)

project(NatsMq LANGUAGES CXX)

option(NATSMQ_BUILD_TESTS "Build unit tests" OFF)
option(NATSMQ_BUILD_EXAMPLES "Build examples" OFF)
option(NATSMQ_BUILD_SANDBOX "Build sandbox" OFF)
#option(NATSMQ_USE_LIBEVENT "Use libevent library" OFF)
#option(NATSMQ_USE_LIBUV "Use libuv library" OFF)

option(with_asan "With Address Sanitazer" OFF)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (with_asan)
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment" )
endif()

if(UNIX)
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall --std=c++17 -O0 -fPIC" )
  set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -O0 -fPIC" )
endif()

FetchContent_Declare(
  cnats
  GIT_REPOSITORY https://github.com/nats-io/nats.c.git
  GIT_TAG        v3.8.2
  GIT_SHALLOW    ON
)

set(NATS_BUILD_WITH_TLS OFF)
set(NATS_BUILD_EXAMPLES OFF)
set(NATS_BUILD_STREAMING OFF)
set(NATS_BUILD_LIB_SHARED OFF)
set(BUILD_TESTING OFF)

add_compile_definitions(PICOJSON_USE_INT64)

FetchContent_MakeAvailable(cnats)
add_library(cnats STATIC IMPORTED)

file(GLOB sources
    "include/*.cpp"
    "src/*.cpp"
    "src/core/*.cpp"
    "src/js/*.cpp"
    "src/private/*.cpp")

file(GLOB headers
    "include/*.h"
    "src/*.h"
    "src/core/*.h"
    "src/js/*.h"
    "src/private/*.h")

add_library(${PROJECT_NAME} SHARED ${sources} ${headers})

target_link_libraries(${PROJECT_NAME} PRIVATE nats_static)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

GENERATE_EXPORT_HEADER(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/Export.h)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

if(NATSMQ_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(NATSMQ_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if(NATSMQ_BUILD_SANDBOX)
  add_subdirectory(sandbox)
endif()

