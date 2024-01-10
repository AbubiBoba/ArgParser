cmake_minimum_required(VERSION 3.12)

project(
    labwork4
    VERSION 1.0
    DESCRIPTION "C++ Year 2023 labwork4"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)


add_subdirectory(lib/argparser)
add_subdirectory(bin)
add_subdirectory(examples)


enable_testing()
add_subdirectory(tests)
