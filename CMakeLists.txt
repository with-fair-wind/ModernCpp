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

message(STATUS "System information:")
message(STATUS "    System processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "    System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "    Pointer size: ${CMAKE_SIZEOF_VOID_P}")
message(STATUS "    Platform: ${CMAKE_VS_PLATFORM_NAME}")
message(STATUS "    Using Windows SDK version: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")

project(ModernCPP LANGUAGES CXX)

# add_executable(Container Containers/containers.cpp)
add_executable(containers Containers/containers.cpp)
add_executable(fundamental_types Basics/fundamental_types.cpp)
add_executable(compound_types Basics/compound_types.cpp)
add_executable(test9 Test/test9.cpp)
add_executable(test10 Test/test10.cpp)
