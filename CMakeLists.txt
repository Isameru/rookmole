
cmake_minimum_required(VERSION 3.3...3.10)

project(rookmole
    VERSION     0.0
    DESCRIPTION "..."
    LANGUAGES   CXX)

add_library(rookmole STATIC
    src/state.cpp)

target_compile_features(rookmole PUBLIC cxx_std_17)
target_include_directories(rookmole
    PUBLIC include
    PRIVATE src)

if(CMAKE_PROJECT_NAME STREQUAL rookmole)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(test)
    endif()
endif()
