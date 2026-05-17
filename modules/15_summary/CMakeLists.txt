# modules/15_summary — 总结：文件系统、时间、数学与随机数
# Summary: Filesystem, Chrono, Math & Random
#
# 演示：std::filesystem、std::chrono（duration/time_point/日期/时区）、
#       std::ratio、<cmath> / <numbers>、<random>。

mcpp_add_demo(NAME path_basics          SOURCES demos/path_basics.cpp)
mcpp_add_demo(NAME directory_iteration  SOURCES demos/directory_iteration.cpp)
mcpp_add_demo(NAME filesystem_ops       SOURCES demos/filesystem_ops.cpp)
mcpp_add_demo(NAME chrono_duration      SOURCES demos/chrono_duration.cpp)
mcpp_add_demo(NAME chrono_timepoint     SOURCES demos/chrono_timepoint.cpp)
mcpp_add_demo(NAME chrono_date          SOURCES demos/chrono_date.cpp)
mcpp_add_demo(NAME chrono_timezone      SOURCES demos/chrono_timezone.cpp)
mcpp_add_demo(NAME ratio_demo           SOURCES demos/ratio_demo.cpp)
mcpp_add_demo(NAME math_demo            SOURCES demos/math_demo.cpp)
mcpp_add_demo(NAME random_demo          SOURCES demos/random_demo.cpp)
mcpp_add_demo(NAME numbers_demo         SOURCES demos/numbers_demo.cpp)

mcpp_add_test(NAME test_path            SOURCES tests/test_path.cpp)
mcpp_add_test(NAME test_directory       SOURCES tests/test_directory.cpp)
mcpp_add_test(NAME test_filesystem_ops  SOURCES tests/test_filesystem_ops.cpp)
mcpp_add_test(NAME test_duration        SOURCES tests/test_duration.cpp)
mcpp_add_test(NAME test_timepoint       SOURCES tests/test_timepoint.cpp)
mcpp_add_test(NAME test_date            SOURCES tests/test_date.cpp)
mcpp_add_test(NAME test_ratio           SOURCES tests/test_ratio.cpp)
mcpp_add_test(NAME test_math            SOURCES tests/test_math.cpp)
mcpp_add_test(NAME test_random          SOURCES tests/test_random.cpp)
mcpp_add_test(NAME test_numbers         SOURCES tests/test_numbers.cpp)
