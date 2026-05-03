# Dependencies.cmake
#
# 通过 `find_package` 解析第三方依赖。约定由用户用 vcpkg（vcpkg.json manifest）
# 或 Conan（conanfile.txt）来准备依赖，再把对应的 toolchain 文件传给 CMake。
# 安装步骤见 README.md。
#
# 当前管理的依赖：
#   * GoogleTest（仅当 MCPP_BUILD_TESTS 为 ON 时）

if(MCPP_BUILD_TESTS)
    # MSVC CRT 对齐：gtest 默认用静态 CRT（/MT），这里强制成动态 CRT（/MD），
    # 与本仓库使用的 vcpkg triplet（x64-windows = 动态 CRT）以及 CMake 的默认
    # CMAKE_MSVC_RUNTIME_LIBRARY（也是动态）保持一致。
    #
    # 如果你将来切换到静态 CRT 的 triplet（x64-windows-static 等），这里改成
    # OFF，并相应地把 CMAKE_MSVC_RUNTIME_LIBRARY 设为 "MultiThreaded" ——
    # 二者不一致会触发 LNK2038 "RuntimeLibrary mismatch" 链接错误。
    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

    find_package(GTest CONFIG QUIET)
    if(NOT GTest_FOUND)
        find_package(GTest MODULE QUIET)
    endif()

    if(NOT GTest_FOUND)
        message(FATAL_ERROR
            "GoogleTest not found.\n"
            "Install it via one of:\n"
            "  * vcpkg:  add to vcpkg.json and pass "
            "-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake\n"
            "  * Conan:  conan install . --output-folder=build/<preset> --build=missing "
            "then pass -DCMAKE_TOOLCHAIN_FILE=build/<preset>/conan_toolchain.cmake\n"
            "See README.md for details. "
            "To skip tests entirely pass -DMCPP_BUILD_TESTS=OFF.")
    endif()

    # 防御性检查 —— Conan + CMakeDeps 的 build_type 不一致陷阱：
    # CMakeDeps 只会为 `conan install -s build_type=...` 传入的那个 build_type
    # 生成 per-config target 文件（GTest-Target-<lower>.cmake）。如果它跟当前
    # CMake config 不匹配，IMPORTED target 仍会被创建（GTest_FOUND = TRUE），
    # 但它们的 INTERFACE_INCLUDE_DIRECTORIES 会被包在
    # $<$<CONFIG:OtherType>:...> 这种 generator expression 里，针对当前 config
    # 求值为空 —— configure 阶段一切正常，构建期才会以 `gtest/gtest.h: No
    # such file` 炸掉。这里把它在 configure 时就拦下。
    #
    # 我们要对 CMake 实际驱动的每一个 config 都做校验：
    #   * 单配置生成器（Ninja、Makefiles 等）：只有 CMAKE_BUILD_TYPE
    #   * 多配置生成器（VS、Ninja Multi-Config）：CMAKE_CONFIGURATION_TYPES
    #     的每一项 —— Conan 的 per-build-type toolchain 只能填充其中一个，
    #     其它的会在构建期静默炸。我们把它也在 configure 时检出。
    if(TARGET GTest::gtest_main)
        set(_mcpp_configs_to_check)
        if(CMAKE_CONFIGURATION_TYPES)
            list(APPEND _mcpp_configs_to_check ${CMAKE_CONFIGURATION_TYPES})
        elseif(CMAKE_BUILD_TYPE)
            list(APPEND _mcpp_configs_to_check "${CMAKE_BUILD_TYPE}")
        endif()

        if(_mcpp_configs_to_check)
            get_target_property(_mcpp_gtest_inc GTest::gtest_main INTERFACE_INCLUDE_DIRECTORIES)
            if(_mcpp_gtest_inc)
                string(FIND "${_mcpp_gtest_inc}" "$<$<CONFIG:" _mcpp_has_genex)
                if(NOT _mcpp_has_genex EQUAL -1)
                    foreach(_mcpp_cfg IN LISTS _mcpp_configs_to_check)
                        string(FIND "${_mcpp_gtest_inc}" "$<$<CONFIG:${_mcpp_cfg}>" _mcpp_has_match)
                        if(_mcpp_has_match EQUAL -1)
                            message(FATAL_ERROR
                                "GoogleTest was found but has no include "
                                "directories for config '${_mcpp_cfg}'.\n"
                                "This usually means `conan install` was run "
                                "with a different build_type than the CMake "
                                "preset wants to drive. Re-run with: "
                                "-s build_type=${_mcpp_cfg}\n"
                                "(For multi-config generators every config in "
                                "CMAKE_CONFIGURATION_TYPES must be matched by "
                                "a separate `conan install`; in practice that "
                                "means using a single-config preset instead — "
                                "see docs/conan-guide.md.)")
                        endif()
                    endforeach()
                endif()
            endif()
        else()
            message(WARNING
                "Dependencies.cmake: neither CMAKE_BUILD_TYPE nor "
                "CMAKE_CONFIGURATION_TYPES is set — skipping the Conan "
                "build_type/include-dir consistency check. If a Conan "
                "build_type mismatch is present, the build will fail later "
                "with `gtest/gtest.h: No such file`.")
        endif()
    endif()

    include(GoogleTest)
endif()
