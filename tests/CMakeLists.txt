cmake_minimum_required(VERSION 3.5)

if(NOT NATSMQ_BUILD_TESTS)
  return()
endif()

include(FetchContent)

project(NatsMqTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB examples_src RELATIVE ${PROJECT_SOURCE_DIR}/examples *.cpp)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(Qt5 REQUIRED COMPONENTS Core Network)

file(GLOB resources "tests/python/SimpleNatsClient.py")
file(COPY ${resources} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(GLOB test_sources "*.cpp" "*.h")
add_executable(${PROJECT_NAME} ${test_sources})
target_link_libraries(${PROJECT_NAME} PRIVATE
    gtest_main
    gmock_main
    Qt::Core
    Qt::Network
    NatsMq
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()


