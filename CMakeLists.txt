cmake_minimum_required(VERSION 3.18)

if(WIN32)
    # set(CMAKE_SYSTEM_VERSION 10.0.17763.0)
    add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release)
    endif()
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(ModernCPP LANGUAGES CXX)

# add_executable(Container Containers/containers.cpp)
add_executable(containers Containers/containers.cpp)
add_executable(fundamental_types Basics/fundamental_types.cpp)
add_executable(compound_types Basics/compound_types.cpp)
