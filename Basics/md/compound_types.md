# Compound types

## Pointer Type & Reference types

### Pointer Type

In C++, **`NULL`** is just an integer rather than a pointer!

### Reference Type

**Differences** from pointer

- Reference is not nullable.
- Reference cannot change the referred object.
- Reference may or may not occupy any memory, while pointer will definitely.
- But this isn’t usually that important because of compiler optimization.
- Reference is forbidden to be template parameters in many parts of standard library, like you cannot **`std::vector<A&>`**.

## Array Types

### Notes

E.g. **`int a[3] = {1,2,3}, b[] = {4,5,6}`**;

- Though you may see **array as pointer** before, it’s **not true…**
    > That’s in fact **decay**

- A special array is *C-string*, e.g. **`const char* str = “test”`**;
  - “test” is **`const char[5]`**, but it may **decay** to **`const char*`**.
  - **`char*`** (drop **const** for string literals) here is not permitted in C++.
  - Null-terminated, so different with **char array itself**.
- There are no **array of functions** or **array of references**.
- VLA (i.e. int arr[n]) is not allowed in C++.

### Multidimensional array

E.g. `int a[2][3] = { {1,2,3},{4,5,6}}`;
All dimensions but the first should have explicit size

### Dynamic allocation

- **`malloc`** in C (function defined in **`<cstdlib>`**) and **`new/new[]`** in C++ (as a *keyword*)
  - *Difference*: **`malloc`** will **only allocate memory**, **`new`** will **call ctor** if the object provides one
  - **`new`** and **`malloc`** will get **a pointer** instead of an array!
    - It points to the content of some continuous space
  - When memory is exhausted
    - **`malloc`** will return **`NULL`**
    - **`new`** will throw **`std::bad_array_new_length`**, an exception inherited from **`std::bad_alloc`**, defined in **`<new>`**
- Deallocation: **`free`** and **`delete/delete[]`**
  - They are not interchangeable; **`delete`** will **call dtor** of objects.
  - Deallocation will & should **always not throw exception**

## Function Types

### Function

- Function implementation (definition):
  - Param types and return type should be same as prototype
  - **default params** should **not appear here**.
  - The definition can specify return type as **`auto`** since **C++14**: it can be deduced from return statement (but **ref** and **const** is still not deduced.)
    - If types of **multiple return differ**, compile error
- The return type **cannot** be **function** or **array** (but **can** be their **references** or **pointers**)
  - shouldn’t return **references (and also pointers)** of **`local variables`**
    > since they will be destroyed after exiting the function, which makes them **dangling references/pointers**(悬空引用/指针)
-**`static`** **local variable**: “global variable” in the function, not directly accessible to others
  - Initialized iff. the first time its initialization is executed

### Understanding complex type

When the return type is **function pointer**, the prototype is like:

```cpp
int Bar(int) {return 0;}
int (*Foo(float realParam))(int) {return &Bar;}  

E.g. int(*Foo(int, int(*)(int)))(int(*)(float));
```

### Type alias

In C, you may use **`typedef`**, but it’s not as intuitive & powerful as **`using`**

- Particularly, it’s not same as pure text replacement! You see the alias **as a whole**.
- For example, **`using CPtr = char*; using ConstCPtr = const CPtr`** will not get **`const char*`**, but **`char* const`** i.e. the pointer itself cannot be changed.

### Attribute

- Sometimes, the return value of a function should not be omitted
  but there is no way to force users to notice that…
- Since **C++11**, **`[[attribute]]`** is introduced; this is a standard way to extend the language
- For function, you may use **`[[nodiscard]]`** since **C++17** before
return type to denote the return value should not be dropped.
  - Since **C++20**, you may specify reason as **`[[nodiscard(“reason”)]]`**
  - The compiler will report a warning if it’s dropped.
- It’s recommended to specify the attribute **both in declaration and in definition**

other attributes：

- **`[[deprecated]]`** and **`[[deprecated(“reason”)]]`** since **C++14**: mark the function as deprecated so that users should not use it in new code.
  - This can also be used in **namespaces** and **enumerators**
- **`[[noreturn]]`** since **C++11**: some functions may never return. Specify functions with this attribute will suppress the warning, and also tell users that this function will never return
- **`[[maybe_unused]]`** since **C++17**: when some entities (functions, variables including params) seem unused, compilers may report a warning. This will suppress the warning.

### Enumeration

E.g

```cpp
enum class Day : std::uint8_t {Monday, Tuesday, Wednesday = 10, Thursday, Friday, Saturday = 20, Sunday};
```

- you may use **`std::underlying_type<Day>::type`** or **`std::underlying_type_t<Day>`** (since **C++14**) to get the integer type (e.g. here is **`std::uint8_t`**); defined in **`<type_traits>`**
    > In **C++23**, you may also use **`std::to_underlying<Day>(day)`** to get the underlying integer directly; defined in **`<utility>`**
- it’s still **legal** to initialize like **`Day day{1}`**; (since **C++17**); But it’s **illegal** to use **`Day day = 1`**; or assign **`day = 1`**
- Enumeration is also widely used in **bitwise operation**
  > However, scoped enumeration **doesn’t support arithmetic operations**
  so you may either use unscoped one carefully (e.g. with namespace) or define the operator yourself
- you need to ensure not to exceed the limit of enumeration value in the meaning of bits **`(i.e. (1 << std::bitwidth(MaxEnum)) – 1 or (1 << (MSB(MaxEnum) + 1)) – 1)`**, otherwise **UB**

## Class

### Ctor&Dtor

- Notice that once the reference member is initialized, it cannot change the referred object. So if the object is actually destroyed, then the member is dangling!
- Ctor initializes data members in the same order of their declaration; destruction happens reversely (like a stack)
