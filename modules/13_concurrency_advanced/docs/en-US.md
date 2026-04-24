并发进阶 Advanced Concurrency

## 现代C++基础

## Modern C++ Basics

Jiaming Liang, undergraduate from Peking University

### • Memory Order Basics

### • Atomic Variable Details

### • Advanced Memory Order

### • Coroutine

# Advanced Concurrency

Memory Order Basics

“Even with C++11 support, I consider lock-free programming expert-level work.” -- Bjarne Stroustrup, HoPL4, P33

# Advanced Concurrency

- Memory Order Basics
- Overview
- Sequentially consistent model
- Acquire-release model
- Relaxed model

- There also exists consume-release model, but since it’s very difficult for users to annotate and for compilers to analyze better optimizations, all compilers strengthen consume-release model to acquire-release model.
- C++20:

- C++26: consume operations are deprecated.

# Memory order

- Current programming world stands on the foundation of sequential execution…
- Compiler / JIT may do aggressive optimization…
- Here we will “cache” global variables to registers, and eliminate redundant expressions (i.e. b = addend + 1).

- Processors may do out-of-order execution and speculative computation…
- Each processor may have its own L1/L2 cache…

# Memory order

- These optimizations are smart and correct in sequential world, but when it comes to parallelism, some assumptions are not that intuitive…

- What if there is another thread that modifies addend here?
- b can be something other than tempb + 4, but compiler optimizations make it impossible.

# Memory order

- Among so many compiler optimizations, processor ISA regulations, cache coherence protocols…
- We need to find a way to unify “as-if” behaviors by abstraction!
- That is what memory order for in C++.
- Three types of memory order:
- Sequentially consistent model (seq_cst)
- Acquire-release model (acq_rel)
- Relaxed model (relaxed)
- BTW, Rust has completely same regulations as C++.

Atomics - The Rustonomicon

[1]: A Concurrency Semantics for Relaxed Atomics that Permits Optimisation and Avoids Thin-air Executions | POPL’16, Jean & Peter from Univ. of Cambridge POPL is Top Academic Conference in Programming Language Design.

# Memory order

- But, how to describe memory order is still an unsolved problem even in academia (even seq_cst model has bug fix in C++20).
- And C++ is pioneer in this field, so the standard has been revised nearly in every version.
- But normally this is defect in theoretical model; real-world behaviors are not severely affected. [1]
- The key problem is that memory order is axiomatic, which is rather weak and cannot exactly describe what we want.
- Memory order gives constraints, and every outcome that can fulfill the constraint is a valid solution. Results that
- While some solutions are not really valid…we’ll see them memory order later.allows What memory order regulates:

Formally, this is regulated by RR/RW/WR/WW

# Memory ordercoherence in standard; we rephrase it here.

- There are some intuitive basic regulations in memory model.
1. Modification order: for a single atomic variable, all threads see the same operation sequences.
- So can r1 == 1 && r2 == 2 && r3 == 2 && r4 == 1?
- No!
- Reason: r4 cannot read value newer than r3, and r2 cannot read value newer than r1.
- r1 == 1 && r2 == 2: 2 is newer than 1;
- r3 == 2 && r4 == 1: 1 is newer than 2; Conflict!
- Compilers are not allowed to reorder.
- But, operations for different atomic variables may have different orders in different threads.

# Memory order

2. Sequenced before: we’ve covered evaluation order previously…

# Memory order

- So if an evaluation A definitely computes before another one B, then we say A is sequenced before B.
- For example, for different statements.
- In the same statement:

- And function parameters are indeterminately sequenced since C++17, so there is some order but it’s unspecified;
- And some evaluations are not regulated at all, which means they’re unsequenced (e.g. a = b++ + b is UB, since b++ and b are unsequenced while b++ has side effect).
- Again, such order is in the sequential view…

Data races occur when non-atomic operations on the same memory location do NOT have

# Memory ordersome certain happens-before relationship.

3. Happens before: in parallel world, which evaluation is executed first is regulated by happens-before.
- If A is sequenced before B, then A happens before B (single-thread case);
- If A synchronizes with B, then A happens before B (inter-thread case);
- Or A happens before B & B happens before C, then A happens before C.

- For non-atomic variables, only when A happens before B will effects of A be visible to B.
- So compilers can do aggressive optimizations, as long as they aren’t visible.
- For atomic variables, HB order is part of MO; if two operations have no HB relationship, then their order in MO is also random.
- Namely, if B doesn’t happen before A, then effects of A may be visible to B.
- Memory order mainly regulates such “synchronize-with” relationship. Note: actually, what we teach here is happens-before since C++26; before that (since C++20) this is called simply- happens-before, but it’s equivalent to happens-before (since C++11) when no consume operation is involved (and again, we’ve said that consume operations are never implemented).

# Advanced Concurrency

- Memory Order Basics
- Overview
- Sequentially consistent model
- Acquire-release model
- Relaxed model

# Sequential Consistency

- In real world, all events are sequenced in some way, and all observers will see the same sequence.
- Similarly, we may think operations to have some total order, and all threads observe the same order.
- This is the core of sequentially consistent model!
- Back to our example: x.load()r1=0 x.store(1)x.store(1) x.load()r3=1 x.store(2) x.store(2) x.load() x.load() x.load()r4=2

x.load() x.load()x.load()r2=2 Interleaving them randomly, we get a total order.

# Sequential Consistency

- Formally, when an atomic load operation B loads a value that’s stored by an atomic store operation A, then A synchronizes with B.
- Then all previous outcomes are visible since B.
- For example:

Can this assert fire?

- No, z.load() is always non-zero.
- Reason: there is a total order, so either x.store(true) or y.store(true) occurs first.
- Let’s assume x.store(true) happens first since it’s completely symmetric.

Synchronize withVisible to x.store(true) Synchronize with y.store(true) For thread 3, onlyFor thread 4, only when x.load() seeswhen y.load() sees x.load()y.load() true can it proceed.true can it proceed. ?

y.load() may gety.load() x.load()x.load() gets true. false.

- Synchronization is implicitly established through reading value.
- Note: x.store(true) and y.store(true) do NOT have happens-before relationship; the order is imposed by total order.

seq_cst model actually uses strongly-happens-before relationship but here they’re equivalent; we’ll cover it in the next section.

# Sequential Consistency

- Note 2: start of threads & joining threads will also establish synchronize-with relationship with function start & return.
- So here thread joining happens before z.load(), and function return happens before thread joining, and ++z happens before function return. Thus z.load() can get 1 or 2 correctly.
- Note 3: operations on atomic variables are indivisible (and thus prevent data races), which is not affected by memory order.
- Called atomicity.
- Our example in the last lecture:
- If a is atomic variable, then lock protection is not needed.

# Advanced Concurrency

- Memory Order Basics
- Overview
- Sequentially consistent model
- Acquire-release model
- Relaxed model

# Acquire-release Model

- In many architectures like RISC-V, ARM and Power, such total- order assumption is quite expensive, while they support weaker model better.
- Acquire-release is a commonly supported order!

- So what does acquire-release model guarantee?
- Only read operations can be “acquire”, and only write operations can be “release”.
- For an acquire operation B, if it reads the value from a release operation A, then A synchronizes with B (and thus A happens before B).
- There is no total order.

# Acquire-release Model

- For example:

Sequenced before store, thus happens before store.

Only when ptr loads some value will the program proceed, then store synchronizes with load (andThrough three thus happens before load).happens-before, we know that Sequenced after load, thusdatais always 42. load happens before asserts.

# Acquire-release Model

- On the other hand, since it doesn’t have total order:

Store of x and y have no happens-before relationship.

Here we only know that x is true (synchronize-with), while y.load and y.store don’t necessarily have happens-before relationship.

Similarly, x.load and x.store don’t necessarily have happens-before relationship.

Thus, z.load() can be 0 here.

# Acquire-release Model

- Another example for transitivity:
- SB(#0, #1)
- SW(#1, #2)
- As only when #2 reads true can thread_2 proceed.
- SB(#2, #3)
- SW(#3, #4)
- SB(#4, #5)
- Thus we know HB(#0, #5).

Obviously, acquire-release model can be used to implement spinlock.

# Acquire-release Model

- By happens-before relationship, acquire-release model implicitly disables compiler reorder optimization.
- An acquire operation B may happen after another release operation A…
- If a compiler reorders statements S1 after B to before B;
- Or if a compiler reorders statements S2 before A to after A;
- Then S1 may fail to observe results in S2.
- Thus, acquire & release offers a one-way instruction barrier implicitly.
- All operations that will cause side effects (that may be used by another threads) cannot go below beyond a release operation;
- All operations that may rely on side effects cannot go above beyond an acquire operation.
- Intuitively, acquire-release forms some critical section; you cannot move out code in between.

# Advanced Concurrency

- Memory Order Basics
- Overview
- Sequentially consistent model
- Acquire-release model
- Relaxed model

# Relaxed Model

- Sometimes we may want even weaker order…
- That is, we only need to maintain atomicity; no synchronize-with relationship is needed.
- This is relaxed model.
- For example:

Exercise: Can this assert fire?

# Relaxed Model

- No!
- Assuming that r1 == 42,
- Then #1 reads value from #4, and acquire-release model makes SW(#4, #1).
- And SB(#3, #4), SB(#1, #2), thus we know HB(#3, #2).
- Thus, effects of #2 are not visible to #3, and r2 is definitely 0.
- Then what about relaxed model?
- This assertion may fire…
- That is, r1 == 42 && r2 == 42 may be true.

# Relaxed Model

- Since relaxed model doesn’t establish any synchronize-with relationship…
- Remember our effect rules?

- So here #1 doesn’t happen before #4, then effects of #4 can be read by #1 so r1 == 42 can be true.
- And #1 happens before #2, so x can store 42.
- And #3 doesn’t happen before #2, then effects of #2 can be read by #3 so r2 == 42 can be true.
- Thus, r1 == 42 && r2 == 42 can be true.

# Relaxed Model

- Note 1: again, we emphasize that there is no total order.
- If there is, then in thread 2 SB(#3, #4) prevents any possible order to make r2 == 42.
- In practice, compilers are allowed to reorder #3 and #4, since destroying such HB doesn’t affect any visible effects.
- Note 2: this outcome doesn’t violate modification order constraint of a single atomic variable. All threads see this same modification order.

#3 can read r1, x0r1Notice that here it’s so it can be 42. can instead of must; #1 and #3 can read older values. y#1 can read 42, 0 42 so r1 can be 42.

# Relaxed Model

- Another complex example:

Like a spinlock, covered later.

All threads start now.

# Relaxed Model

- So what it does is:
- Three threads, with each one only modifying one of the atomic variables, and reading all of them;
- Two threads that only read all atomic variables.
- It can only guarantee that:
- The thread that modifies the variable will see it increases one by one, constrained by happens-before relationship.
- For example, values[0] will have (0, …, …), (1, …, …), …, (9, …, …).
- And constrained by single-atomic modification order, other variables that are not modified by itself will have non-decreasing values.
- That is, once a value is read (not necessarily the newest), values older than it cannot be read.

# Relaxed Model

nd
- Courtesy of C++ Concurrency in Action, 2ed. by Anthony Williams.

# Relaxed Model

- Relaxed model may cause very astonishing results, so it needs to be used with extreme caution…
- Usually it either cooperates with other sync operations (like acquire- release model)…
- Or it’s used to do very simple job that only needs atomicity.
- For example, std::shared_ptr has a counter to count its copies; when all copies are destructed, the memory is finally freed.
- We can check the shared count by .use_count(), which is normally a relaxed load since it doesn’t need to participate in synchronization.

# Advanced Concurrency

Atomic Variables

# Advanced Concurrency

- Atomic variables
- Basic operations
- atomic_flag
- Specializations
- atomic_ref

# Basic Operations

- We can divide atomic operations into three categories:
- Read operations;
- Write operations;
- Read-Modify-Write (RMW) operations.
- And for the most general atomic types std::atomic<T>:
- Read operations are .load(memory_order=std::memory_order_seq_cst);
- And an operator T, which can only use seq_cst as order.
- Write operations are .store(T newObj, memory_order=std::memory_order_seq_cst);Not T& or std::atomic<T>&!
- And operator=(T), which can only use seq_cst as order and returns T newObj.
- Notice that atomic types are neither copyable nor moveable.

For methods of atomic operations, if it accepts memory order, then the default parameter is std::memory_order_seq_cst; if it doesn’t accept memory order, then it just uses std::memory_order_seq_cst. We’ll not repeat them in the following slides.

# Read-Modify-Write

- And we also need atomic RMW operations…
- This is not same as atomic read + atomic write, since two atomic operations are divisible.
- RMW operations are indivisible as a whole.
- For example: a++;
- Read: temp = a; Modify: temp++; Write: a = temp.
- If it’s divisible, two threads running Inc may still get value other than 200000.

- And RMW operations are:
- .exchange(T desired, memory_order) -> T: read the original value and write desired; then return the original value.
- Actually no modification, but read & write are atomic as a whole.

# Read-Modify-Write

- And a composite operation:
- .compare_exchange_strong(T& expected, T desired, memory_order success, memory_order failure) -> bool:
- Read op: read the value v, compare it with expected;
- Modify op: no modification;
- Write op: if equal (or called success), write back desired; otherwise (failure) write back nothing (but assign expected = v).
- Return value means success or not.
- So bit of strangely, its operation type depends on its read op:
- If failure, then it’s just a load operation;
- If success, then it’s RMW operation (and the whole operation is atomic).
- And thus, you can assign two memory order.

BTW such operations are generally called CAS operations (compare-and-swap / compare-and-set). CAS is the basic operation for lock-free data structures.

# Read-Modify-Write

- Since RMW operations involve both read and write, the memory order will constrain both of them.
- seq_cst: both read and write use sequential consistent model.
- relaxed: both read and write use relaxed model.
- But for acquire-release model, acquire and release are separate…
- So you can use std::memory_order::acq_rel!
- acq_rel: use acquire-release model, where read is acquire operation and write is release operation.
- acquire: read is acquire operation while write is relaxed.
- release: write is release operation while read is relaxed.
- And RMW ensures that it reads the newest value in MO, and writes the consequent result as the newest value in MO.

# Read-Modify-Write

- There also exists a one-memory-order overload:
- .compare_exchange_strong(T& expected, T desired, memory_order success) -> bool;
- Failure takes order from success, since RMW order includes read order.

- Notice that failure is not seq_cst by default.

But we notice that it’s normally a bad idea to use RMW operation to do spinlock, since write is special

# Read-Modify-Write

in cache coherence protocol (like MESI) and harms efficiency. Here it’s just an example.

- Take our previous example:

Can be rewritten as:

# Atomic Operations

- Note 1: atomic variables are bitwise-compared and bitwise- written.
1. A customized operator== doesn’t affect CAS operation;
2. Particularly, for floating points, bitwise-comparison is very misleading.
- For example, -0.0 and 0.0 are not bitwise-equal.

- When expected is -0.0, this CAS returns false; when it’s 0.0, it’s true.
3. To make bitwise-written reasonable, std::atomic<T> has constraint on T – T should be trivially copyable.

# Atomic Operations

- Note 2: padding bits are NOT compared since C++20.
- Before C++20, they’re compared.
- Formally, object representation v.s. value representation. Object Representation

0 1 2 3 4 5 6 7

char c padding float f

Value Representation

We’ll talk about memory layout in detail in Memory Management.

# Atomic Operations

- Reason: padding comparison may lead to astonishing false. If you really want to compare padding, you can manually pad as members.

- But, if it’s atomic union, when different types have different value representations (i.e. padding positions are not same), only shared padding parts will be omitted.

- NOTICE:
1. libc++ hasn’t implemented it; libstdc++ currently implements it as DR (i.e. since gcc13, no matter what standard you specify, only value representation is compared).
2. For union, no matter whether types have shared padding positions, MS- STL will only compare object representation (as of 2025/7). A simple test.

# Atomic Operations

- Note 3: there also exists .compare_exchange_weak(…), with completely same parameters as .compare_exchange_strong.
- The effects are also same, except that weak may fail spuriously.
- That is, it may report failure when it’s in fact equal; but when it reports success, then it’s definitely equal.
- So that in some platforms, it may be cheaper to use weak than strong.
- Normally we don’t want that spurious failure, so weak is usually used in a loop.

- So when the loop body is relatively cheap, weak can be beneficial to performance.
- But if you don’t use in loop or loop is very expensive, strong is expected.

# spinlock

- Though we can implement spinlock by acq_rel, it’s very inefficient.

- Normally we should rely on platform-dependent features.
- Like in x86, there are lots of idle instructions (PAUSE, UMWAIT, etc.) to reduce busy-wait overhead.
- And in OS layer, we can use lots of native utilities like futex on Linux, WaitOnAddress on Windows, etc.
- To maximize efficiency, C++20 introduces .wait() for atomics.

A brief but good article about idle instructions: 漫话Linux之“躺平”: IDLE 子系统

- .wait(T old, memory_order): block when .load(memory_order) is equal to old.
- Similar to condition variables:
1. You need to call .notify_one() and .notify_all() after modification to waken up waiting side;
- And pay attention to possible ABA problem.
2. It may spuriously wake up and do comparison, even if not notified.
- For example, again:

# spinlock*

- About how .wait() is implemented, FYI.
- Windows & MS-STL: by WaitOnAddress if supported in Windows SDK; otherwise by e.g. condition variable.
- Linux & libstdc++:

Memory order is contained in __vfn.

For article to introduce it, see Implementing C++20 atomic waiting in libstdc++ | Red Hat Developer.

Spin for __atomic_spin _count times

“Relax” first

false directly, so this part is jumped.

By e.g. PAUSE instruction.

By futex

# Lock-free?

- Finally, usually the reason we use atomic variables instead of lock is that they’re more efficient.
- But are atomic variables really lock-free?
- No, not necessary…
- C++ does NOT regulate that atomic variables should be lock-free.
- Common platforms will support small types like integers to be lock-free by atomic instructions in ISA;
- But if you use a very large struct, then no atomic instruction can do that!
- Or if your platform only supports very weak ISA, then even not all atomic integers are lock-free…
- Instead, C++ provides interface to check whether it’s lock-free.

For non-lock-free atomic types, you may need to link additional libraries; like in gcc and clang you need –latomic.

# Lock-free?

- A constexpr static boolean: std::atomic<T>::is_always_lock_free; only when on the current platform std::atomic<T> is definitely lock-free will it be true.
- A normal function: .is_lock_free(); some lock-free types may be only determined in runtime (e.g. when its address are over-aligned).
- C++20 adds some aliases that are guaranteed to be lock-free:

# atomic_flag

- Besides, C++ standard regulates a special type to be definitely lock-free: std::atomic_flag.
- It’s similar to std::atomic<bool>, but the latter is not regulated to be lock- free.
- And since its value is either true or false, methods are renamed directly.
- Read op:
- .test(memory_order), since C++20.
- Write op:
- .clear(memory_order): set to false.
- RMW op:
- .test_and_set(memory_order): set to true and return the previous test result.
- Spinlock:
- .wait(bool old, memory_order), .notify_one(), .notify_all(), since C++20.

# atomic_flag

- And for ctor:

Before C++20

# Advanced Concurrency

- Atomic variables
- Basic operations
- atomic_flag
- Specializations
- atomic_ref

# Specializations

- Some atomic types are specialized to provide more convenient methods; we list them here.
- Integers:

- Floating points (since C++20):

- And raw pointers.

# Specializations

- They just add common RMW operators and corresponding function overloads (to provide memory order).
- Operators: +=, -=, ++, --, &=, |=, ^=;
- But they do NOT return *this; except for postfix ++, they return the new value (i.e. the stored value).
- Functions: fetch_xxx, i.e. fetch_add/sub/and/or/xor(T, memory_order).
- And they return the original value.
- And another two functions since C++26: fetch_max/min(T, mo), which writes maximum/minimum value back.
- Floating points only provide +=, -=, add, sub;
- Pointers only provide +=, -=, ++, --, add, sub, max, min;

# Specializations

- Note 1: since C++20, there also exist specializations for std::shared_ptr and std::weak_ptr, and we’ll talk about them in Memory Management.

- Note 2: atomic pointers do NOT mean you access underlying objects atomically; they mean pointer themselves are atomic.
- And that’s why there are no operator* and operator-> for atomic pointers.
- Since C++20, std::atomic_ref is introduced for that atomic access.

# atomic_ref

- An example adjusted from C++20 the Complete Guide by Nicolai. M. Josuttis.

- Most of methods in std::atomic_ref<T> are same as std::atomic<T> as if operating on it directly.
- So not listed again.

# atomic_ref

1. To denote const reference, you can use std::atomic_ref<const T>; then write operations will be disabled.
- const std::atomic_ref<T> is shallow const; as reference itself is already const, this shallow const does nothing.
2. When an object is accessed by std::atomic_ref, you shouldn’t access it by normal reference and pointers to avoid data races.
- And of course, you need to ensure the lifetime of referenced object doesn’t end (i.e. not dangling reference).
- And different std::atomic_ref shouldn’t overlap. Formally:

# atomic_ref

3. And some unique members:
- Data members:
- static constexpr std::size_t required_alignment, the referenced object should align with required_alignment; otherwise UB.
- Methods:
- .address(): since C++26, returning pointer to the referenced object.
- copy ctor: reference the same object as another std::atomic_ref.
- But it’s not copy assignable.
4. Even if std::atomic<T> is lock free, std::atomic_ref<T> may not be lock free; their implementations are different.

# Advanced Concurrency

Advanced Memory Order

# Advanced Concurrency

- Advanced Memory Order
- Release Sequence
- Out-of-thin-air Problem
- Memory Model Conflict
- Fence

# Release Sequence

- Observe code below:

# Release Sequence

- Assuming that there is only one producer and one consumer, then it’s definitely correct.
- If producer is not ready, then consumer will wait;
- The first time idx > 0, it means that #2 read value from #1 and thus SW(#1, #2).
- And SB(#0, #1), SB(#2, #3), thus HB(#0, #3).
- That is, when the consumer extracts a value, it’s guaranteed that the producer has already stored it, which ensures correctness.
- And for following fetch_sub, since they’re performed in the same thread, SB makes it still correct.

- But, what about one producer + two consumers?

# Release Sequence

- For the consumer that first sees idx > 0, it’s still correct (as we analyzed before).
- But for the second consumer, #2reads value from write in #2. 2 1
- But write in #2is relaxed, so there is no SW(#2, #2); 1 12
- Thus, we cannot conclude HB(#0, #3)…
- That is, when the second consumer extracts a value, it’s NOT guaranteed that the producer has already stored it.
- To solve it, we can use acq_rel instead of acquire;
- But we’ve said that acquire only introduces one-way barrier, while acq_rel will introduce two-way barrier, which harms optimization.

# Release Sequence

- To overcome this counter-intuitive result, C++ introduces release sequence.

[intro.multithread]

[atomics]

- In other words, the release operation can be maintained through a continuous bunch of RMW operations (no matter what memory order they have), as long as there are no other new modifications kick in.
- In our example, this means SW(#1, #2) is thus guaranteed.

# Release Sequence

- This is slightly weaker than acq_rel.
- If we use acq_rel, we can conclude SW(#2, #2) and thus infer HB(#1, #2).
- But by release sequence, we can only conclude SW(#1, #2); there is no HB relationship between #2and #2. 1 2
- Take our previous example, again: Can we weaken this order to relaxed?

# Release Sequence

- Yes, #1 + #2 is a release sequence, and thus SW(#1, #4).
- With SB(#0, #1) and SB(#4, #5), we thus know HB(#0, #5), which means assert never fire.
- But code right is incorrect:
- Since there is no SW(#1, #2), thus we don’t know HB(#1, #6), and thus no HB(#0, #6).
- And we say that two non-atomic operations that have no HB relationship will cause data races.
- Thus, #6 is UB.
- If we use acq_rel here, then from SW(#1, #2) we know HB(#0, #6), then it’s correct.

# Release Sequence before C++20*

- This part is optional.
- Actually before C++20, release sequence can have more components:

- And C++20 weakens release sequence; but why do C++11 introduce it while C++20 delete it?
- Consider code right:

# Release Sequence before C++20*

- We know that to execute #5, #4 needs to be true, which means that #4 needs to load value from #3.
- But #3 is a relaxed store, thus there is no SW relationship, and thus we cannot infer HB(#1, #5).
- So here #5 has data races with #1.
- But intuitively, since SB(#2, #3) and #2 is release operation, it’s “natural” to think SW(#2, #4).
- Thus, C++11 regulates that following writes in the same thread are also part of release sequence.

[1, 2]
- However, this is not natural at all…

[1]: Common Compiler Optimisations are Invalid in the C11 Memory Model and what we can do about it | POPL’15, Viktor et.al. [2]: P0982R1: Weaken release sequences

# Release Sequence before C++20*

- We may introduce a new thread:
- If #6 kicks in between #2 and #3 in modification order of x…
- Then this release sequence is destroyed and suddenly #5 has data races with #1 again.
- This is counter-intuitive again and will make program buggy.
- That is, the relaxed order, which should not be engaged in any SW relationship, weirdly corrupts other HB relationship.

# Release Sequence before C++20*

- Two ways to solve that:
- [1], as an academic paper, proposes a very complex way to strengthen the memory model to make release sequence still valid;
- [2], as a C++ proposal, proposes to minimize changes and thus weaken release sequence by cancelling the first rule.

- This means even if there is no thread 3, code has data races (as we reason before that HB(#1, #5) doesn’t hold water).

[1]: Common Compiler Optimisations are Invalid in the C11 Memory Model and what we can do about it | POPL’15, Viktor et.al. [2]: P0982R1: Weaken release sequences

# Advanced Concurrency

- Advanced Memory Order
- Release Sequence
- Out-of-thin-air Problem
- Memory Model Conflict
- Fence

# Out-of-thin-air Problem

- We’ve said relaxed model can cause astonishing results:
- Here r1 == 42 && r2 == 42 is possible.
- Then what about code below?

- We only add two if without any new atomic operations, which should not affect any HB order, and theoretically r1 == 42 && r2 == 42 is still a valid solution.

# Out-of-thin-air Problem

- Reasoning process is same as we point out before:

- However, this outcome is contradictory with logical causality（因 果律）.

# Out-of-thin-air Problem

- The logical preconditions are as follows (→ means “requires”):
- #2 store happens → r1 == 42 → #1 loads 42 → #4 store happens → r2 == 42 → #3 loads 42 → #2 store happens.
- So the precondition to make #2 happen, is that #2 has already happened.
- This is logical fallacy, i.e. begging the question（循环论证，即通过假设结果正 确，推出结果正确。）
- Compared with our normal example:
- #2 store happen have NO precondition.

- Such logical fallacy is called “out-of–thin-air problem” in relaxed order; r1 == 42 && r2 == 42 comes from nowhere but it’s allowed by theoretical model.

# Out-of-thin-air Problem

- If we allow it to happen, a scary example will be right too:
- Initially x == 0 && y == 0, we can still get r1 == 42 && r2 == 42.
- Because we can “assume” that r1 loads 42, and then we find that x == 42 && y == 42 && r1 == 42 && r2 == 42 is a valid and consistent solution.
- Again, we beg the question…
- However, these problems are still under investigation in academy:
1. How can we describe out-of-thin-air problem in current model?
2. How can compilers detect out-of-thin-air problem?
- Currently we can only describe it by data dependency, which is almost not trackable in complex program.
3. How can we avoid out-of-thin-air problem in the most efficient way?

An Initial Study of Two Approaches to Eliminating Out-of-Thin-Air Results is a good academic survey for this.

- Of course, lots of academic work tries to solve them with different approaches…
- And before a widely-accepted model & description is proposed, C++ standard chooses the most conservative way to state it:

And since out-of-thin-air problem is not well- defined now, we only assert that no processor will do operations that violate causality.

# Advanced Concurrency

- Advanced Memory Order
- Release Sequence
- Out-of-thin-air Problem
- Memory Model Conflict
- Fence

# Memory Model Conflict

- We’ve said that sequential consistent model ensures total order, while acquire-release & relaxed model doesn’t.
- What if we mix their operations? [1, 2]
- For example:

- Initial value of x and y are both 0, can r1 == 1 && r2 == 3 && r3 == 0?

[1]: Repairing sequential consistency in C/C++11 | PLDI’17, Lahav et.al. [2]: P0668R5: Revising the C++ memory model

# Memory Model Conflict

- In seq_cst total order:
- To make r1 == 1 && r2 == 3, C needs to read y == 1 but D needs to read y == 3 and thus in total order C → E.
- If E → C, then D can never get 3.
- B is not seq_cst operation so B → C is not among total order.
- To make r3 == 0, F needs to read x == 0 and thus in total order F → A.
- And we know that SB restricts total order E → F.
- So in seq_cst model, such result just needs total order C → E → F → A.

- While in HB relationship…
- We first note that in SW relationship, seq_cst is equivalent to acq_rel.

# Memory Model Conflict

- We only know:
- SW(Init_x, F), SB(A, B), SB(C, D), SB(E, F).
- To make r1 == 1, C reads value from B and thus SW(B, C).
- Thus HB(A, B, C, D).
- To make r2 == 3, as long as HB(D, E) is not true.
- And there is no way to deduce it’s true, and thus it’s Okay.
- So in different angles of views, we seem to get contradictory results:
- In total order, C → E → F → A;
- In HB order, HB(A, B, C, D).
- Before C++20, it’s regulated HB order should be consistent with total order, i.e. r1 == 1 && r2 == 3 && r3 == 0 is impossible.

# Memory Model Conflict

- However, Power and ARM allow it (especially Power)…
- To maximize optimization, instead of fixing compilers, the C++20 standard is thus revised to allow such contradiction.
- Formally, only strongly-happens-before relationship should obey total order.

- In our example, SW(B, C) are not all seq_cst atomic operations, and thus SHB(A, B, C, D) is not true.
- We can only assert SHB(A, D), since SB(A, B) && HB(B, C) && SB(C, D); but since D is not seq_cst, SHB(A, D) is still not involved in total order.

# Happens-before Revision*

C++20C++26 Consume operations C++11Happens-beforeare deprecated.

Include (sameMerge Happens-before Happens-beforeif no consume operation) Include (sameSimply-Happens- if no consumeInclude Becomebefore operation)No Include Strongly-Happens-need Strongly-Happens-No need beforeto beforeto obey Strongly-Happens- obey before(like in Obey ObeyPower) Obey

Total order Total orderTotal order

What we teach is based on C++26; and since consume operations are never implemented, it can also be seen as based on C++20.

# Advanced Concurrency

- Advanced Memory Order
- Release Sequence
- Out-of-thin-air Problem
- Memory Model Conflict
- Fence

# Fence

- Sometimes we want to synchronize without an explicit atomic variable…
- And fence, as a global barrier, is for that!
- std::atomic_thread_fence(memory_order).
- It somehow imposes memory order globally:
- For a release fence, as if adding release order for following atomic writes.
- BUT the relationship is just SW from fence.
- For an acquire fence, as if adding acquire order for previous atomic reads.
- BUT the relationship is just SW from fence.
- Specifically:

# Fence

- For example:
- When #3 is true, it reads value from #2;
- And we say #3 is “as if” an acquire operation since it’s atomic read before acquire fence…
- So then SW relationship is established.
- And SW starts from fence…
- Thus it’s SW(#2, #4), not SW(#2, #3).
- So HB(#1, #5), then it’s safe for this read.

# Fence

- Is it still correct if we swap #4 and #5?
- No, since SW(#2, #4) doesn’t imply HB(#1, #5) then.
- So we can see that fence strengthens atomic operations globally， which also incurs higher overhead.
- Formally:

# Fence

#1
- Another example: #2
- If #6 is true, then #4 #3 reads value from #3;
- And #4 is atomic read before acquire fence, #3 is atomic write after release fence. #4
- So “as if” #3 is release operation, #4 is acquire operation.#5
- Thus SW relationship from fence is established.#6 #7
- Then SW(#2, #5), and thus HB(#1, #7), so #7 is safe to read.

# Advanced Concurrency

Coroutine

# Advanced Concurrency

- Coroutine
- Overview
- Basics
- Awaiter
- std::generator in Detail

# Coroutine

- We know that threads are competitive execution context.
- They won’t suspend until schedulers force them by interruption.

- But sometimes, instruction streams cooperate with each other.
- That is, they suspend and switch to other streams voluntarily.
- This is coroutine (cooperative routine).
- Coroutines define a set of suspension points;
- When execution streams reach a suspension point, they will transfer rights to other coroutines;
- Execution will be restored when other coroutines transfer rights back.
- Just like a state machine!

# Coroutine category

- And there are two ways to implement coroutine… *
- Stackful coroutines, or fiber（有栈协程 / 纤程）.
- Recap: thread model.
- Each thread has its own registers and stack; context switch saves old registers and loads new registers in kernel space.
- Similarly, stackful coroutines have their own registers and stack.
- And suspension point just saves & loads registers voluntarily in user space.
- Stackless coroutine（无栈协程）.
- The state is explicitly allocated in some space;
- Thus registers don’t represent its state and there is no need to save them.
- They will just occupy stack of current thread, instead of allocating a new stack.

*: Sometimes fiber refers to user-space threads, which is still competitive instead of cooperative.

# Coroutine

- A very naïve example (pseudocode).

- For stackful coroutine:
- coro is just Context;
- Resume: restore ins. pointer, restore stack pointer, restore registers.
- Suspend: set result, save ins. pointer, save stack pointer, save registers.

# Coroutine

- For stackless coroutine, we can rewrite without any assembly.
- Any variable with lifetime that spans over suspension point will be saved in context.
- It’s just like a normal function call, so it uses stack of caller.

- C++ adopts stackless coroutine for zero-overhead abstraction.
- But there also exists P0876 for fiber.

# Advanced Concurrency

- Coroutine
- Overview
- Basics
- Awaiter
- std::generator in Detail

Coroutine interface

# Coroutine

Suspension point

- To use coroutine in C++, we need to define a function with co_await, co_yield or co_return. co_return; can be omitted here.

- For caller, it gets coroutine interface from a coroutine.
- Coroutine interface needs to encapsulate underlying details, and provides APIs to manipulate coroutine.

Declare first to use in template parameter.

# Coroutine

- So how is a coroutine interface defined?
- It should have a std::coroutine_handle (defined in <coroutine>) as data member, which already represents high-level abstraction of a coroutine.
- Then it needs to define promise_type, which customizes the behavior of a coroutine.
- Roughly speaking, a coroutine will be translated to:

Copy coroutine params to coroutine frame first.

# Coroutine

- std::coroutine_handle exposes several operations:

Promise on the coroutine frame can be used to get the handle, and vice versa.

Equiv. to

# Coroutine

- And then, the function body of the coroutine is treated as below:

Let’s see what happens step by step…

# Coroutine

1. Here the coroutine will stop immediately, before execution of user-defined code.
2. If user-defined code throws, .unhandled_exception() will be called.
3. Execute when user-defined code co_return, here the coroutine will suspend at final point.
- Must be noexcept.
4. co_return; doesn’t return anything, thus promise needs to define return_void.

# Coroutine

- And finally, we need to expose APIs of handle in coroutine interface:

# Coroutine

1. Besides std::suspend_always, std::suspend_never is also sometimes used.
- co_await std::suspend_never{} means “do not suspend, continue to execute”;
- For example:

Immediate resume when creating the coroutine.

suspend_always suspend_never

# Coroutine

2. .final_suspend() should almost always suspend, and thus std::suspend_never is seldom used here.
- If final_suspend doesn’t suspend, then coroutine frame will be destroyed automatically and return to caller; thus it’s not due to memory leak.
- Reason 1: coroutine_handle and promise will be illegal after deallocation of coroutine frame…
- Thus, it’s UB to use .done() to check whether coroutine can be resumed after that, and UB to access data member of the promise.
- Also, it’s UB to .destroy() due to double free, making it hard to wrap in coroutine interface.
- Just like a dangling pointer.
- Reason 2: if lifetime of a coroutine is nested within caller, then heap allocation elision optimization (HALO) may be performed by compiler.
- And suspension at final point makes it much easier for compiler to judge HALO.

# Coroutine

3. Even if you suspend at .final_suspend(), it’s UB to call .resume() again.
- And resume or destroy a non-suspended coroutine (like call .resume() in another thread) is also UB.
4. If you want to co_return some value, you can define void return_value.

Coroutine interface

Return type of return_void and return_value must be void.

# Coroutine

- Note 1: you can define overloads for return_value, or even make it template, so that you can co_return different types.
- For example:

- Note 2: you cannot define both return_void and return_value.
- And co_return; is only interpreted as p.return_void(), so defining return_value without accepting parameters is also useless.
- Note 3: you cannot use return in coroutine.

[dcl.fct.def.coroutine], [stmt.return.coroutine].

# Coroutine

5. Common practices to implement .unhandled_exception():
- Ignore the exception;
- Process the exception locally; for example:

rethrow the exception in catch block.

- End or abort the program (e.g., by std::terminate());
- Storing the exception for later use with std::current_exception().

# Coroutine

6. The promise_type is actually looked up by std::coroutine_ traits<R, Args…>.
- Like in our example, it’s found by std::coroutine_traits<Task, int>.
- By default, it just uses R::promise_type, and thus we need to define promise_type inside Task.
7. Besides default initialization, promise can also be constructed with parameters of coroutine.
- Like in our example,

- This may be useful for allocator parameter (covered in the future).

In a rare case, you can specialize std::coroutine_traits to e.g. pass coroutine interface by parameter and define promise_type elsewhere. Still, you need to keep return type of .get_return_object() to be convertible to return type of coroutine. We don’t cover it here since it’s not very useful.

# Coroutine

8. If you don’t want to throw exception when failing to allocate coroutine frame, you can define static CoroutineInterface get_return_object_on_allocation_failure() additionally;
- Then new will use std::nothrow_t…

- For example:

# Coroutine

9. Finally details about std::coroutine_handle…
- It’s very similar to a raw pointer to the coroutine frame.
- Default ctor / ctor by nullptr: the handle doesn’t represent a coroutine;
- Copy & Move ctor / assignment: shallow copy; copies refer to the same coroutine;
- Address-related:

- Comparable and hashable.
- And finally, std::coroutine_handle<void> is specialized to represent any coroutine frame.
- As it erases promise type, .promise() and .from_promise() are not defined.
- It’s useful for general execution, since .resume() exists.

# Coroutine

- But we only implement suspension now…
- How to implement the pseudocode example at the beginning?
- To yield a value, you need:
- co_yield xxx; in coroutine;
- Awaiter yield_value(Type xxx) in promise_type.
- And a common Awaiter is just std::suspend_always, which means co_yield will always suspend the coroutine.
- Similar to .return_value(), you can overload / make it template.
- And interface:

# Coroutine

- To conclude, promise_type needs five mandatory operations:
- .get_return_object() -> CoroutineInterface;
- .initial_suspend() -> Awaiter, to specify initial behavior before executing user code;
- .final_suspend() -> Awaiter, to specify final behavior after executing user code;
- .return_void/return_value() -> void, to support co_return;
- .unhandled_exception() -> void, to handle exceptions thrown in coroutine;
- And some optional operations:
- .yield_value(), to support co_yield;
- static get_return_object_on_allocation_failure()-> CoroutineInterface, to provide default coroutine interface object instead of throwing exception when coroutine frame allocation fails.

Minimal example of a coroutine interface.

You can also add e.g. .yield_value(), and add APIs in coroutine interface (after definition of promise_type, since using its members needs complete definition).

# Coroutine

- Exercise: implement a very simple std::generator.
- We’ve briefly taught std::generator in Lecture 4 Ranges & Algorithms…
- So just add .yield_value() and iterator!
- As end iterator cannot be determined, just use std::default_sentinel_t.

Trivial parts…

Here we:
1. Store the value inside promise and do forward assignment in .yield_value();
2. Return reference to it in operator*;

The actual specification and implementation of std::generator is more complex and will be covered later.

# Coroutine Lifetime Concern

- Finally a very important note: parameters of coroutine should rarely be reference.
- For example:
- For a normal function, when will the temporary bound to s destructed?
- Temporaries are destructed when the statement ends, i.e. after ;.
- Normally, as use of parameter ends when function returns, passing temporary is completely fine.
- But for coroutine…

# Coroutine Lifetime Concern

- Coroutine in C++ is still a function!
- Compilers just “transform” your coroutine code to another normal function, like our switch + goto code.
- So for loop in coroutine, after the first suspension…
- s will be dangling reference and future .resume() is wrong.

- This problem is even more subtle for lambda coroutine.
- Is code below correct?

# Coroutine Lifetime Concern

- No!
- We know that lambda captures have same lifetime as lambda itself.
- And getCoro() creates a temporary lambda, and passing 3 generates a coroutine interface task.
- And the temporary lambda is destructed, so captures are invalid and thus coroutine code is also UB.
- The correct way is quite strange:
- So to conclude, unless you’ve considered carefully:
1. Do NOT use reference type in coroutine parameters;
2. Do NOT use stateful lambda coroutine, or generally pay special attention to public member coroutine.

# Advanced Concurrency

- Coroutine
- Overview
- Basics
- Awaiter
- Symmetric Transfer
- std::generator in Detail

# Awaiter

- Previously we just use two predefined types to await, either std::suspend_always or std::suspend_never.
- But how can they suspend / continue execution?
- Generally speaking, awaiter can be defined to manipulate behaviors of later coroutine.
- For awaiter, three APIs needs to be defined:
- .await_ready() -> bool: false means later execution is not ready, i.e. suspend; true means not to suspend;
- .await_suspend(CoroutineHandle continuation) -> void: called when suspended, determine what should be done for later execution;
- .await_resume() -> T: when the current suspension resumes, what should be executed first before later execution. T means return value of co_await.

# Awaiter

- So actually, predefined ones are very easy to implement: suspend_never just returns true here.

Nothing needs to be done additionally. Just suspend.

Nothing needs to be handle.resume() will continuedone after resumption. execution here, including calling .await_resume() and destruction of the temporary awaiter.

# Awaiter

- For example, implement NaiveTaskCont that:
- When we await another task, we want next .Resume() to resume the new task.

And we can use linked list to go to The basic parts are completely same. the deepest task.

In co_await x; we call x.await_suspend(), and we need to set new task x as next level handle of current coroutine.

Each time we call .Resume(), we find the deepest task by linked list and resume it.

1. Test() is called, coroutine interface A is created and suspends at initial point;
2. Resume(), execute until co_await Test2();
- Test2() is called, new interface B is created and suspends at initial point;
- B.await_ready() returns false, meaning A needs to suspend;
- B.await_suspend(cont) attaches handle of B (this->coroHandle_) as next level of handle of A (cont).
3. Resume(), find the deepest task (B) and resume it, execute until suspend_always.
4. Resume(), execute until B reaches final point.
5. Resume(), execute until A reaches final point.
- B.await_resume() is called first;
- B is destructed, as it’s a temporary returned by Test2().
- Later code like println is then executed.

# Awaiter

- Besides defining awaiter APIs directly, there also exist two other ways to do co_await:
1. By operator co_await, which delegates co_await to return value.
- Quite like operator->!
- This can isolate awaiter APIs and make them not exposed to users in coroutine interface.

# Awaiter

2. By .await_transform() in promise, which will intercept almost all co_await x; and substitute as co_await promise.await_transform(x).

- You can also add constraint or overloads;
- To disable co_await in your coroutine, you can use =delete.
- Like in std::generator:
- Order is: for co_await x, if p.await_transform() fails, then x.operator co_await; if x.operator co_await fails, then find awaiter APIs of x.

# Awaiter

- Look back to co_yield x…
- It’s essentially equiv. to co_await promise.yield_value(x).
- Particularly, initial await, final await and co_yield will bypass .await_transform().
- For example, if we want to do a back-and-forth conversion:

# Awaiter

- Return value of .await_resume() is treated as return value of co_await, and thus is also return value of co_yield.

- Not that useful, just a naïve example.

# Advanced Concurrency

- Coroutine
- Overview
- Basics
- Awaiter
- Symmetric Transfer
- std::generator in Detail

# Symmetric Transfer

- All programs before are sequentially executed, though we can suspend and resume them.
- In many cases, coroutine will cooperate with multithreading, which will then execute different streams in parallel.
- Consider a common case:
- We have a ServiceA and a ServiceB, and the result of ServiceA will be sent to ServiceB.
- For example, ServiceA is network receive, and ServiceB is data processing.
- For synchronous code, we may just write:

# Symmetric Transfer

- More generally, we may use callback:
- So actually, we just treat DoServiceB as later execution of DoServiceA.
- Of course, we can use coroutine to do so automatically:

When the current task ends, resume original task automatically.

When a task is awaited, attach the execution stream and resume new task.

A task can only be attached once, and then it will be executed to end. Use rvalue ref qualifier to notice users.

The whole process is like:
1. DoServiceB() creates a coroutine interface B;
2. .Start() resumes it until co_await;
3. DoServiceA() creates a coroutine interface A;
4. awaiter.await_suspend() resumes A;
5. A reaches co_return;
6. final_awaiter.await_suspend() resumes B;
7. B reaches co_return;
8. B.resume() returns -> final_awaiter.await_suspend() returns -> A.resume() returns -> awaiter.await_suspend() returns.
9. Task.Start() returns, all instructions are fully executed.

# Symmetric Transfer

- Before we continue, a small discussion:
- If seems not necessary to use coroutine at all…
- However, we can use awaiter to transfer execution to another thread easily:

- Then all instruction streams since DoServiceAAsync() will execute on another thread in the thread pool, and the main thread can go on without waiting.
- By coroutine, it’s very easy to switch executions on different schedulers.

However, you should pay special attention to synchronization problem before & after transfer. Particularly, mutex cannot be locked on thread A and unlocked on thread B (and such code is easy to implement in coroutine!).

# Symmetric Transfer

- However, such stream concatenation has a severe problem…
- Consider another simple code segment:

- When count is large enough, it will cause stack overflow.

- Let’s analyze the whole process again…
- For each loop, the stack occupation is like:
- completes_synchronously() creates a new coroutine; coroutine frame generation will occupy stack temporarily.
- When this function returns, stack will be popped then.

# Symmetric Transfer

- Other parts will occupy stack indeed:
1. awaiter.await_suspend();
2. newTaskHandle.resume();
3. And the new task will execute its own code, until co_return. Stackless coroutine will occupy caller’s stack, just like a function call.
4. final_awaiter.await_suspend();
5. lastTaskHandle.resume();
6. And the loop continues to execute, which again occupies stack.
- Only when loop reaches its end will all of stack be popped.
- However, we expect the loop to execute without stack push…
- Core reason: recursive call of .resume.

# Symmetric Transfer

- To solve this problem, coroutine also supports symmetric transfer.
- That is, .await_suspend() can return a coroutine handle, meaning that continue to execute this handle.
- Compared with calling .resume() inside .await_suspend() directly, symmetric transfer will pop the stack first.
- In previous analysis, this means:
- 2, 3 and 4 will be popped before 5;
- 5, 6 and next 1 will be popped before next 2.
- This utilizes tail-call optimization elegantly, but we don’t dig into that.

If you’re interested, I strongly recommend you to read C++ Coroutines: Understanding Symmetric Transfer by Lewis Baker.

And notice that gcc doesn’t implement optimization here (see bugzilla) so symmetric transfer will still cause stack overflow.

# Symmetric Transfer

No if like
- But still a small bug… code above.
- When lastLevelHandle is nullptr, then we are resuming nullptr and it’s thus UB.
- To return the caller like void, we can use std::noop_coroutine():

- It returns std::coroutine_handle<std::noop_coroutine_promise>, another specialized handle that will return to caller directly when resumed.

# Symmetric Transfer

- And finally, when .await_suspend() has only two options…
- Either suspends, or continue to execute;
- Then you can also use bool-returning variant; For example:

- Note 1: You can rewrite it with handle-returning variant, but since bool-returning variant is simpler, compilers are likely to do optimization better.

# Symmetric Transfer

- Note 2: compared with in .await_ready(), coroutine already suspends in .await_suspend().
- If you return true in .await_ready(), .await_resume() won’t be called;
- But if you return false in .await_suspend(), .await_resume() will be called.
- Note 3: all awaiter APIs and promise_type APIs can be constexpr, just like normal methods.
- However, coroutine cannot be constexpr currently.
- This may be enhanced by P3367R4: constexpr coroutines.

# Advanced Concurrency

- Coroutine
- Overview
- Basics
- Awaiter
- std::generator in Detail

# Generator

- Let’s see what std::generator really does after learning coroutine!
1. The promise type will store pointer to yielded value, instead of storing value and doing assignment.
- First, let’s see T is value type or rvalue reference.

- We know that it’s transformed to:

# Generator

- So it seems that we store pointer to temporary…
- We know that it’s dangerous to do so in normal function.
- A similar example from homework of Lecture 5 Lifetime & Type Safety:

- But here it’s safe!
- All temporaries destruct when the whole statement (i.e. until ;) ends.
- So when will co_await end?
- When coroutine resumes again, after calling .await_resume()!

# Generator

- Thus, the temporary persists until next resumption, which makes it safe to store pointer and dereference.
- Compared with move assignment, this saves one move.
- Then iterator just returns rvalue reference:

- Wait, what about yielding lvalue?
- Here we only accept rvalue reference…
- If user yields lvalue, we should not move it like rvalue in operator*.

# Generator

- std::generator adds an overload for const T&, which copies it to awaiter and then stores pointer.
- The iterator operator* still returns rvalue reference, but refers to the copy.

# Generator

- For std::generator<T>, where T is lvalue reference…
- Then just store pointer and return as corresponding reference.
- So to conclude, T just means how to reference the object:

# Generator

- Besides, generator also supports to specify value type + reference type by std::generator<Ref, Value>.
- So actually std::generator<Ref> just sets Value as void, and deduces real value type from Ref.

- For example, if you want operator* to return value type:
- But promise still stores pointer, and accepts const& in .yield_value().

# Generator

- It can also be used for proxy reference type.
- Like std::vector<bool>::reference as a bit proxy.

But to be honest std::generator<Ref, Value> is rarely used.

std::generator also accepts allocator as the third type parameter; covered in the next lecture.

# Generator

- Finally, std::generator also supports recursive yield.
- That is, it delegates resumption to another generator; when delegator finally ends, it continues to execute itself.
- To distinguish yield and delegation, it needs stdr::elements_of as tag.
- For example, to do inorder traversal of a binary tree:

Actually you can delegate to any range, which is equivalent to loop and yield its elements. But currently it may have some defeats and will be submitted as DR.

# Generator

- Note: .unhandled_exception() will propagate exception level by level, so you can catch it at any level.
- For example:

If you don’t catch it here, it will propagate to caller.

This is implemented by storing exception_ptr and checking in .await_resume() returned by delegation.

# Summary

- Memory order basics•Out-of-thin-air problem
- Happens-before, Synchronizes-•Strongly-happens-before with• Fence
- Sequential Consistent
- Coroutine
- Acquire-Release
- Stackful & stackless coroutine;
- Relaxed
- Basics: promise_type;
- Atomic variables•Awaiterand symmetric transfer;
- Read, Write, RMW, spinlock• std::generator
- Specializations, atomic_flag, atomic_ref
- Advanced memory order
- Release Sequence

# Next lecture…

- We’ll talk about memory management…
- Smart pointers;
- Allocator;
- PMR;
- …