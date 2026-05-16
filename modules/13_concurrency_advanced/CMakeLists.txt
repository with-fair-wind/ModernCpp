# modules/13_concurrency_advanced — 高级并发 / Advanced Concurrency
#
# 演示：内存序、原子操作、atomic_flag、atomic_ref、栅栏、协程。

find_package(Threads REQUIRED)

mcpp_add_demo(NAME memory_order_seq_cst SOURCES demos/memory_order_seq_cst.cpp LINK_LIBS Threads::Threads)
mcpp_add_demo(NAME memory_order_acq_rel SOURCES demos/memory_order_acq_rel.cpp LINK_LIBS Threads::Threads)
mcpp_add_demo(NAME memory_order_relaxed SOURCES demos/memory_order_relaxed.cpp LINK_LIBS Threads::Threads)
mcpp_add_demo(NAME atomic_operations SOURCES demos/atomic_operations.cpp LINK_LIBS Threads::Threads)
mcpp_add_demo(NAME atomic_flag_spinlock SOURCES demos/atomic_flag_spinlock.cpp LINK_LIBS Threads::Threads)
mcpp_add_demo(NAME atomic_ref_demo SOURCES demos/atomic_ref_demo.cpp LINK_LIBS Threads::Threads)
mcpp_add_demo(NAME fence_demo SOURCES demos/fence_demo.cpp LINK_LIBS Threads::Threads)
mcpp_add_demo(NAME coroutine_basics SOURCES demos/coroutine_basics.cpp LINK_LIBS Threads::Threads)

mcpp_add_test(NAME test_atomic_basic SOURCES tests/test_atomic_basic.cpp LINK_LIBS Threads::Threads)
mcpp_add_test(NAME test_atomic_flag SOURCES tests/test_atomic_flag.cpp LINK_LIBS Threads::Threads)
mcpp_add_test(NAME test_atomic_ref SOURCES tests/test_atomic_ref.cpp LINK_LIBS Threads::Threads)
mcpp_add_test(NAME test_memory_order SOURCES tests/test_memory_order.cpp LINK_LIBS Threads::Threads)
mcpp_add_test(NAME test_fence SOURCES tests/test_fence.cpp LINK_LIBS Threads::Threads)
mcpp_add_test(NAME test_coroutine SOURCES tests/test_coroutine.cpp LINK_LIBS Threads::Threads)
