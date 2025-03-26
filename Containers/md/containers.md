# Containers

## Foreword

- Some containers may have methods that **`<algorithm>`** has provided, but they are more efficient (otherwise why bother?)
- Before all these things, we may introduce two special integral type aliases defined in **`<cstddef>`**:
  - **`std::size_t`**: the type of **`sizeof()`**; this means the size of object cannot exceed the representable value of **`std::size_t`**
    - So, the maximum size of array is within **`std::size_t`**, so index is also within it.
    - Thus, in **containers**, all **`.size()`**-like thing will return **`std::size_t`**, and all **`operator[]`**-like thing for **array** will accept **`std::size_t`**.
    - It’s **`unsigned`**, and the signed version **`ssize_t`** is **not standard**.
    - Since **C++23**, you can use **`z`** as literal suffix for **`signed std::size_t`**, and **`zu`** as literal suffix for **`std::size_t`**.
  - **`std::ptrdiff_t`**: the return type of subtracting two pointers(**signed**)
  - They exist because they may be different on different platforms.
    - E.g. for **x86 & x64**, size_t is often respectively **32-bit/64-bit**.
    - **`ptrdiff_t`** is needed because on some old platforms, you need segments to represent the **array**, and pointer can only operate address in a segment

## Iterators

### Base

As its name, iterators are an abstraction of way to iterate over the containers and many other iterable things

- There are 6 kinds of iterators:
These iterators don’t have inheritance hierarchy, though the requirements are exactly more and more strict
  - **Input/Output iterator**:
    - for output, you can only do **`*it = val, it++, ++it and it1 = it2`**, provides **write-only** access
    - for input, you can also use **`==, != and ->`**, provides **read-only** access.
    These two iterators are **not for containers**, but for e.g. algorithm requirement of iterator and other things.
  - **Forward iterator**: same as input iterator, and can also be copied or default constructed.
    This is the **weakest** iterator for containers (e.g. **single linked list**).
  - **Bidirectional iterator**: same as forward iterator, and can also do **`--it, it--`**.
    - Can go in bi-direction one by one, e.g. for **double linked list**, **map**
  - **Random access iterator**: same as bidirectional iterator, and can also **`+/-/+=/-=/[]`** with an **integer** and be compared by **`</>/<=/>=`** (But for generality, we usually use **`!=`** in loop).
    - E.g. **deque**; This is almost equivalent to pointers in operations.
  - **Contiguous iterator** (since **C++17**): same as random access iterator, with an additional guarantee that memory occupied by iterators is contiguous.
    - E.g. **vector**, **string**.
- Since **C++20**, they become **concept**, so that if your iterators don’t match some requirements, the error information is more readable.
  - They are applied in algorithms for ranges; We’ll cover them later!
- **IMPORTANT**: Iterators are as **unsafe** as pointers.
  - They can be invalid, e.g. exceed bound.
  - Even if they’re from different containers, they may be mixed up!
  - Some may be checked by high iterator debug level
