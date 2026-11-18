cmake_minimum_required(VERSION 3.25)

project(central_vehicle_computer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CVC_WERROR "Treat compiler warnings as errors" OFF)
set(CVC_SANITIZER "" CACHE STRING "Sanitizers to build with, e.g. address,undefined or thread")

add_library(cvc_options INTERFACE)
target_compile_options(cvc_options INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    $<$<BOOL:${CVC_WERROR}>:-Werror>
)

if(CVC_SANITIZER)
    target_compile_options(cvc_options INTERFACE -fsanitize=${CVC_SANITIZER} -fno-omit-frame-pointer)
    target_link_options(cvc_options INTERFACE -fsanitize=${CVC_SANITIZER})
endif()

add_library(cvc_core
    src/core/scheduler.cpp
    src/core/system_clock.cpp
)
target_include_directories(cvc_core PUBLIC include)
target_link_libraries(cvc_core PUBLIC cvc_options)

add_library(cvc_comm
    src/comm/crc8.cpp
    src/comm/e2e.cpp
    src/comm/e2e_receiver.cpp
    src/comm/e2e_sender.cpp
)
target_include_directories(cvc_comm PUBLIC include)
target_link_libraries(cvc_comm PUBLIC cvc_options)

add_subdirectory(apps)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
