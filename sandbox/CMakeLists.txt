cmake_minimum_required(VERSION 3.5)

if(NOT NATSMQ_BUILD_SANDBOX)
  return()
endif()

project(NatsMqSandbox LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Network)

file(GLOB test_sources "*.cpp" "*.h")

add_executable(${PROJECT_NAME} ${test_sources})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    NatsMq
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()


