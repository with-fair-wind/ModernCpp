# modules/04_streams_strings — 流与字符串 / Streams & Strings
#
# 演示：std::string、std::string_view、charconv、format/print、
# 自定义 formatter、iostream/fstream/sstream、spanstream、IO 操纵器。

mcpp_add_demo(NAME string_basics           SOURCES demos/string_basics.cpp)
mcpp_add_demo(NAME string_view_demo        SOURCES demos/string_view_demo.cpp)
mcpp_add_demo(NAME charconv_demo            SOURCES demos/charconv_demo.cpp)
mcpp_add_demo(NAME user_defined_literals   SOURCES demos/user_defined_literals.cpp)
mcpp_add_demo(NAME format_basics           SOURCES demos/format_basics.cpp)
mcpp_add_demo(NAME format_ranges           SOURCES demos/format_ranges.cpp STANDARD 23)
mcpp_add_demo(NAME print_demo              SOURCES demos/print_demo.cpp STANDARD 23)
mcpp_add_demo(NAME user_defined_format     SOURCES demos/user_defined_format.cpp)
mcpp_add_demo(NAME stream_basics           SOURCES demos/stream_basics.cpp)
mcpp_add_demo(NAME stream_buffer           SOURCES demos/stream_buffer.cpp)
mcpp_add_demo(NAME spanstream_demo         SOURCES demos/spanstream_demo.cpp STANDARD 23)
mcpp_add_demo(NAME io_manipulators         SOURCES demos/io_manipulators.cpp)

mcpp_add_test(NAME test_string             SOURCES tests/test_string.cpp)
mcpp_add_test(NAME test_string_view        SOURCES tests/test_string_view.cpp)
mcpp_add_test(NAME test_charconv           SOURCES tests/test_charconv.cpp)
mcpp_add_test(NAME test_format             SOURCES tests/test_format.cpp)
mcpp_add_test(NAME test_format_ranges      SOURCES tests/test_format_ranges.cpp STANDARD 23)
mcpp_add_test(NAME test_print              SOURCES tests/test_print.cpp STANDARD 23)
mcpp_add_test(NAME test_stream             SOURCES tests/test_stream.cpp)
mcpp_add_test(NAME test_user_format        SOURCES tests/test_user_format.cpp)

find_package(Threads REQUIRED)

mcpp_add_demo(NAME regex_demo              SOURCES demos/regex_demo.cpp)
mcpp_add_demo(NAME fstream_demo            SOURCES demos/fstream_demo.cpp)
mcpp_add_demo(NAME stringstream_demo       SOURCES demos/stringstream_demo.cpp)
mcpp_add_demo(NAME osyncstream_demo        SOURCES demos/osyncstream_demo.cpp LINK_LIBS Threads::Threads)

mcpp_add_test(NAME test_regex              SOURCES tests/test_regex.cpp)
mcpp_add_test(NAME test_fstream            SOURCES tests/test_fstream.cpp)
mcpp_add_test(NAME test_stringstream       SOURCES tests/test_stringstream.cpp)
