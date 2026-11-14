cmake_minimum_required(VERSION 3.25)

project(car_gateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(GATEWAY_WERROR "Treat compiler warnings as errors" OFF)
set(GATEWAY_SANITIZER "" CACHE STRING "Sanitizers to build with, e.g. address,undefined or thread")

add_library(gateway_options INTERFACE)
target_compile_options(gateway_options INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    $<$<BOOL:${GATEWAY_WERROR}>:-Werror>
)

if(GATEWAY_SANITIZER)
    target_compile_options(gateway_options INTERFACE -fsanitize=${GATEWAY_SANITIZER} -fno-omit-frame-pointer)
    target_link_options(gateway_options INTERFACE -fsanitize=${GATEWAY_SANITIZER})
endif()

add_library(gateway_core
    src/core/scheduler.cpp
    src/core/system_clock.cpp
)
target_include_directories(gateway_core PUBLIC include)
target_link_libraries(gateway_core PUBLIC gateway_options)

add_subdirectory(apps)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
