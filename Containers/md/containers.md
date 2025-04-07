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
- All containers can get their iterators by:
  - **`.begin(), .end()`**
  - **`.cbegin(), .cend()`**: read-only access.
- Except for forward-iterator containers (e.g. single linked list):
  - **`.rbegin(), .rend(), .crbegin(), .crend()`**: reversed iterator, i.e. iterate backwards
- You can also use global functions to get iterators:
  - E.g. **`std::begin(vec), std::end(vec)`**.
  - They are defined in any container header.
- Notice that **pointers are also iterators!**
  - So, for **array type** (not pointer type), e.g. **`int arr[5]`**, you can also use **`std::begin(), std::end()`**, etc.
    - We just get two pointers, e.g. here **`arr`** and **`arr + 5`**.
- There are also general methods of iterator operations, defined in **`<iterator>`**.
  - **`std::advance(InputIt& it, n)`**: **`it += n`**(for non-random, just increase by **`n`** times). **`n`** can be negative, but it should be bidirectional.
  - **`std::next(InputIt it, n = 1)`**: return **`it + n`**, not change original one.
  - **`std::prev(BidirIt it, n = 1)`**: return **`it - n`**, not change original one.
  - **`std::distance(InputIt it1, InputIt it2)`**: return **`it2 – it1`**(for non random access, just iterate **`it1`** until **`it1 == it2`**)

> They have **ranges-version**, e.g. **`std::ranges::begin`**; use them since **C++20**

### Traits

- Iterators provide some types to show their information:
  - **`value_type`**: The type of elements referred to.
  - **`difference_type`**: The type that can be used to represent the distance between elements (usually **`ptrdiff_t`**).
  - **`iterator_category`**: e.g. **`input_iterator_tag`**. **`continuous_iterator_tag`** is added since **C++20**.
    - It’s recommended to use **`iterator_concept`** when it’s available instead of category in **C++20**, which has more precise description of the iterator, especially for iterator of **C++20 ranges**.
  - **`pointer`**: the type of pointer to the referred element, only available in container iterators.
  - **`reference`**: the type of reference to the referred element, only available in container iterators.
  - You may use **`std::iterator_traits<IteratorType>::xxx`** (defined in **`<iterator>`**) to get them (absent ones will be **`void`**)
- Since C++20, you can directly get them by:
  - **`std::iter_value_t<IteratorType> / std::iter_reference_t<IteratorType> / std::iter_const_reference_t<IteratorType> / std::iter_difference_t<IteratorType>`**
  - **pointer** and **category** are not provided directly.

### Stream Iterator

Beyond iterators of containers, stream iterators and iterator adaptors are also provided in standard library.

- When reading from input stream/writing to output stream in a simple and fixed pattern, you can use **`std::istream_iterator<T>`** and **`std::ostream_iterator<T>`** (respectively input and output iterator).
- They are **initialized** by the stream, e.g. **`std::cin/std::cout`**.
- The **initialization of istream_iterator** will **cause the first read**, **`*`** will get the value, **`++`** will trigger the **next read**.
***Example:***

```cpp
    std::vector<int> vec(5);
    std::istream_iterator<int> it{std::cin};
    vec[0] = *it;
    // output with \n as the separator
    std::ostream_iterator<int> out{std::cout, "\n"};
    for (int i = 1; i < 5; ++i)
        vec[i] = *(++it);
    for (auto &ele : vec)
        *(out++) = ele;
```

- However, they are mostly used with **`std::(ranges::)copy`**.

```cpp
    std::vector<int> vec(5);
    std::copy(std::istream_iterator<int>(std::cin), std::istream_iterator<int>(), vec.begin());
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<int>(std::cout, "\n"));
```

- The default constructed **`istream_iterator`** is “end”; this means to terminate until the input stream cannot be parsed as **`int`** (e.g. input a non-digit character) or encounter with stream error (e.g. end of file for file stream).
  - So it’s dangerous because you may not assume the input of users, and the vector iterator may exceed its bound…
  - Some may hope to use **`std::copy_n(std::istream_iterator<int>{std::cin}, vec.size(), vec.begin())`**, but if the input stream reaches its end, the dereference is invalid too…
    - There is no **`copy_until`**!

### Iterator adaptor

- There are two kinds of iterator adaptors:
  - One is created from iterators to perform different utilities:
    <img src="img/iterator_adaptor.png" alt="iterator_adaptor" style="display:block; margin:auto;" />
    - E.g. reversed iterators, i.e. the underlying type is also iterators, while **`++`** is in fact **`--`**.
      - You can construct from an iterator, i.e. **`std::reverse_iterator r{ p.begin() }`**.
    - You can get the underlying iterator by **`.base()`**, which actually returns the iterator that points to the elements after the referred one.
      - E.g. **`rbegin().base() == end()`**
    - There is another adaptor called move iterator, which will be covered in ***Move Semantics***
- Another is created from containers to work more than “iterate”.
  - **`std::back_insert_iterator{container}`**: **`*it = val`** will call **`push_back(val)`** to insert.
  - **`std::front_insert_iterator{container}`**: call **`push_front(val)`** to insert.
  - **`std::insert_iterator{container, pos}`**: call **`insert(pos, val)`** to insert, where **pos** should be an iterator in the container.
  - They are all output iterators, and **val** is provided by assignment
  ***Example:***

  ```cpp
  std::vector<int> vec;
  std::copy(std::istream_iterator<int>{std::cin}, std::istream_iterator<int>(), std::back_insert_iterator{vec});
  std::copy(vec.begin(), vec.end(), std::ostream_iterator<int>(std::cout, "\n"));
  ```

- Notice that inserting/assigning a range directly is usually better than inserting one by one (as done in **inserter**) for **`vector/deque`**.
  - Or at least “reserve” it before (we’ll learn them sooner).
- Final word: there are methods like **`std::make_xxx`** or **`std::xxx`** (e.g.**`std::back_inserter()`**, **`std::make_reverse_inserter()`**);
  You can also use these methods to get the corresponding iterator adaptors.
  - Before **C++17**, you have to specify the type parameter for template of class, so if you don’t use these functions, you have to write tedious **`std::back_inserter_iterator<std::vector<int>>(vec)`**.
  - Since **C++17**, **CTAD** (***Class Template Automatic Deduction***) will deduce it, so methods are generally not shorter than object initializations.

## Sequential Containers

### array

- We’ve learnt **C-style** array, e.g. **`int a[5]`**;
  - However, it will *decay* to **`int*`** when passing to function, and the size information is dropped.
    - i.e. the first dimension of the array parameter is meaningless, void func(**`int a[5]`**) is just same as **`void func(int a[])`** or **`void func(int* a)`**.
    - So, **`sizeof(a)`** is different inside and outside the function…
    - The **return type** cannot be **`int[5]`**, too…
  - For int **`a[5], b[5], a = b`** is invalid.
  - The bound is never check so invalid memory may be accessed…
- All in all, we need a safer array! **`std::array<T, size>`** is for you.
  - It’s same as **`T[size]`**, except that it always preserves size, can copy from another array, and can do more things like bound check.
- For ctor: just initialize **`std::array`** in the same way as **`C-style`** array (may need adding an additional pair of paratheses).
  - For example, **`struct S {int i; int j;};`**, **`std::array<S,2> arr{{ {1,2}, {3,4} }}`**.
- For member accessing:
  - **`operator[]/at()`**: accessing by index; **`at()`** will check the bound, i.e. if the index is greater than size, **`std::out_of_range`** will be thrown.
  - **`front()/back()`**: get the first/last element of vector.
  - Contiguous iterators, as we stated.
  - If you want to get the raw pointer of array content, you can use **`.data()`**.
- You can also use some additional methods:
  - **`.swap()`**: same as **`std::swap(arr1, arr2)`**.
  - **`operator=`**, **`operator<=>`**.
  - All these methods need same array size!
  - **`.fill(val)`**: fill all elements as val.
  - **`std::to_array(C-style array)`**: since **C++20**, get a **`std::array`** from a C style array.
- For size operations:
  - **`.size()`**: get size (return **`size_t`**).
  - **`.empty()`**: get a bool denoting whether **`size == 0`**.
  - **`.max_size()`**: get maximum possible size in this system(usually useless).

### vector

- To be exact, vector is dynamic array which can be resized.
  - It supports random access and occupies contiguous space.
  - When inserting and removing elements at the end (i.e. pushing / popping back), the complexity is **amortized 𝑂(1)**.
    - If not at end, it’ll be **𝑂(𝑛)**.
- This container is so important that we’ll spend lots of time on it.
  - It’s the most commonly used containers in all containers.
  - Though sometimes its theoretical complexity may be higher than other containers (e.g. list), it may still be a better choice since it significantly **utilizes cache**.
    - We’ve learnt in ICS that a cache-friendly program may consume hundreds or even thousands times less time than a bad one.
    - Use a profiler if you cannot determine which one is better!
- the most naïve version of vector is:
  - When pushing back, allocate continuous space with one more, copy* all and add the new one to the new space, finally freeing the original.
    - You cannot directly allocate one new space since it cannot guarantee the property of array - “contiguous space”.
  - When popping back, shrink the space by one and copy all rest to the new one, finally freeing the original.
- Obviously, you need **𝑂(𝑛)** on every pushing or popping…
  - So, what if we “prepare” more space than needed in allocation, so that pushing will only construct new object at back?
  - This is **𝑂(1)**, and we just need to control **reallocation** to happen only rarely so that copying will be **amortized 𝑂(1)**.
  - The element number is called **size**; total space is called **capacity**
- The easiest strategy is increasing space linearly.
  - E.g. 0->4->8->12->16…
  - Every `𝑘` operations will trigger reallocation and copy **`𝑛 = 𝑘𝑚`** elements.
  - So, the amortized complexity is $:\Theta\left(\frac{\sum_{i=1}^{m}ki}{km}\right)=\Theta(m)=\Theta(n/k)$
  - Considering that 𝑘 is an constant, this is still **𝑂(𝑛)**.
  - This means “linear” is not “rare”!
- So, what about exponentially?
  - E.g. 1->2->4->8->16->32…
  - Every $2^{k}$ operations will trigger reallocation and copy $n=2^{k}$ elements
    - So, the amortized complexity is $\Theta\left(\frac{\sum_{i=1}^{k}2^{i}}{2^{k}}\right)=\Theta(1)$
    - Summation of arithmetic progression v.s. geometric progression.
- vector also supports insertion of a range.
  - So more than one elements may be inserted.
- Considering that some insertion will make >2x growth.
  - You may calculate the smallest capacity that is larger than needed.
  - In MS, it directly allocates needed space, which is cheaper
  