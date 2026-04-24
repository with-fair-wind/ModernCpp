内存管理 Memory Management

## 现代C++基础

## Modern C++ Basics

Jiaming Liang, undergraduate from Peking University

### • Low-level Memory Management

### • Smart Pointers

### • Allocators

- PMR

- The real structure of memory is quite complex…

Credit: Jie Zhang @ PKU Arch.

- However, OS has abstracted them as virtual memory by page table, so in most cases users can view memory as a large contiguous array.
- When such abstraction causes performance bottleneck, programmers need to dig into that further;
- C++ also has some utilities to solve some common problems.

# Memory Management

Low-level Memory Management

# Memory Management

- Low-level Memory Management
- Object layout
- operator new/delete in detail

# Object layout

- Object will occupy a contiguous segment of memory that:
- Starts at some address that matches some alignment;
- And ends at some address that matches some size.
- A complete object may have many subobjects as members or elements (e.g. array or class).
- sizeof reflects size of the type when it forms a complete object, which is always >0.
- For example:
- In most cases, subobjects just occupy memory in the same way:

# Object layout

- However, some subobjects as class member can have 0 size…
- Formally called “potentially-overlapping objects”.
1. For a class, if it fulfills:
- No non-static data members;
- No virtual methods or virtual base class;
- It’s a base class. Then it’s allowed to have 0 size.

Moreover, it’s forced to have 0 size if
- The derived class is a standard-layout class.
- Also called “Empty Base (Class) Optimization” (EBO/EBCO).

# Object layout

- So now we can understand static_cast / reinterpret_cast…

In lecture 5, Lifetime & Type Safety.

FYI, this can be checked by std::is_pointer_interconvertible_with_class and std::is_pointer_interconvertible_base_of since C++20.

# Object layout

- Empty base will be collapsed so conversion is safe.

: Strictly speaking, it should be “similar types”, e.g. adding

# Object layoutcv-qualifiers is allowed. See[conv.qual]for details.

2: Except forpotentially non-unique objectlike string literals.

2. Since C++20, for a member subobject that is marked with attribute [[no_unique_address]], it’s allowed to have 0 size.
- Particularly, msvc will ignore this attribute for backward compatibility; instead, it respects [[msvc::no_unique_address]].
- For example: In gcc/msvc/clang, sizeof(Y) == 4.

- Note: C++ regulates two objects of the same typemust have distinct addresses. All three compilers make
- For example:sizeof(Y) == 2.

# Object layout

- Theoretically, this can be optimized as sizeof(W) == 2; however, all three compilers make sizeof(W) == 3.
- And again, we can understand in standard layout…

# Object layout

- Now EBCO doesn’t guarantee to happen:

- In ABI, base class may be put first;
- As subject of base class must be distinguished from the first member, then base class may be not really “empty”. *
- And a non-empty base leads to non-standard-layout.

*: there may be some defects in current definitions. See SO question.

# Layout Compatible*

- This part is optional.
- Finally we fix our claim before:

- Rigorously, when types have common initial sequence, it’s legal to access out of lifetime:

# Layout Compatible*

- Formally, we say two types are layout compatible if:
- Naïve cases:
- They are of the same type, ignoring cv qualifier; or,
- They are enumerations with the same underlying integer type.
- Otherwise,
1. They are both standard-layout; and,
2. Their common initial sequence covers all members.
- where common initial sequence means the longest sequence of non-static data members and bit-fields in declaration order that:
1. corresponding entities are layout-compatible; and,
2. corresponding entities have the same alignment requirements; and,
3. either both entities are bit-fields with the same width or neither is a bit- field.

# Layout Compatible*

- For example:A and B are layout-compatible.

*
- Since C++20, you can use std::is_layout_compatibleand std::is_corresponding_member to check it.

*: Strictly speaking, std::is_layout_compatible will tolerate non-struct-type, while the standard only regulates struct-type.

# Alignment

- To maximize efficiency, data should be aligned properly.
- For example, on some platform:

- In C++, it can be checked by alignof(T);
- Platform-dependent, return std::size_t, quite like sizeof.

*Or using type traits std::alignment_of.

# Alignment

- When wrapping data in class, every object will be aligned to its own alignment, leading to padding.
- For example:

Each element in C array should be suitably aligned, thus sizeof(X) must be multiple of alignof(X).

# Alignment

- Naturally, all scalar types will have alignment not greater than alignof(std::max_align_t) (in <cstddef>).
- And allocation will align to this alignment by default.
- However, sometimes you may want over-aligned data.
- Then you can use alignas(N) to make alignment N.
- Ignored when N == 0, compile error if N is not power of 2.
- For example, to match OpenGL uniform layout:

These three members are all aligned to 16.

# Alignment

- Note 1: you can also use alignas(T) to have alignment same as T.
- Note 2: when using multiple alignas, the largest one will be selected.
- So our previous code segment can be rewritten:

- Note 3: you can do pack expansion in alignas, which is same as alignas(arg1) alignas(arg2) … alignas(argN).
- i.e. select the largest alignment among N arguments.

# Alignment

- Note 4: over-align only: if alignas is weaker than its natural alignment (i.e. alignment without alignas), compile error.
- Some compilers will ignore or only warn.
- Note 5: alignment is NOT part of the type, so you cannot alias it in using or typedef. Attributes are added after struct.

- Note 6: function parameter and exception parameter are not allowed to use alignas.

# False Sharing

- Practical example: false sharing
- From abstraction, when different threads operate on different data, parallelism will be maximized since no lock is needed.

- However, due to limitation of computer architecture, such abstraction is wrong…
- Cache on different processors has to obey coherence protocol like MESI.
- To put it simply, when write happens on a cache line, it’ll inform other processors whose cache also own this line to make it invalid.
- And invalid line needs to be reloaded, leading to inefficiency.

# False Sharing

Cache line ofCache line of Processor 1.Processor 2.

… Memory

arr

Illustrative animation for false sharing. (Details may vary for different architectures)

# False Sharing

arr[0]++

…

arr

# False Sharing

Invalid

…

arr

# False Sharing

arr[1]++

…

arr

# False Sharing

Load from other processors to read

…

arr

# False Sharing

Perform write

…

arr

# False Sharing

Invalid

…

arr

# False Sharing

- So when writes in different threads are on the same cache line, every write will happen exclusively as if having a lock.
- This leads to false parallelism, degrading the performance.
- Solution: make threads access data on different cache lines!
- C++17 provides constant std::hardware_destructive_interference_size in <new>.
- For example:

# False Sharing

- On the other hand, for a single thread, we hope accessed data to lie on the same cache line to minimize pollution.
- For example:

Improperly aligned, use two cache lines.

Properly aligned, use single cache line.

- To force data to lie on the same cache line, we can align the head as cache line head.
- C++17 thus introduces std::hardware_constructive_interference_size for that.

# False Sharing

- For example:

- Question: aren’t std::hardware_destructive_interference_size and std::hardware_constructive_interference_size just same as cache line size?
- Why do we need two constants to represent them?

# False Sharing

- Reason: in some architecture, destructive interference will be larger than a cache line…
- For example, on Intel Sandy Bridge processor, it will do adjacent-line prefetching.
- So when loading a cache line, the next cache line may or may not be substituted, leading hardware_destructive_interference_size == 128 while hardware_constructive_interference_size == 64.

# Supplementary

- Note 1: there exist several utilities for alignment in <memory>.
1. std::align:

- Assuming that we have a space that starts from ptr and has size space;
- Now we want to allocate an object with size and alignment on the space;
- Assuming that it can be allocated on new_ptr on new_space (i.e. suitably aligned).
- So std::align just modifies ptr to new_ptr, space to new_space, and returns new_ptr.
- If space is too small, then nothing happens and nullptr is returned.

# Supplementary

- For example:

# Supplementary

2. To maximize optimization, you can inform compiler that a pointer is aligned by std::assume_aligned<N>(ptr) since C++20.
- It’s UB if it’s not aligned to N, quite like [[assume]].
- Since C++26 you can also add std::is_sufficiently_aligned<N>(ptr) to check precondition in debug mode.
- For example:

# Supplementary

- Note 2: since C++17, you can use trait std::has_unique_object_representations to check if same value representations of two objects lead to the same object representation.
- For example, for float, two NaN are not distinguishable but may have different bits, so the trait returns false.
- Particularly, for a struct, when there are padding bytes, then it definitely returns false since they are not part of value of struct.

- This trait can be used to check whether it’s correct for a type to be hashed as a byte array.

# Memory Management

- Low-level Memory Management
- Object layout
- operator new/delete in detail

# new/delete

- To combine allocation and construction, C++ uses new- expression to substitute malloc in C.
- Roughly speaking, it calls two different functions:
- Allocation new, which only allocates memory (quite like malloc).
- Placement new, i.e. construct the object on memory.
- And similarly, delete-expression has two parts:
- Destructor, i.e. destruct the object on memory.
- Deallocation delete, which only deallocates memory (quite like free).
- C++ allows you to override (replace) the allocation new by operator new and the deallocation delete by operator delete.

# new/delete

- Thus the most basic versions like malloc and free are as below:
- You can override them in global scope (i.e. namespace ::).

- Besides, you can provide class-specific allocation & deallocation:

- Which is preferred than global override, and isn’t required to be noexcept.
- They are always static function, even if you don’t add keyword static.

NOTE: before P3107 (DR23), std::println will use std::string and thus may need to use operator new/delete, causing infinite recursion. MS-STL has implemented this DR so it’s fine to do so.

Here we don’t add static, but it’s still static function. You can’t use this here.

# new/delete

- However, C++ also allows you to delete basePtr, which will call virtual dtor.
- Ideally, it should call Derived::operator delete instead of Base::operator delete…
- Let’s try it!

# new/delete

- The output is like:
- So Derived::operator delete is called, quite like virtual dtor!
- But operator delete is static! How? *
- Reason: compiler will generate a “deleting destructor”.
- That is, it will generate a new virtual function:
- For a normal object, just use normal dtor;
- For delete ptr, it will call this new function.

- With virtual dispatch, we can extract more information from the type to improve malloc-like version!

*: This is implementation-defined; here we use method of Itanium ABI. Seethis blogfor details.

# new/delete

- Before going on, let’s do some recap…
- In ICS, we’ve written a very basic allocation strategy:
- Allocate memory block that’s slightly larger than requested, then store block size and pointer to next block alongside it.
- However, many metadata will never change after allocation, which will pollute cache line.
- So in modern memory allocators, it’s much more complicated…
- Roughly speaking, a common way is to split memory into bins indexed by approximate size.
- And metadata may record more info:

Adopted from mimalloc.

# Sized-delete

- And in C++, we can almost always know object type exactly…
- So we can know size of object!
- To facilitate optimization, C++ introduces size-aware delete (also called sized-delete).
- Global sized delete is provided in C++14, while class-specific one is from C++11.

- For a naïve example:

# Sized-delete

- Note 1: some practical example:
- Like in jemalloc:

- Note 2: compilers are free to choose sized-delete or normal delete.
- So, programmer should always provide both of them.
- For gcc/msvc/clang (clang needs –fsized-deallocation flag):
- For global override, it will prefer sized version when it exists.
- For class-specific override, it will prefer normal delete when it exists (since you can easily know its size by sizeof).

# Aligned new/delete

- But these overloads don’t specify alignment…
- Before C++17, over-aligned types may be not correctly handled (normally compiler warning in e.g. -Wall).

- Since C++17, alignment-aware new/delete are introduced.
- Here std::align_val_t is scoped enumeration as tag.
- For type whose alignment requirement exceeds macro __STDCPP_DEFAULT_ NEW_ALIGNMENT__, alignment-aware overloads are preferred.
- Of course, you can override them too.

- For class-specific ones:

C11/C++17 provides aligned_alloc similarly; but MS-STL doesn’t provide aligned_alloc since Windows doesn’t provide ability to allocate aligned memory and thus must over-allocate and align manually. Therefore, it cannot be freed correctly by free; instead, _aligned_alloc and _aligned_free should be used.

# new/delete

- Note 1: all new-overloads has nothrow variants:

- Note 2: essentially, new-expression new(args…) Type{…} will call operator new(size(, align), args…).
- The arguments before args… are usually determined by compilers, while the latter are specified by users.

* : Placement is abused in the context of new/delete; here it just means to provide additional parameters in new(…),

# new/deletewhich includes nothrow variant and placement-new variant.

- That’s why you can use:
- new(std::nothrow) Type, for there exists nothrow variant.
- new(ptr) Type, for there exists placement-new variant.
- But they can’t be overridden by users.

- More generally, you can provide customized arguments for user- * defined placementallocation new:
- Class-specific ones also exist, omitted here.

- Plus placement deallocation delete:
- Each user-defined new must has a matching user-defined delete; when constructor throws, new memory will be freed by corresponding delete.
- Otherwise memory leak! For example (omit sized-delete):

Only failed new- expression will call corresponding placement delete! Prevent memory leak.

# new/delete

- And similarly, for nothrow new, you need to customize placement delete…

- Finally, if a placement allocation corresponds to a non-placement deallocation, then compile error.

This is sized delete.

# new/delete

- Note 3: for the default thrown operator new, it will call new handler when allocation fails.
- As if:

The default new handler is just nullptr, so it will throw std::bad_alloc directly.

# new/delete

- You can customize it by std::set_new_handler(…) in <new> (thread-safe), and the handler is expected to:
1. Make more memory available (so after calling handler, allocation retry may succeed);
2. Terminate the program (e.g. by std::terminate);
3. Throw exception derived from std::bad_alloc, or std::set_new_handler( nullptr).
- Return value: previous handler.
- For example:

# new/delete

- Note 4: C++20 introduces class-specific destroying-delete.

- Which will be preferred over all other overloads.
- delete-expression will call destroying-delete directly, without calling dtor.
- That is, it’s duty of the destroying-delete to call dtor.
- Array doesn’t have this overload.
- Note 5: it should be thread-safe to call operator new/delete.

# new/delete in coroutine

- Special example: control allocation of coroutine.
- Coroutine will allocate its state/frame by new;
- C++ allows you to customize operator new/delete of promise_type to control such allocation!
- It’s specially treated so not exactly same as normal class-specific allocation/deallocation.
- Class-specific ones need lots of overloads to cover every possible case;
- But promise_type only needs to define a few for compiler to choose!
- For new, it only needs:
- For delete, it only needs:
- When this overload doesn’t exist, it needs:

# new/delete in coroutine

Memory resource will be covered in later sections.
- For example:

# new/delete in coroutine

- Note 1: when defining get_return_object_on_allocation_failure, you should make operator new act as if nothrow instead of defining nothrow variant.
- For example:

- Note 2: compilers are allowed to omit your operator new/delete when performing HALO.
- So theoretically, one way to ensure HALO is to only declare operator new/delete without definition, so allocating on heap will lead to link error.
- Note 3: operator new is allowed to accept parameters of coroutine.
- A naïve example:
- And it’s preferred if exist.

# new/delete in coroutine

- Take std::generator as an example:

For member coroutine.
- Implementation may then allocate more bytes than size, then put allocator on additional space.
- The delete can extract allocator from the frame to do deallocation.

# new/delete in coroutine

- Use it by passing additional parameters.

# new/delete

- Final note: in shared library, global override of operator new/delete should be paid special attention.
- Reason: if each shared library has its own override, it may be unclear which one is used.
- For example, when A is loaded, its memory is allocated by its operator new;
- And B is loaded, then operator delete is replaced;
- And when A frees its memory, it uses operator delete of B, causing unknown results.
- The behaviors are totally implementation-defined.
- In static library, this will cause link error for symbol conflict.

# Memory Management

Smart Pointers

# Overview

- Similar to every RAII type, smart pointer can be used to prevent memory leak by releasing resource in dtor.

In Lecture 7 Error Handling, Section “Exception safety”.

- Generally, smart pointers represent kind of “ownership”.
- std::unique_ptr represents unique ownership; only one can destroy it.
- std::shared_ptr represents shared ownership; the last holder will destroy it.
- …
- So when someone doesn’t need ownership, it’s enough to use raw pointer. Do NOT abuse smart pointer.
- We’ll talk more about this later.

# Memory Management

- Smart Pointers
- unique_ptr
- indirect and polymorphic (C++26)
- shared_ptr All of them are
- weak_ptr defined in <memory>.
- Adaptors

# unique_ptr

- As it’s easy and we’ve taught it briefly, we first list APIs and add some important notes. Move-only, i.e. have move ctor & assignment, no copy ctor & assignment.

Give up ownership and set nullptr; Return original pointer. Destroy original resource; replace it with parameter ptr (by default nullptr).

# unique_ptr

- Note 1: we know that unique_ptr can also handle array by specifying T[].
- Such partial specialization is slightly different:
1. Instead of having operator->/*, it has operator[] as if accessing an array.
2. Of course, if will call delete[] by default.
- This also makes it impossible to do CTAD for ambiguity; given a pointer, it cannot determine whether it’s unique_ptr<T> or unique_ptr<T[]>.
- Note 2: if you want to denote const T* (i.e. point to immutable object), you should use unique_ptr<const T> instead of const unique_ptr<T>.

# unique_ptr

- Note 3: more generally, unique_ptr can handle any resource by customized deleter.
- A deleter needs to define:
1. A type named pointer (if it doesn’t exist, T* will be used);
- Which is stored and managed inside unique_ptr.
2. operator() to do destroy operation (e.g. delete in std::default_delete<T>, and delete[] in specialization std::default_delete<T[]>).
- For example:

This unique_ptr manages memory.

This unique_ptr manages OpenGL descriptor. Remove GPU resources related to descriptor buffer.

# unique_ptr

- Another example?

1. unique_ptr now stores unsigned int instead of a pointer;
2. operator() will be called in dtor.

- But it cannot use some methods (like .release()), since it will try to assign nullptr as empty resource…
- To make it fully compatible, you should make pointer fulfill NullablePointer.
- And support operator*/-> additionally if needs to use these operator*/-> of unique_ptr.

# unique_ptr*

- For example: Quite complex…

If you only need some general RAII wrapper, you can write it yourself instead of using std::unique_ptr Then all methods of with customized deleter std::unique_ptr are defined (especially if pointer is with e.g. pointer class. We some customized class). don’t dig into that and just check cppreference.

# unique_ptr

- Note 4: dtor will actually check empty state first.
- So if your deleter cannot process nullptr correctly, it’s okay.

- Note 5: you can also use std::make_unique<T>(Args…) to do construction.

Initialized by () instead of {}
- For array, only size can be specified and all elements are value-initialized.
- E.g. here all elements are 0.

# unique_ptr

- Before C++17, std::make_unique can prevent subtle memory leak caused by indeterministic evaluation order.
- For example, order may be new int{1} -> new int{2} -> construct unique_ptr;
- So when new int{2} throws, memory leak may still happen.
- Since C++17, we know that function parameters are evaluated in a non- overlapping way, so this problem won’t happen at all.
- And sometimes it may be unnecessary to do value initialization…
- For example, we’ll read binary data from network, so we don’t need to assign all elements 0.
- Then you can use std::make_unique_for_overwrite since C++20.
- The essential difference is just new int() v.s. new int.

# unique_ptr

- Back to our previous claim…
- “When someone doesn’t need ownership, it’s enough to use raw pointer. Do NOT abuse smart pointer.”

- Use function parameter as example…
- Raw pointer (T*)
- std::unique_ptr<T>
- std::unique_ptr<T>&
- std::unique_ptr<T>&&
- const std::unique_ptr<T>& which one to choose?

# unique_ptr

1. In most cases, raw pointer is enough…
- Precondition: except for nullptr, pointed object is valid.
- And function read / write the object by pointer.
- By contrast, it should rarely manipulate lifetime like by delete.
- Observation instead of ownership.
- For example:

- This function does NOT care about where ptr comes from (stack, heap, or static segment, etc.); it only observes.

# unique_ptr

2. By contrast, std::unique_ptr<T> means to hold the ownership;
- So the caller will give up its ownership.
- And the function may transfer ownership to others, or just let it destroy automatically when exiting function.

std::unique_ptr<T>&& is quite
- For example: similar, except that when you don’t move inside function, the caller won’t release its ownership.

While by taking value as parameter, ownership will be definitely released.

# unique_ptr

3. For std::unique_ptr<T>&…
- Generally, for a ref parameter U&, what we want to do is to modify the parameter itself.
- So similarly, std::unique_ptr<T>& means to modify caller’s unique_ptr.
- For example, set a new object ownership:

- Of course, it can read & write content, and transfer ownership to others;
- But if it only needs to undertake these duty, it’s unnecessary to use &.
- Which is quite like T* v.s. T**.
4. Finally, for const std::unique_ptr<T>&, since its read-only features are same as T*, this form is useless.

# Memory Management

- Smart Pointers
- unique_ptr
- indirect and polymorphic (C++26)
- shared_ptr
- weak_ptr
- Adaptors

# PImpl

- Before going on, we first introduce a technique called pointer to implementation idiom (pimpl).
- When programming in multiple files, for a class:
- We need to expose in header files:
- Data members;
- Declaration of methods and (non-inline) static variables;
- And hide in source files:
- Definition of methods and static variables.
- So when we:
1. Want to add / remove methods;
2. Want to modify data members, no matter change type or add new ones.
- We have to code in header files, and all related files need to re-compile…

# PImpl

- But ideally, when public members remain the same, what is exposed to users is unchanged; other files should not re-compile.
- PImpl tries to solve this problem.
- Class in header only owns a pointer to its members, and expose public interface.
- For example, a naïve example of normal implementation:

When we add float cacheSum_, cacheProd_; and remove InnerProd_, then other parts need to re-compile…

When we add float cacheSum_, cacheProd_; and remove InnerProd_, then only this source file will be modified. Thus we only

# PImpl

need to re-compile a single file, and relink.

- If we use PImpl:

# PImpl

- We notice that pimpl has many variants.
- For example, previous code doesn’t manage inheritance well.
1. The derived class needs to allocate new space for its own members, causing memory fragmentation;
2. You cannot change protected APIs freely, as it will change header file.
- We can then improve like:
1. Write BaseImpl class into another header, which is only included for inheritance (thus re-compilation is restricted in limited files only);
2. The DerivedImpl class then inherits BaseImpl;
3. Base exposes pointer to BaseImpl as protected;
4. And finally, Derived inherits Base, and assigns new’ed DerivedImpl to Base in ctor; when it needs to use DerivedImpl, just static_cast it.
- Also, if you want to use interface of Class in ClassImpl, you can also add a Class* in ClassImpl* to point back. The above two variants are adopted in QT
- etc… and renamed as “d-pointer & q-pointer”.

# PImpl

- Pros:
- Reduce build time significantly when project is large.
- Maintain binary compatibility.
- Normally, when data members change, object layout will also change;
- Then new-version header files + old-version shared library will crash; users have to re-link.
- However, by pimpl, users just pass the pointer, and how to process it is completely determined by library.
- As long as users don’t use new public APIs, they don’t need to re-link.
- Completely hide members that is originally be in public header files, so no privacy concerns.

- Of course, everything comes with a cost…

# PImpl

- Cons:
- Initialization overhead: need an additional dynamic allocation;
- Runtime overhead: all member access need one more indirect addressing;
- Cannot inline simple methods, since header files don’t know members;
- Cannot utilize default special member functions (e.g. default copy ctor, default dtor, etc.);
- Const incorrectness: const Class object has ClassImpl* const instead of const ClassImpl*, so const methods in Class can access non-const methods in ClassImpl.
- Which then needs additional attention to maintain correctness.

# PImpl

- We can notice that we are managing pointer manually…
- It seems very proper to use std::unique_ptr!
- But when we compile, it fails…

- Reason: it’s UB to delete incomplete type that has a non-trivial dtor.
- So std::default_delete enhances safety, which will emit error directly inside operator().
- And default dtor is inline inside class, which is thus equivalent to call operator() when type is incomplete.
- Solution: write default definition in source file!

In header file:

# PImpl

In source file:
- For example:

*
- Move ctor and move assignment are quite similar.
- Sometimes default move may be not our expectation, as it breaks abstraction of pimpl.
- It just points to implementation, so it should perform value semantics (i.e. all operations should happen in the underlying object).
- For example, for copy ones, we’ll write like:

*: strictly speaking, movectorofunique_ptrdoesn’t require complete type. However, C++ regulates thatctormay calldtorof subobjects (see [class.base.init]), so all compilers reject inline =default.

# PImpl

- Similarly, for move:
- You can call underlying move ctor & assignment if you want.
- Then moved-from interface points to a moved-from implementation, instead of getting nullptr.

- Though easier to implement compared with raw pointer, std::unique_ptr is still kind of inconvenient.
- You need to reimplement many methods, like copy, comparison, etc.
- As default ones will copy / compare /… pointers, which is pointer-semantics instead of value-semantics.
- And const-correctness is still under concern.

# std::indirect

- Since C++26, we can use std::indirect to solve it!
- It’s a value-semantic std::unique_ptr, i.e. major operations just call methods of the underlying object.
- Copy ctor & assignment;
- Comparison;
- Hash.
- And some special methods:
- swap: swap the pointer;
- Move ctor: transfer the pointer.
- Thus, the stored pointer of the moved-from object will be nullptr, which can be checked by .valueless_after_move().
- Move assignment: swap pointers, and destroy resource of the other.

# std::indirect

- For ctor:

Construct T by forwarded v or args or initializer list + args.

- Default ctor value-initializes the underlying object instead of owning nullptr.
- Every ctor has an allocator-aware variant; we’ll talk about allocator later.

# std::indirect

- And finally you can also use operator->/* to access.
- All methods will maintain const correctness, e.g. here const std::indirect<T> will access by const T*.
- For pimpl, it’s then very easy to implement basic operations:
- Just =default all of them in source file.

# std::indirect

- We notice that the real effects are slightly different if allocators of two std::indirect are unequal.
- For example, for move ctor std::indirect<T> a = std::move(b):
- When they have “equal” allocators, then a just takes pointer of b;
- But when they have “unequal” allocators, it will be like:
- a uses its allocator to allocate memory;
- Construct T with std::move(*b).
- We’ll cover them later…

# std::polymorphic

- Finally, std::indirect<T> can only handle T, though it stores T*.
- std::polymorphic<Base> is to correctly handle inheritance!
- You can store any Derived object inside it.

Here U must be same as or publicly derived from T. Arguments are used to construct U too.

# std::polymorphic

- Copy ctor: construct Derived object, where Derived is same as the underlying copied object.
- It will NOT slice, i.e. it doesn’t construct Base for std::polymorphic<Base>.
- Copy assignment: copy-and-swap, still to prevent slicing problem.
- For two std::polymorphic<Base>, assuming the underlying objects are Derived1 and Derived2, it will store pointer to a copy of Derived2.
- Move ctor / assignment: same as std::indirect, by taking pointer and swap-and-destroy.
- Dtor: it will call dtor of Derived directly, even if dtor of Base is not virtual.
- Since types may vary, other methods are limited:

# Memory Management

- Smart Pointers
- unique_ptr
- indirect and polymorphic (C++26)
- shared_ptr
- weak_ptr
- Adaptors

# shared_ptr

- unique_ptr represents unique ownership to a resource.
- Which is thus only moveable.
- Sometimes, there exist resources that are shared by many…
- Everyone has the right to destroy it, and will destroy it if it’s the last holder.
- Then we can use std::shared_ptr.
- For example:

You should NOT construct two shared_ptr with the same raw pointer to share ownership;

Instead, use copy ctor!

# shared_ptr

- Essentially, it maintains an atomic reference counter;
- Just like what we state in stop token handling.
- So unlike unique_ptr that merely stores original pointer, shared_ptr stores pointer to a control block.
- An example control block: *
- Ctor: assign counter as 1;
- Copy ctor: set blockPtr as another.blockPtr, increase counter;
- Move ctor: set blockPtr as another.blockPtr, set another.blockPtr as nullptr.
- Assignment: copy-and-swap idiom;
- Dtor: decrease counter; when counter reaches 0, destroy the resource.

*: initial counter value is slightly more complex and will be covered later.

# shared_ptr

- For ctor:

Y* must be convertible to T*.

Question: unlike unique_ptr<T, Deleter>, shared_ptr only has a single template parameter <T>; How can it initialize with deleter and allocator?

# shared_ptr

- By type erasure! (covered in Lecture 12 Advanced Template)
- For example:

Of course, here you can utilize EBO to compress Deleter.

- So for some ctor, even if you pass nullptr, it still needs to allocate.

# shared_ptr

- Besides ctor, you can also use make_shared to do construction.
- Or allocate_shared if you want an allocator.

- Unlike assigning pointer like PlainBlock, it stores object inside control block directly.
- It only needs to allocate once, so:
1. Reduce runtime allocation overhead;
2. Prevent memory fragmentation.
- Thus: in usual cases, prefer make_shared and allocate_shared over ctor to get new shared_ptr.

# shared_ptr

- Note 1: since C++17, shared_ptr also supports array semantics * correctly.

- And since C++20, make_shared and allocate_shared also add related overloads:

Value-initialized for each element; When T is unbounded, e.g. std::shared_ptr<int[]>.Each element is copy-initialized withu.

When T is bounded, e.g. std::shared_ptr<int[5]>.

* : Before it may succeed to compile, but it’s incorrect. See StackOverflow for details.

# shared_ptr

- Note 2: since C++20, for_overwrite variants are also added.

- Note 3: some newly-added type aliases since C++17:

# Count sharing

- Sometimes, two shared_ptr share the same ownership, though they’re not convertible.
- For example:

- PartialWork only needs to use a, but it still has ownership (i.e. needs to keep the resource valid).
- To solve that, shared_ptr introduces so-called aliasing ctor.

# Count sharing

We have a practical example for LockFreeStack in homework.
- For example:

- Then it shares the same control block with ptr;
- Note that when reference count reaches 0, the original object is destroyed.
- Particularly, operator<=> for shared_ptr are compared against stored pointer;
- So aliased pointer may be unequal to the original pointer (i.e. here ptr.get() != &(ptr->a), assuming int* and Resource* are comparable).
- If you want owner-based comparison, you should use .owner_xx.
- In owner-based meaning, aliased pointer are always equal to the original pointer (as they share the same ownership).
- Practically it just compares address of control block.

# Count sharing

- For example:

The first two will be true + false; the last two will be two false.

# Count sharing

- Note 1: it’s wrapped as functor too:
- However, it can only compare two pointer with same types;
- But ownership may be shared among different types (like previous shared_ptr<int> and shared_ptr<Resource>).
- So since C++17, template operator() is added by std::owner_less<void>.
- Which is quite like transparent operator.

# Count sharing

- Note 2: since C++26, owner-based equality and hash are also added.

Member Functors: methods:

- But owner_hash and owner_equal are not template at all; they use template method operator() directly.

- Note 3: aliasing ctor adds a rvalue overload in C++20.
- which will transfer pointer directly like move ctor.

# Count sharing

- Note 4: if you want to share ownership with cast pointer, you can use std::xx_pointer_cast:
- Equiv. to cast raw pointer, and then call aliasing ctor.

# Self-sharing

- In some cases, a class may need to keep “ownership of itself” inside a member method.
- For example:Simplified example of boost::asio::tcp_connection ::async_xx().

- We need code like: But now when Work returns, conn will be destructed so code in new thread is UB.

# Self-sharing

- Two factors to make it correct:
1. TcpConnection cannot be normally constructed; users should only be able to get its shared_ptr.
- Which is quite easy; just hide all ctors as private and expose them as static shared_ptr<TcpConnection> Create(Args…).
2. In AsyncRead, you must get copy of such shared_ptr.
- However, you cannot write shared_ptr<TcpConnection>{this};
- We’ve said that it’s wrong to construct two shared_ptr with the same raw pointer to share ownership.
- To solve this problem, you can let T inherits from std::enable_shared_from_this<T>.

- For example:

1. Now users can only get shared_ptr to TcpConnection;
2. To share ownership inside member method, use the inherited .share_from_this().

Note that enable_shared_from_this is implemented by std::weak_ptr (covered later), and also has .weak_from_this() since C++17.

*: Time-of-check To Time-of-Use, meaning that when you use the value, it’s already different from the value you checked. (so that precondition of code later may not hold anymore).

# shared_ptr

- Finally just list APIs:
1. Ctor & assignment also accept std::unique_ptr&&, which transfer ownership to itself.
2. .reset() has variants to accept deleter and allocator, which is equivalent to construct-and-swap.
3. .use_count() normally uses relaxed load; it’s usually not reliable in multi- * threading env for TOC/TOUproblem.

4. std::get_deleter() will return nullptr if the shared_ptr doesn’t has one; otherwise return pointer to deleter. (Non-member)

Address of object it refers to v.s.

# shared_ptr*

Control block it refers to (control block may store nullptr)
- A not-that-important note…
- (A) shared_ptr.get() == nullptr (or !operator bool) isn’t equivalent to (B) shared_ptr.use_count() == 0 (or called “empty shared_ptr”).
- A && B: only these two ctors.
- A & !B: other ctors that assign nullptr;

- B & !A: aliasing ctor that accepts empty source shared_ptr, while aliased pointer isn’t nullptr.

# Memory Management

- Smart Pointers
- unique_ptr
- indirect and polymorphic (C++26)
- shared_ptr
- weak_ptr
- Adaptors

# weak_ptr

- Instead of ownership, sometimes we only need to observe…
- That is, we don’t force a resource to exist;
- Instead, when we use it, we first try to share the ownership;
- If the resource doesn’t exist, that’s fine; we can process it.
- Otherwise we get its ownership and just use it.
- weak_ptr is just for this!
- It has a weak reference to shared_ptr;
- That is, even if there is some weak reference, resource of shared_ptr can be freed as long as there is no shared reference.
- When you want to use the resource, use .lock() to get the shared_ptr;
- It may return a null pointer, which needs special process;
- And it may return a valid shared_ptr, which then holds the ownership so that you can safely use the resource.

# weak_ptr

- For example:

In concurrent environment, some resources may be used by many threads; instead of loading again and again, we can prepare a cache.

When nobody uses it, cache will release the resource immediately.

Converted toConstructed shared_ptror assigned when used.with shared_ptr.

*Note that this function may still be thread- unsafe unless iterator access is thread-safe.

# weak_ptr

- Essentially, control block of shared_ptr also maintains a weak counter for weak_ptr.
- When shared count reaches 0 (or called “expired”), object will be destroyed;
- After that, all .lock() of weak_ptr will return null shared_ptr.
- When weak count also reaches 0, the control block will be destroyed.
- weak_ptr also stores pointer to the control block.

- Note 1: Weak reference arises problem for make_shared…
- make_shared allocates all memory in a single control block;
- And only when weak count reaches 0, the control block will be freed.
- So the memory occupied by object will persist even if shared count reaches 0.

# weak_ptr

- So in the case that:
1. Object memory is very large;
2. It may be referred by weak_ptr;
3. You want to release memory in time;
- It’s improper to use make_shared.
- Note 2: enable_shared_from_this just has a weak_ptr as data member.Question: why not
- Ctor of shared_ptr will detect and assign.have ashared_ptr?

# weak_ptr

- Note 3: weak_ptr can also be used to break cyclic reference of shared_ptr.
- For example, when you implement a list:
- Normally we will allocate an empty node and concatenate two ends.
- The list just holds a shared_ptr to the empty node.
- However, when the list is destructed, nodes are not released…
- Each node is referred by its previous and next nodes, so all .use_count() are 2.
- This generates unreachable memory, causing memory leak…
- In real scenarios, cyclic reference will be more subtle and hard to detect.
- And if you really need such cyclic reference, you should insert weak_ptr.

# weak_ptr

- Note 4: finally list APIs.
1. Constructed by either shared_ptr<Y> or weak_ptr<Y>, where Y* is implicitly convertible to T*.
2. No .get() because it’s only safe to access by converting to shared_ptr and .get().

3. .use_count() is still shared count.

4. Though ctor of shared_ptr can accept weak_ptr, it will throw bad_weak_ptr if weak_ptr is expired. So you should always use .lock().

# atomic smart pointer

- Finally, concurrent access of shared_ptr itself causes data races.
- For example:

- Reason: shared_ptr holds pointer to atomic counters (i.e. control block with two atomic counters); but the pointer itself is non-atomic.
- For raw pointer, we have specialization std::atomic<T*>;
- Since C++20, std::atomic<shared_ptr/weak_ptr> is added.

# atomic smart pointer

- Its usage are also quite similar to std::atomic<T*>.
- But no specialized methods like .fetch_add()/operator+=.

- Note 1: this doesn’t mean that the underlying object is atomically accessed.
- Just like std::atomic<T*> doesn’t mean pointer can access T in parallel.
- Instead, you should use either shared_ptr<atomic<T>>, or by atomic_ref.
- Just like std::atomic<T>*.

# atomic smart pointer

- For example:

# atomic smart pointer

- Note 2: though atomic<shared_ptr> doesn’t exist before C++20, you can use global methods for shared_ptr (no weak_ptr).
- For example:

- However, as it’s hard for functions to store states, these methods are likely to have worse performance than specialized class.
- For example, std::atomic<shared_ptr> can store a std::atomic_flag as data member to implement, but functions are hard to do so.
- Thus removed in C++26.

# Memory Management

- Smart Pointers
- unique_ptr
- indirect and polymorphic (C++26)
- shared_ptr
- weak_ptr
- Adaptors

# Smart pointer adaptors

- In C APIs, sometimes we need to accept T**, meaning to set a pointer.
- It’ll be slightly clumsy to manage by smart pointers:

- C++23 adds small utilities to make it easier:

- Essentially, this function creates a temporary std::out_ptr_t<SmartPtr, RawPointer, Args…>.

# Smart pointer adaptors

- std::out_ptr_t is basically like:

1. it also has operator void**, which just static_cast from RawPtr*.

2. Actually it’s .reset(static_cast<SP> (rptr), …) as below:

# Smart pointer adaptors

- The additional arguments args are useful for std::shared_ptr.
- For example:

- As it’s normally used with foreign framework…
- When calling .reset(), you almost always need to pass deleter.
- So std::out_ptr_t will make compile fail for shared_ptr when sizeof…(Args) == 0.

- Besides, there is another adaptor std::inout_ptr_t.
- This is used for functions that first release the resource, and then do re- initialization (quite like freopen).

# Smart pointer adaptors

- Essentially like:

1. As the foreign function will release automatically, we shouldn’t call .reset(). (Thus impossible to use std::shared_ptr)

2. They also have support for raw pointer; std::out_ptr(rawPtr) and std::inout_ptr(rawPtr) is basically equiv. to &rawPtr or (void**)&rawPtr. (Do pay attention to possible leak)

Note: these adaptors should always be used by passing std::inout_ptr(…) or std::out_ptr(…) into function directly. Otherwise it’s easy to lead to e.g. dangling reference.

# Memory Management

Allocator

# Memory Management

- Allocator
- Basics
- PMR

# Allocator

- In standard library, most of types that need dynamic memory allocation will use allocator.
- Users can specify different allocators to control the behavior.
- Formally, allocator is a general concept that encapsulates strategies for allocation/deallocation.
- The standard library will extract properties of allocator by std::allocator_traits<Alloc>.
- Theoretically, allocator needs to regulate lots of alias for generalization;
- For example, what is pointer type for T?
- But std::allocator_traits provides many default values that are enough to use in most cases.
- Thus we will omit trivial parts.

# Allocator

- For example:

The only one that needs to be defined.

Default value is usually enough (e.g. T*, const T*, …)

# Allocator

- To support a minimal allocator Alloc<T>, you need:
1. Nested type value_type, e.g. by alias using value_type = T;
2. Member method allocate(elemNum) and deallocate(ptr, elemNum);
3. “Equality-comparable” and “Copyable”.2.
- For example:

1.

3. 2.

3.

# Allocator

- Then we can use it with std::vector!
- We can also easily write an “allocator-aware” container:

Note that a correct version should deallocate previous memory.

# Allocator

- We can apply allocator for std::list similarly:
- Well, not that similar…
- Does std::list really allocate a single int?
- No, it allocates a Node<int>, which cannot be handled by Alloc<int>.
- Instead, it should be handled by Alloc<Node<int>>.
- So, allocator supports “rebind”, meaning to transform Alloc<T> to Alloc<U>.

NOTICE that it’s library UB to use Container<T, Alloc<U>> before C++20, and compilation error since C++20. Here we enhance it.

# Allocator

- If we apply it in our vector implementation:

- Thus “copyable” means ability to convert Alloc<U> to Alloc<T>.

# Allocator

- Similarly, for our previous homework List implementation:

Rewrite it with allocator…

# Allocator

# Allocator-defined con/destruction

- Besides, allocator can also control construction & destruction.
- For example, allocators may pass additional parameters, so it’s not enough to just placement new directly…

- And similarly, allocator_traits will provide default version in its static methods:

- For construct, placement new by default;
- For destroy, dtor by default.

# Allocator

- So essentially we should manage them by allocator in code:

- However, there seems no guarantee for exception safety…
- When construction throws exception, then allocation needs to be reverted.
- So we may write a simple RAII wrapper ourselves:

# Allocator

- More generally, we may even need to revert all previous constructions:

- C++ provides uninitialized memory algorithms (defined in <memory>), but unfortunately they are allocator-unaware.
- i.e. construction just uses placement new, and destroy uses dtor.

# Allocator

Ranges-version is also provided since C++20.

# Allocator

- As if:

Otherwise it may throw in catch block, and elements afterwards won’t be destructed.

- For example: Allocator-aware version will be left as our homework.

# Allocator propagation

- Finally, what happens during container assignment?
- For example, previously we’ve stated what std::vector does normally during move assignment:
- Release the original memory and exchange three pointers.
- Where is allocator?
- So in essence, allocators define propagate_on_container_move_assignment (POCMA) to determine what should be done. So normally1. When POCMA::value is true, allocator will be copied to the assigned container; POCMA is
- The assigned container just releases memory by the original allocator, gets alias to the new allocator, and then exchange pointers. true_type or false_type.
2. When POCMA::value is false, the assigned container will keep its own allocator.
- Then allocators will be compared by equality;
- Equal allocators mean that their memory are inter-operatable, i.e. it’s correct to use allocator A to release memory allocated by allocator B.

# Allocator propagation

- So again two sub-cases for case 2: ① When two containers have equal allocators, then just same as normal ones.
- i.e. Release the original memory and exchange three pointers.
- No allocator copy. ② When two containers have unequal allocators, we cannot exchange memory pointer directly.
- So every element is move-assigned individually.
- If memory isn’t enough, the original allocator should allocate more.
- And finally, allocator can define is_always_equal for optimization.
- Then allocator comparison can be done during compilation time.
- Normally only stateless allocator defines it, i.e. all allocators manage the same memory (e.g. std::allocator only manages global heap).

# Allocator propagation

- Similarly, we can define propagate_on_container_copy_assignment (POCCA) and propagate_on_container_swap (POCS) to determine behavior of copy assignment and swap.
- In most cases, POCMA, POCCA and POCS are of same value.
- Particularly, when POCS is false and allocators are unequal, direct swap is not well-defined (and standard containers mark it as UB).
- And we can extract these properties by allocator_traits too:

When allocator class is empty class, it doesn’t store any state and is regarded as always equal.

# Allocator propagation

- For example:

# Allocator propagation

- And finally, allocator will be informed when container is copy- constructed.
- Just define select_on_container_copy_construction (SOCCC), which should return new allocator from current allocator.
- And similarly, you can call it by allocator traits:
- By default just return copy of parameter.

- For example:

# Allocator propagation

- Particularly, this doesn’t apply for move ctor of container.
- Move ctor just moves allocator and exchanges pointers.

- And all ctors (including copy ctor and move ctor) have allocator-aware version, which will then copy the allocator and allocate by that.

Guess how move ctor with allocator is implemented.

# Allocator propagation

- Besides container, let’s use std::indirect as another example to see how we may treat propagation behaviors.
- Copy ctor: self allocator is either initialized with SOCCC of other’s allocator, or initialized with explicitly provided parameter a.
- The owned object is allocated and copy constructed from object of other.

Pseudo code, no e.g. exception safety.

# Allocator propagation

- Move ctor: self allocator is either moved from other’s allocator, or initialized with explicitly provided parameter a. other.valueless
1. When moved from other’s allocator, just take its pointer; _after_move() is
2. Otherwise if its allocator is equal to other’s allocator, just take its pointer; true after move.
3. Otherwise, the owned object is allocated and move constructed from object of other.

# Allocator propagation

- Copy assignment: will finally copy allocator when POCCA.
1. If self allocator is equal to other’s allocator and self is not valueless, copy- assign the underlying object.

2. Otherwise, two allocators are not equal, std::indirect chooses to “copy-and- swap” to process all cases uniformly.

# Allocator propagation

- Strictly speaking, to prevent unnecessary operations (e.g. temp needs to copy allocator), copy-and-swap is done manually.

# Allocator propagation

- Move assignment: basically equivalent to copy assignment, except that case 1 doesn’t move-assign, but just steal-and-destroy.

# Allocator propagation

- Swap: similarly, UB when POCS is false while allocators are unequal.

- To conclude: though allocators have some regulations (e.g. by POCMA), the specific propagation behaviors are determined by the class itself.
- E.g. move-assignment or steal-and-destroy, it’s up to your design!

# Allocator

- Note 1: C++23 adds an optional interface allocate_at_least for allocator.

- It should allocate space not less than n elements.
- This is useful for some allocation that tolerates more space; a typical example is vector reallocation.
- For example, our allocator can only allocate 16-byte chunks; but reallocation strategy wants three int (assuming 4-byte).
- Then we can return 16-byte chunk and such reallocation leads to four int.

The history is a little bit complex… void* is seemingly hard to handle in compilers’ constexpr evaluator and thus the template method construct_at is introduced. But actually the type information is always tracked and thus some conversion to void* in context is allowed since C++26, and thus placement new is also possible.

# Allocator

- Default behavior of allocator_traits is just return {allocate(alloc, n), n}, i.e. allocate requested size exactly.
- Note 2: placement new is constexpr since C++26, with some restrictions (mentioned in homework of Lecture 11).
- Before that, you can use template function std::construct_at since C++20.
- And that’s why the default behavior of allocator_traits::allocate is changed to std::construct_at in C++20.

# Allocator

Special note that std::construct_at cannot handle bounded array correctly beforeIssueThis [1] is to correctly return pointer 3436: std::construct_at should support arrays.to array.

- And symmetrically, std::destroy_at:

- Actually dtor can be constexpr, so this function is introduced in C++17 to act as if ptr->~auto(), since the type is deduced by template.

# Memory Management

- Allocator
- Basics
- PMR

# PMR

- We say that allocators are equal when their memory are inter- operatable.
- So a simple way to implement allocator is to use a static variable to denote its memory arena:

- However, then every arena leads to a unique allocator type…
- Oops, you have to write template code to support every allocator.
- And containers don’t support e.g. copy for different allocators.
- You have to write e.g. .assign(another.begin(), another.end()).
- Instead, we can use same allocator type, and control memory arena by polymorphic memory resource (PMR).

Defined in <memory_resource>. All components are defined in namespace

# PMRstd::pmr and thus not repeated afterwards.

- The allocator is just std::pmr::polymorphic_allocator, which controls memory represented by std::pmr::memory_resource.
- Just like:

You can change allocation strategy without changing the type of the allocator.

- memory_resource needs to expose interface below:

# PMR

- To define a customized resource, just inherit memory_resource and override private virtual methods:

- So essentially memory_resource just uses template method pattern.
- A very naïve example:

# PMR

- There are some predefined memory resources in standard library.
- Two naïve singleton resources that are returned by function:
1. null_memory_resource(): allocate nothing.

Singleton can be compared just by singleton address, no need to use dynamic_cast.

# PMR

2. new_delete_resource(): use operator new and delete.

- There exists “global default resource”, which can be got/set (thread-safely) by get_default_resource() and set_default_resource(ptr_to_mr) -> old_ptr.
- Quite like std::get/set_new_handler().
- The initial default resource is just new_delete_resource().

# PMR

- And three complex classes.
- They all have an “upstream” memory resource; when their own memory are exhausted, they will request more from the upstream.
- The default upstream is just global default resource.

1. Memory pool: synchronized_pool_resource/unsynchronized_pool_resource;
- It consists of a collection of chunk pools (bins), with each one owning many blocks of the same size.
- Just like what we mentioned in allocation strategy in sized-delete.
- Allocations are dispatched to the smallest bin that can accommodate required size.
- Deallocation returns memory to pool, and pool may or may not deallocate blocks further by releasing to the upstream memory.
- And unsynchronized_pool_resource assumes all allocations happen in the same thread (i.e. no possible races), which is likely to be faster than synchronized.

# PMR

- Just like:

Credit: Thanks for the memory (allocator) - Sticky Bits - Powered by Feabhas

# PMR

- You can configure the pool by struct pool_options, with two data members:
- std::size_t max_blocks_per_chunk: when a pool allocates new chunk from upstream memory, how many blocks are allowed to allocate at once.
- Implementation may use a smaller value than specified.
- std::size_t largest_required_pool_block: the maximum block size in pool; if the required size is larger, it will be allocated from upstream memory directly.
- Implementation may use a pass-through threshold.
- There exists an implementation-defined limit for each member, which will be used when some greater-than-limit value or 0 is provided.
- Non-inherited APIs are easy:

Equiv. to call upstream->deallocate().

Pool(const pool_options& opts, memory_resource* upstream),

# PMRand every parameter can be omitted (filled with default).

For example, here default ctor equiv. to { pool_options{}, get_default_resource() }.
- For example:

std::pmr::list<T> is alias of std::list<T,std::pmr::polymo rphic_allocator>.

Actually constructed by polymorphic_allocator, which can be further constructed by memory_resource* (implicitly).

Blocks of the chunk have been exhausted, which then allocate from upstream memory, leading to jumping interval. Though sizeof(Node) is 24 or 176, the block size is 32 & 256,By memory resources, we can arrange nodes to locate which is allocated as a whole.close to each other, making it much more cache-friendly.

# PMR

- 2. monotonic memory: monotonic_buffer_resource.
- Allocated memory will NEVER be deallocated; new allocations will fill buffer monotonically.
- Thus, allocation and deallocation are very fast.
- For example:Allocate (Ignore align)“Deallocate” 1 byte Actually nothing Allocation starts fromAllocatehappens byte 1 instead of 0.2 bytes

- You can provide an initial buffer (not managed by the class); when it’s exhausted, monotonic_buffer_resource will request from upstream, and
1. Like vector reallocation, requested size will increase exponentially.
2. All requested memory will be released (i.e. call upstream->deallocate()) when monotonic_buffer_resource is destructed or its .release() is called.
- .release() will reset buffer and size to initial buffer and size.

*Note that reallocation strategy differs for different implementation. And here we use MS-STL with Visual Studio Release mode. In Debug mode, MS-STL may allocate more memory for safety check.

# PMR

- For example:

Use stack as the initial buffer to make allocation very fast.

Allocated on the stack.

Since the buffer can at most hold 16 int, new buffer must be allocated from upstream. And though 13 ints can be accommodated inside the initial buffer, monotonic_buffer_resource will never go back so it’s still allocated on heap.

# PMR

- And note ctors of monotonic_buffer_resource:

(1~4): set current buffer to nullptr; when an allocation happens, allocate from upstream with initial_size as the initial buffer. Absent initial_size will be some implementation-defined value, and absent upstream will use default resource.

(5, 6): set with explicit buffer; absent upstream will use default resource.

# PMR

- Of course, you can combine these resources by setting upstream.
- Like our previous example in coroutine:

When allocation fails, don’t request more memory; instead throw an exception.

# PMR

- Note 1: lots of raw pointers are used in PMR and thus lifetime should be carefully managed.
1. You need to ensure upstream resource and initial buffer to be valid when they’re used.
2. When you call set_default_resource, the original resource shouldn’t be destroyed immediately since previously allocated container may use it.
3. When you destruct objects (or equivalently, call .release()) of monotonic_buffer_resource/(un)synchronized_pool_resource, you need to ensure nothing occupies its allocated memory.

*There exists DR to fix previous unnecessary dynamic_cast.

# PMR

- Note 2: these classes have some special properties:
- They are neither copyable nor moveable. *
- Different resource objects are always unequal.
- Though theoretically these memory resources may be inter-operatable, the standard library chooses to conservatively mark them as unequal.
- Note 3: PMR are “sticky”; it doesn’t POCMA, POCCA and POCS.
- And SOCCC just returns default constructed allocator (which uses default memory resource, instead of same resource as copied allocator).
- And remember that…

- This leads to the fact that it’s usually UB to swap two std::pmr::container that have unequal polymorphic_allocator.
- Particularly, different standard MRs are always unequal, so such code is UB:

# PMR

- Note 4: since C++20, polymorphic_allocator adds some utilities to avoid rebinding.

Equiv. to use resource to allocate bytes directly (instead of * sizeof(T)). Default alignment parameter is alignof(std::max_align_t) instead of alignof(T).

With template parameter <U>, equiv. to use polymorphic_allocator<U> with the same memory resource to do allocation.

With template parameter <U>, equiv. to use polymorphic_allocator<U> with the same memory resource to do allocation and construction. Exception-safe, i.e. when construction fails, space will be deallocated.

And C++20 also adds std::byte as default template type parameter for polymorphic_allocator. Personally I think it’s a design mistake, in line with Arthur O’Dwyer.

# Uses-allocator Construction

- Finally, PMR will perform uses-allocator construction to “down- propagate” allocators.
- That is, polymorphic_allocator::construct will try to pass itself to the constructed object.
- A naïve example:

- Here since std::pmr::string can accept polymorphic_allocator, allocator of the vector will be passed to newly constructed std::pmr::string.
- So string will allocate its memory from pool too.

# Uses-allocator Construction

- Essentially, whether allocator is propagated to new object of type T is judged by procedures below:
1. Judge whether T uses allocator by trait std::uses_allocator_v<T>.
- It’s true if T has nested type allocator_type.
- Or you can specialize it manually, like many other types:
2. If 1. is true, find the way to pass the allocator; ① First try leading-allocator convention, i.e. ctor is invocable by T(std::allocator_arg, alloc, args...). ② If it fails, then try trailing-allocator convention, i.e. ctor is invocable by T(args..., alloc). ③ Otherwise ill-formed.
- If we finally find the way, allocator is propagated by ctor.

# Uses-allocator Construction

- Since C++20, this process is wrapped as std::make_obj_using_allocator<T>(alloc, args…);
- Essentially:

- where std::make_from_tuple<T> in <tuple> is same as unpacking the tuple and construct T (quite like std::apply);
- And std::uses_allocator_construction_args<T> judges the calling convention, and return the corresponding tuple that satisfies the convention;
- Particularly, if std::uses_allocator_v<T> is false, then just ignore alloc and return tuple of reference to args.

# Uses-allocator Construction

- And if you want to placement new, you can use std:: uninitialized_construct_using_allocator<T>(p, alloc, args…).

- Thus construct of polymorphic_allocator can be easily implemented:

# Uses-allocator Construction

- Note 1: Pay attention to object that’s constructed outside the container, leading to possible inefficiency.
- For example, explain what happens in code below:

- .push_back accepts std::pmr::string, so it’s constructed outside.
- From const char* to string, default constructed allocator will be used if it’s not provided, meaning that the default memory resource will be used!
- As PMR is sticky and non-equal, element-wise move will happen (and in string it’s just equivalent to copy).
- Solution: either by .emplace_back, or construct with allocator explicitly.

# Uses-allocator Construction

- Note 2: std::uses_allocator_v<std::pair<…>> is false, but std::pair actually uses allocator for its two elements.
- Its ctors don’t fulfill calling conventions, so uses-allocator construction provides lots of overloads for it (each one corresponds to a ctor overload).

# Uses-allocator Construction

- By contrast, std::tuple uses leading-allocator convention, so it just specializes std::uses_allocator without other special treatments:

Defined in <scoped_allocator>

- Note 3: standard library also provides an allocator that “propagates” allocators to construct subobjects.
- To be exact, it collects allocators at once and dispatch them level by level.
- For example, for vector of vector of int…
- We need to use two allocators, for int and vector of int respectively.

- So normally, we need to write code like:

- What if we forget to pass b?

# Uses-allocator Construction

- Then the new vector uses default-constructed allocator, instead of b.

- If we use scoped allocator:

# Uses-allocator Construction

- For nested container, you need to use nested scoped allocator:

Scoped<Outer, Inners…>{ o, is… } will use o to allocate and construct subobject S; the construction passes Scoped<Inners…>{ is… } to do uses-allocator construction.

# Uses-allocator Construction

- If you stop to use scoped allocator, then uses-allocator construction cannot find the calling convention and thus forward without allocator.

Process:
1. LVVec uses LVAlloc (a) to construct, and passes ScopedAlloc1 (b+c) to VVec;

2. VVec accepts VAlloc, while ScopedAlloc1 inherits from VAlloc, thus VVec is constructed with b.
- c is discarded here!

3. VVec uses VAlloc (b) to construct; as it’s not scoped allocator, Vec is normally constructed with default Alloc.

# Summary

- Low-level memory• Allocators management•Interface and traits (POCMA,
- Object layout, alignmentPOCCA, POCS, SOCCC).
- operator new & delete•PMR
- Uses-allocator construction
- Smart Pointers
- unique_ptr
- pimpl
- indirect and polymorphic
- shared_ptr, weak_ptr
- atomic specialization
- out_ptr, inout_ptr

# Next lecture…

- Eventually, we’ve successfully finished all major topics!
- In the final lecture, we’ll quickly cover topics that are left out.
- File system;
- Chrono (time-related facilities);
- Math (including random-number generation);
- And finally, we’ll give a rough introduction to C++26 for future vision.