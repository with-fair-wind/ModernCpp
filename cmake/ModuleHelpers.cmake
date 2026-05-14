# ModuleHelpers.cmake
#
# 让每个 modules/NN_*/CMakeLists.txt 都能保持简短的辅助函数。
#
# mcpp_add_demo(NAME <name>
#               SOURCES <files...>
#               [STANDARD <n>]
#               [LINK_LIBS <targets...>]
#               [INCLUDE_DIRS <dirs...>]
#               [PCH <headers...>])
#   每个 demo 编译成一个可执行文件，链接到 mcpp::warnings。
#   target 名为 "<module>__<name>"，确保全局唯一。
#   STANDARD 用于覆盖单 target 的 CMAKE_CXX_STANDARD。
#   LINK_LIBS 追加 PRIVATE 链接依赖（例如 Threads::Threads），
#   这样模块就不需要知道内部 target 命名规则。
#   INCLUDE_DIRS 追加 PRIVATE 头文件搜索路径（替代 directory-scoped 的
#   include_directories()，粒度到单个 target）。
#   PCH 指定预编译头（调用 target_precompile_headers PRIVATE）。
#
# mcpp_add_test(NAME <name>
#               SOURCES <files...>
#               [STANDARD <n>]
#               [LINK_LIBS <targets...>]
#               [INCLUDE_DIRS <dirs...>]
#               [PCH <headers...>])
#   行为同 mcpp_add_demo，但额外链接 GTest::gtest_main，
#   并通过 gtest_discover_tests 注册测试用例。

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
    set(multiValueArgs SOURCES LINK_LIBS INCLUDE_DIRS PCH)
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
    if(ARG_INCLUDE_DIRS)
        target_include_directories(${_target} PRIVATE ${ARG_INCLUDE_DIRS})
    endif()
    if(ARG_PCH)
        target_precompile_headers(${_target} PRIVATE ${ARG_PCH})
    endif()
    set_target_properties(${_target} PROPERTIES
        OUTPUT_NAME "${ARG_NAME}"
        # 给每个模块一个独立的输出目录，避免不同模块下同名（NAME 冲突）的
        # demo / test 可执行文件互相覆盖（例如两个模块都叫 `playground`）。
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
    set(multiValueArgs SOURCES LINK_LIBS INCLUDE_DIRS PCH)
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
    if(ARG_INCLUDE_DIRS)
        target_include_directories(${_target} PRIVATE ${ARG_INCLUDE_DIRS})
    endif()
    if(ARG_PCH)
        target_precompile_headers(${_target} PRIVATE ${ARG_PCH})
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
