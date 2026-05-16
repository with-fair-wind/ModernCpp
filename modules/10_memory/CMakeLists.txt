# modules/10_memory — 内存管理 / Memory Management
#
# 演示：对象布局/对齐、operator new/delete、智能指针、
# PImpl、分配器、PMR。
#
# 测试：布局、对齐、智能指针、分配器、PMR 与类域 new/delete。

find_package(Threads REQUIRED)

mcpp_add_demo(NAME object_layout         SOURCES demos/object_layout.cpp)
mcpp_add_demo(NAME alignment_demo        SOURCES demos/alignment_demo.cpp)
mcpp_add_demo(NAME new_delete            SOURCES demos/new_delete.cpp)
mcpp_add_demo(NAME unique_ptr_demo       SOURCES demos/unique_ptr_demo.cpp)
mcpp_add_demo(NAME shared_ptr_demo       SOURCES demos/shared_ptr_demo.cpp)
mcpp_add_demo(NAME weak_ptr_demo         SOURCES demos/weak_ptr_demo.cpp)
mcpp_add_demo(NAME pimpl_demo            SOURCES demos/pimpl_demo.cpp)
mcpp_add_demo(NAME allocator_basics      SOURCES demos/allocator_basics.cpp)
mcpp_add_demo(NAME pmr_demo              SOURCES demos/pmr_demo.cpp)
mcpp_add_demo(NAME smart_ptr_adaptors    SOURCES demos/smart_ptr_adaptors.cpp)
mcpp_add_demo(NAME false_sharing_demo SOURCES demos/false_sharing_demo.cpp LINK_LIBS Threads::Threads)

mcpp_add_test(NAME test_layout           SOURCES tests/test_layout.cpp)
mcpp_add_test(NAME test_alignment        SOURCES tests/test_alignment.cpp)
mcpp_add_test(NAME test_unique_ptr       SOURCES tests/test_unique_ptr.cpp)
mcpp_add_test(NAME test_shared_ptr       SOURCES tests/test_shared_ptr.cpp)
mcpp_add_test(NAME test_weak_ptr         SOURCES tests/test_weak_ptr.cpp)
mcpp_add_test(NAME test_allocator        SOURCES tests/test_allocator.cpp)
mcpp_add_test(NAME test_pmr              SOURCES tests/test_pmr.cpp)
mcpp_add_test(NAME test_new_delete       SOURCES tests/test_new_delete.cpp)
