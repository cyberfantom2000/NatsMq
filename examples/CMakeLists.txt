cmake_minimum_required(VERSION 3.5)

if(NOT NATSMQ_BUILD_EXAMPLES)
  return()
endif()

include(FetchContent)

project(NatsMqExamples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB examples_src RELATIVE ${PROJECT_SOURCE_DIR}/examples *.cpp)

find_package(Qt5 REQUIRED COMPONENTS Core)

foreach(src ${examples_src})

    string(REPLACE ".cpp" "" example_name ${src})
    string(REPLACE "../" "" example_name ${example_name})
    set(example_exe "natsmq-${example_name}")

    add_executable(${example_exe} "${PROJECT_SOURCE_DIR}/examples/${src}")
    target_link_libraries(${example_exe} NatsMq Qt::Core)

endforeach()
