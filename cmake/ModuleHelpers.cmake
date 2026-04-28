# ModuleHelpers.cmake
#
# Per-module helpers that keep each modules/NN_*/CMakeLists.txt short.
#
# mcpp_add_demo(NAME <name>
#               SOURCES <files...>
#               [STANDARD <n>]
#               [LINK_LIBS <targets...>])
#   Builds one executable per demo, linked against mcpp::warnings.
#   Target name is "<module>__<name>" to stay globally unique.
#   STANDARD overrides CMAKE_CXX_STANDARD for this single target.
#   LINK_LIBS adds extra PRIVATE link dependencies (e.g. Threads::Threads)
#   without modules having to know the internal target name pattern.
#
# mcpp_add_test(NAME <name>
#               SOURCES <files...>
#               [STANDARD <n>]
#               [LINK_LIBS <targets...>])
#   Same as demo but additionally links GTest::gtest_main and registers
#   test cases via gtest_discover_tests.

include_guard(GLOBAL)

function(_mcpp_current_module OUT_VAR)
    get_filename_component(_mod "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
    set(${OUT_VAR} "${_mod}" PARENT_SCOPE)
endfunction()

function(_mcpp_apply_standard TARGET STANDARD)
    if(NOT STANDARD STREQUAL "")
        target_compile_features(${TARGET} PRIVATE cxx_std_${STANDARD})
        set_target_properties(${TARGET} PROPERTIES
            CXX_STANDARD ${STANDARD}
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF)
    endif()
endfunction()

function(mcpp_add_demo)
    if(NOT MCPP_BUILD_DEMOS)
        return()
    endif()

    set(options)
    set(oneValueArgs NAME STANDARD)
    set(multiValueArgs SOURCES LINK_LIBS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_NAME)
        message(FATAL_ERROR "mcpp_add_demo: NAME is required")
    endif()
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "mcpp_add_demo(${ARG_NAME}): SOURCES is required")
    endif()

    _mcpp_current_module(_module)
    set(_target "${_module}__${ARG_NAME}")

    add_executable(${_target} ${ARG_SOURCES})
    target_link_libraries(${_target} PRIVATE mcpp::warnings mcpp::sanitizers)
    if(ARG_LINK_LIBS)
        target_link_libraries(${_target} PRIVATE ${ARG_LINK_LIBS})
    endif()
    set_target_properties(${_target} PROPERTIES
        OUTPUT_NAME "${ARG_NAME}"
        # Per-module output directory keeps demo / test executables from
        # different modules from colliding when they happen to share a NAME
        # (e.g. two modules both register a `playground` demo).
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${_module}"
        FOLDER "modules/${_module}/demos")
    _mcpp_apply_standard(${_target} "${ARG_STANDARD}")
endfunction()

function(mcpp_add_test)
    if(NOT MCPP_BUILD_TESTS)
        return()
    endif()

    set(options)
    set(oneValueArgs NAME STANDARD)
    set(multiValueArgs SOURCES LINK_LIBS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_NAME)
        message(FATAL_ERROR "mcpp_add_test: NAME is required")
    endif()
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "mcpp_add_test(${ARG_NAME}): SOURCES is required")
    endif()

    _mcpp_current_module(_module)
    set(_target "${_module}__${ARG_NAME}")

    add_executable(${_target} ${ARG_SOURCES})
    target_link_libraries(${_target}
        PRIVATE
        mcpp::warnings
        mcpp::sanitizers
        GTest::gtest
        GTest::gtest_main)
    if(ARG_LINK_LIBS)
        target_link_libraries(${_target} PRIVATE ${ARG_LINK_LIBS})
    endif()
    set_target_properties(${_target} PROPERTIES
        OUTPUT_NAME "${ARG_NAME}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${_module}"
        FOLDER "modules/${_module}/tests")
    _mcpp_apply_standard(${_target} "${ARG_STANDARD}")

    gtest_discover_tests(${_target}
        TEST_PREFIX "${_module}."
        PROPERTIES LABELS "${_module}")
endfunction()
