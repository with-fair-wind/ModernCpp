# modules/11_error_handling — 错误处理 / Error Handling
#
# Demos for exceptions, std::error_code, std::expected (C++23), assertions.

mcpp_add_demo(NAME expected_basics SOURCES demos/expected_basics.cpp)

mcpp_add_test(NAME test_expected   SOURCES tests/test_expected.cpp)
