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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(ModernCPP LANGUAGES CXX)

# link_libraries(stdc++exp)

# add_executable(Container Containers/containers.cpp)
add_executable(containers Containers/containers.cpp)
add_executable(fundamental_types Basics/fundamental_types.cpp)
add_executable(compound_types Basics/compound_types.cpp)
add_executable(test9 Test/test9.cpp)
add_executable(test10 Test/test10.cpp)
add_executable(test11 Test/test11.cpp)
add_executable(test12 Test/test12.cpp)
add_executable(test13 Test/test13.cpp)
add_executable(test14 Test/test14.cpp Test/t1.cpp Test/t2.cpp)

# target_link_libraries(test9 PUBLIC stdc++exp)