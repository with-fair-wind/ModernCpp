模板进阶 Advanced Template

## 现代C++基础

## Modern C++ Basics

Jiaming Liang, undergraduate from Peking University

- Supplementary Template parameter that’s not a type Type Deduction Friend in class template Laziness
- Variadic Template

- SFINAE

- Common Techniques CRTP Type Erasure

# Advanced Template

Supplementary Miscellaneous Knowledge

### Supplementary Miscellaneous Knowledge

- Supplementary
- Template parameter that’s not a type
- Type Deduction
- Friend in class template
- Laziness

## Template parameter that’s not a type

- First and foremost, very strangely…
- Template parameter that’s not a type ≠ non-type template parameter (NTTP)
- There are three types of template parameters:
- Type template parameter, i.e. template<typename T> or template<class T>;
- Non-type template parameter, e.g. template<int N>;
- Template template parameter.

Easier, so we talk about it first.

## Template template parameter

- First observe how a template is defined:

- Similar to function parameter, name can be neglected if it’s not actually used:

- And that’s how a template template parameter should be defined:

Here we can also use typename since C++17.
- And we can use template in this form to fill in the parameter!
- E.g. B<A>.

## Template template parameter

- A more complex example:

- Require a template with a type parameter and an NTTP, and the NTTP depends on the type parameter.
- Notice that this T cannot be used outside:

- This is very like scope of variable; outer scope cannot use names in inner scope.

## Template template parameter

- Exercise: explain this template.

- It accepts a type parameter T, and a template template parameter U…
- Where U has a NTTP, whose type is T.
- For example:

Have A<0> a; as data member in B<int, A>.

## Template template parameter

- A practical example: write a stack class!
- Users may use any container to store elements as long as it supports push/pop/…, e.g. std::vector/deque, std::inplace_vector in C++26.

We notice that std::stack uses two type parameters, i.e. template<typename T, typename Cont = std::deque<T>>.

## Template template parameter

- Example in standard library: std::ranges::to.

- Note 1: before C++17, template template parameter requires exact match.
- After C++17, default parameters are considered.
- E.g. std::deque actually has two parameters:

- You have to use:

Cannot fill std::deque before C++17.

## Template template parameter

- Note 2: you can also use default parameter inside template template parameter.

<> Is needed even no parameter is manually filled.

## NTTP

- Previously, our NTTPs are always integers…
- E.g. std::array<T, N>, IsPrime<N>.
- While there are more possible types, including:
- Enumerations (special kind of integer);
- Pointers (and nullptr_t), pointer to member;
- Lvalue reference;
- Floating points;
- Some “simple” classes;SinceC++20
- constexpr Lambda.

- These types are referred as “structural types”.

## NTTP

- For example:

There are actually many restrictions for passed pointer / reference, see [temp.arg.nontype].

## NTTP

- For pointer and reference, passed argument has restrictions.
1. Its address should be determined in compile time, so only those with static storage duration are allowed.
2. Linkage requirement: C++98 external, C++11 includes internal, C++17 includes no linkage (i.e. static variable in function).

Whether two templates are the same instantiation depends on the address.

## NTTP

- A special kind of “pointer” is string literals.
- It’s not allowed to use string literals to initialize const char*.
- Solution:

- That’s inconvenient since we have to introduce additional named variable…
- And normally our understanding of equivalent template should be “have the same string content”, instead of same address.
- Since C++20 you can use class-type NTTP!

## NTTP

- For class NTTP, we first introduce literal types:

Not class, ignored.

## NTTP

- And class NTTP just requires:
- Be a literal type;
- All base classes and non-static data members are public, non-mutable and structural types (or array of structural types).
- Particularly, for pointer and reference member, it has the same restrictions as NTTP.
- Finally, template class is also allowed to write at NTTP, and the concrete type will be deduced automatically.

- So we can easily write a FixedString class…

## NTTP

Cannot be const char* str as:
1. It’s non-owning, cannot do complex operations like concatenation;
2. It’s forbidden to point to string literal as the last page shows. We’ll provide an exercise in homework to write a more reasonable class.

## NTTP

- Floating points can be seen as instantiated by “underlying binary representation”.
- E.g. if the platform uses IEEE 754, Test<1.0f> can be hypothetically viewed as Test<3f800000>.
- But all NaN will be seen as equivalent.
- And we notice that floating point rounding error is still important.

## NTTP

- Since C++17, it’s also allowed to accept NTTP of any type.

- Before that, you need to add an additional type parameter for assistance.
- But you have to specify that type manually, very inconvenient…

- It’s then easy to accept lambda as NTTP:

## NTTP

std::invocable<F, Args…> requires F to be callable with arguments Args…; here it means Callable should use 0 parameter.

Call Callable. Lambda should be constexpr (thus no capture is allowed), but this is implicitly added since C++17 as long as all operations are allowed in constexpr function.

- Notice that each lambda has its unique type even if they have same closure body, so the instantiation is unique theoretically.

### Supplementary Miscellaneous Knowledge

- Supplementary
- Template parameter that’s not a type
- Type Deduction
- Friend in class template
- Laziness

## Type Deduction

- We don’t need to fill all template parameters due to type deduction…
- Each function parameter will deduce template parameter independently.
- Assuming template parameter is P and passed argument type is A…

- For each deduction, there are generally two rules:
- For non-reference parameter P, decayed A is deduced.
- P will ignore its top-level cv-qualifier.
- For reference parameter (e.g. P&), the original A is deduced.
- *auto has basically the same rule.
- And finally, if deduction leads to conflict types, fail to match.

## Type Deduction

- For example:

Top-level cv is ignored, equiv. to use 1 to deduce T.

Not decayed, so conflict.

## Type Deduction

- In most cases, conversion is forbidden in deduction.

Deduction failure due to conflict, rather than T = int + A -> int conversion.

Due to separate deduction, this is inevitable.

Deduction failure instead of T = int + B<int> -> A<int> conversion.

This may be evitable?

## Type Deduction

- There are three special cases to allow conversion:
1. If P is reference, the deduced A can be more cv-qualified than A.
2. If P is pointer, the deduced pointer can have qualification conversion.

1 and 2 are different, since more cv-qualified int* is int* const, not const int*.

3. If P is base class (pointer), A is derived class (pointer), derived-to-base conversion is allowed.

## Type Deduction

- Sometimes deduction needs further match…

If we regard it as a whole, it’s not reference type (i.e. V funcPtr).

Then we know V = void(*)(int&), the decayed function type.

And we match T(*)(U&), getting T = void and U = int.

- Such match is exact, no conversion is allowed.

## Type Deduction

- Lots of pattern can be used to match, not described in detail.
- Sometimes recursive match is needed.

## Type Deduction

- There also exists non-deduced context ([temp.deduct.type]), i.e. where parameters cannot be deduced.
- std::initializer_list cannot be deduced, as we stated before.
- auto in definition can deduce it, e.g. auto a = {1,2,3}.
- The first/major array bound, if P is not reference / pointer type.
- Reference deduction won’t decay; pointer has already decayed the first bound.

## Type Deduction

- NTTP cannot be deduced from expression:
- Cannot deduce N = 5 here.
- Qualified type names cannot be used for deduction:
- Cannot deduce T = int here.
- Default parameter cannot be used for deduction:
- Though Func() should be equiv. to Func(1), T cannot be deduced as int.
- decltype type.
- Cannot deduce T = int here.

## Type Deduction

- There are some other deduction contexts, not cover it in detail.
- Address of an overload set, where return type will also be used for deduction.
- If there are multiple best match after overload resolution, ambiguous.
- Conversion function template.
- …

- Since C++17, class template argument deduction (CTAD) is introduced.
- It deduces class template argument from constructor, so rules are similar to deducing from function call.

## CTAD

- Just transform template parameter of class to ctors!

- And then use rules before…
- Wait, where is copy/move ctor?
- As deduction will strip reference, they’re combined together as a single hypothetic function:

- Besides, C++ allows user-defined deduction guides in CTAD.

## CTAD

- For example:
- CTAD of std::list is quite simple; it comes from list(std::initializer_list<T>).
- But vector from iterator is template<typename Iter> vector(Iter, Iter).
- There is no T at all…How is class template parameter deduced?
- User-defined deduction guide is very similar to ctor, just add trailing return type to specify “what should be deduced”.

Use std:: iter_value_t <InputIt> since C++20.

## CTAD

- explicit: if written, disable copy initialization deduction.

- requires clause: specify concept constraints for when deduction happens.

## CTAD

- Note 1: CTAD cannot do partial deduction. All or nothing!

- Note 2: CTAD doesn’t have to be same as some ctor; deduction and overload resolution are separate.

## CTAD

- Note 3: unlike method definition, the class name cannot use qualified name in deduction guide.
- Assuming A is in namespace Test.

- So, usual practice is to write deduction guide immediately after class definition.

## CTAD

- Note 4: in class context, injected template name is preferred over CTAD.
- Injected class name means that A refers to current instantiation, no need to write A<T> explicitly.
- But injected class name is disabled when using qualified name, then CTAD happens.

- In this example CTAD or not doesn’t differ…

## CTAD

- A better example:

Otherwise it’s X<T>(b, e), but we actually want X<U>(b, e).

## CTAD

- Note 5: in deduction guide, T&& is still universal reference, not rvalue reference.
- Here due to reference collapsing, ctor is then transformed to A(int&).
- *So we say A(T&&) isn’t universal reference because it cannot deduce lvalue reference without deduction guide.

- To prevent that astonishment, normally deduction guides use value type.
- Here A<int> rather than A<int&> is deduced, compilation error as expected..

Clang 19 is officially released several months ago.

## CTAD

- Note 6: only the class itself can be deduced; adding reference / pointer would lead to failure.
- In most cases just use auto.
- Note 7: implicit deduction guide for aggregate is added since C++20.
- Example: user-defined guide must be added in C++17.

- Note 8: C++20 also introduces deduction for alias template.
- To put it simply, for every deduction guide, use alias to deduce parameters by trailing type as much as possible. Remove deduced ones, add alias constraints and use only non-deducible ones to form new guides.

# Alias Template Deduction*

- This is very complex so optional.
- We use A as example here; #1 and #2 form two deduction guides.
- From #1, use C<V*, V*> to deduce C<T, U>, forming

- From #2, use C<V*, V*> to deduce C<T, std::identity_t<U>>, forming

- U is not deducible, as it’s an alias of qualified type (non-deduced context).
- Strip useless parameters and add constraints (here A doesn’t have constraints; if it’s B, then add std::integral to W).

# Alias Template Deduction*

- Now we get two guides for alias:

- Finally, when it’s deduced, we need to check again whether it satisfies alias.

from #1 both #1 and #2 have V*

#2 deduces C<int*, double*>, but it doesn’t match A<V>.
- See more practice in C++ standard and cppreference.
- gcc has a bug on this feature, see c++ - How to write deduction guidelines for aliases of aggregate templates? - Stack Overflow (and you can also do practice by this analysis).

## CTAD

- Note 9: since C++23, CTAD can happen through inherited ctor.
- For template parameters of derived class, if they’re part of parameters of base class, they’ll be deduced by guides of base class.
- Let’s see examples in the standard directly:

T isn’t part of base class.

Inherited ctor.

T and U are part of base class.

### Supplementary Miscellaneous Knowledge

- Supplementary
- Template parameter that’s not a type
- Type Deduction
- Friend in class template
- Laziness

## Friend in class template

- To define a friend method for a class:
- We can also define friend function in the class, and ADL will help us to find it.
- Like defining operator<< as friend.
- For a class template, splitting definition is harder…
- Is it correct? No!

Friend of A<T> is a normal function func(A<T>), not a template function.

*But unlike normal functions, their definition is instantiated only when used.

## Friend in class template

- If we call & compile:

- For A<int>, the friend method is Func(A<int>), so we may try:
- Succeed to compile!
- But we can never define Func for any T.
- Solution 1: make template method as friend instead of normal method.

## Friend in class template*

Optional as the
- Limitation: not exactly same as our previous intention.difference is subtle.
- Since all instantiation regard template Func as friend, instead of its own normal Func.

A<int> friend: void Func2(A<int>&, A<U>&);

A<float> friend: void Func2(A<float>&, A<U>&);

Different friends, so A<int> friend cannot access A<float> member. Okay Both A<int> Error: cannot and A<float> access b.member. regard it as friend, so okay.

## Friend in class template

- Solution 2: specify template instantiation as friend.
- Limitation: hard to code; we need two additional forward declarations.

We notice that no template keyword is needed here, unlike explicit instantiation.

- Solution 3: define friend methods inside the class directly, and use ADL to call it.
- Limitation: ADL is quite obscure.

## Friend in class template

- For friend class:

U

- For friend T, if T isn’t a class, it will be ignored.

### Supplementary Miscellaneous Knowledge

- Supplementary
- Template parameter that’s not a type
- Type Deduction
- Friend in class template
- Laziness

## Lazy Instantiation

- When a class template is instantiated, not all of its members are immediately fully instantiated.
- Some of them will only be fully instantiated when they’re actually used.

C++ standard, section [temp.inst].

## Lazy Instantiation

- Example:

E.g. we define A<int, -1> a.

As long as we don’t call / use them to force their full instantiation.

Declaration is not valid. Only definition can be lazy instantiated.

## Lazy Instantiation

- Similarly, there exists “laziness” in some other cases.
- Value template initialization:

- When value of v is not used, default_value() is allowed to not exist in T.
- For example, decltype(v<int>) will compile successfully.
- Default value:
- When default value isn’t used, this assignment is allowed to make no sense.
- E.g. when Func is called without filling default value (i.e. only calling Func(xxx), never calling Func()), T is allowed to be not default constructible.
- Pointer definition:
- Definition of Array isn’t checked so N = -1 won’t make compilation fail.
- And concept (check our homework in the last lecture!).

## Lazy Instantiation

- Some laziness depends on compiler.
- Virtual function: some compilers always instantiate virtual function to construct full vtable.
- Semantic error: instantiation check is after syntactic and semantic check theoretically, while some compilers will drop semantic check if it’s not instantiated.

Defined in class A; Not all compilers detect this as error if it’s not called.

# Advanced Template

Variadic Template

## Variadic Template

- Variadic Template
- Basics (Pack expansion & …)
- Fold expression

## Variadic Template

- Many functions can accept unbounded number of parameters…
- std::vector::emplace_back, std::format, std::invoke, etc..
- This is enabled by variadic template!
- Let’s use printing a bunch of parameters as example.firstArg is 7.5 and T is double.

args are “hello”, s and Types are const char*, std::string. Called template parameter pack, meaning “any number of template parameter”. Called function parameter pack, meaning “any number of parameter”. Called pack expansion, meaning unpack args to arg1, arg2, …. print(7.5, “hello”, s) -> print(“hello”, s) -> print(s) -> print()

## Variadic Template

- Every function call passes value type, which incurs overhead.
- They’re read-only, make them const&!

- And we may want to eliminate empty function…
- That is, when number of args is 0, do nothing.
- You can use operator sizeof…(pack).

Here you can also use sizeof…(Args); any pack is ok.

## Variadic Template

- Exercise 1: write declaration of std::invoke (use universal ref, no need to add concept).

- And we need to forward args to func.
- Assuming that func is called by operator().
- So generally, pack expansion is:
- Write a pattern, as if operating on a single normal parameter.
- E.g. here the pattern is std::forward<Args>(args).
- And add … (sometimes need additional space before …), meaning that applying pattern on every element and concatenating with ,.
- So it’s like std::forward<Arg1>(arg1), std::forward<Arg2>(arg2)….
- Notice that it’s semantic substitution, not a comma expression.

## Variadic Template

- Exercise 2: write a function that accepts a container and many indices, and passing corresponding elements to print.

- Exercise 3: what’s the type of result?

std::tuple< Answer: std::pair<char,float>, std::pair<int, double> >

## Variadic Template

- [Hard] Exercise 4: explain what g accepts.

Assuming sizeof…(OuterTypes) == N, sizeof…(InnerTypes) == M.

- InnerTypes(innerValues)… is expanded to I1{iv1}, I2{iv2}, ….
- And these M parameters are used to construct some type OuterTypes, forming a new pattern.
- Expand this pattern, we get O1{ I1{iv1}, I2{iv2}, … }, O2{ I1{iv1}, I2{iv2}, … }, ….
- And these N parameters are passed to g.

## Variadic Template

- Exercise 5: write a class that public inherits all of its template parameters.

- Note: since C++17, it’s allowed to inherit all methods by expansion:

## Variadic Template

- Exercise 6: write a ctor to accept params to initialize each base class.

## Variadic Template

- Exercise 7: explain code below.

## Variadic Template

- This is basically equivalent to generate:

- Here A, B, C are compiler-generated types for lambda, and each of them generates an overload for operator().
- Notice that aggregates can have base class and implicit CTAD guides only since C++20; in C++17 we need additional guides.

## Variadic Template

- Exercise 8: write deduction guide for Overloaded in C++17.

- Exercise 9: what does g(1, 2) print?

Answer: 2 3 3 4

h(args) + args…  h(arg1) + arg1, h(arg2) + arg2, …

h(args…) + args…  Expansion pattern will h(arg1, arg2, …) + arg1, examine all previous code as a whole!h(arg1, arg2, …) + arg2, …

## Variadic Template

- Note 1: semantic substitution is different from plain text substitution. It has two steps:
- Syntactic parse: examine what meaning the code wants to explain; this is unrelated to size of pack.
- Substitution: insert types in AST.

- This explains why it works when sizeof…(T1) == 0:
- If it’s just text substitution, then it’s class A: public {} (wrong syntax!).
- If it’s parsed to AST and then types are substituted, then it means “A inherits 0 type”, which is correct.
- Note 2: variadic pack can only be the last parameters.

Intuitive reason: pack will match eagerly, so args matches 1, 2 and arg matches nothing.

## Variadic Template

- But this is Okay:

- Reason: Ts… is written at last so it matches last. This is unrelated to position of template parameter declaration (as it can be deduced)!
- Note 3: variadic template is less preferred than normal template in overload resolution.

- But when non-variadic one has more parameters filled with default, it’s still * ambiguous.

*: Though this is regulated in C++ standard, only clang implements it.

## Variadic Template

- Note 4: NTTP can also use variadic template.

- But there is no way to hybrid two variadic templates.
- i.e. impossible to define “any number of template parameter, either type or non-type”.
- Note 5: pack can be indexed at compile time since C++26.
- But only id pack; expression pack cannot be indexed.

## Variadic Template

- And template template parameter pack isn’t allowed to index.
- Note 6: friend can also be expanded pack since C++26.

## Variadic Template

- Note 7: to capture variadic arguments in lambda, you need to:

- However, you cannot transform it to a named parameter like normal capture:

- Since C++20, this is improved so that you can write:

## Variadic Argument

- We notice that C also has variadic argument (e.g. printf), but it’s very obscure to use.

- We’re not going to talk about how to use it, but several notes:
1. It still uses ellipsis, but not on a pack expansion:

- Comma before … can be omitted (deprecated since C++26), so you can write something like: VariadicVariadic templateargument
2. It has lowest overload resolution precedence, so void Test(…); means “the least preferred overload”.
- We’ll utilize this property in SFINAE.

## Variadic Template

- Variadic Template
- Basics (Pack expansion & …)
- Fold expression

## Fold expression

- With only simple pack expansion, print is still very strange…
- Why do we have to do recursion? Normally it should just be a loop…
- To reduce recursive operations in pack, fold expression is introduced since C++17.
- For example, to add all elements in a pack:

- (Pack OP … OP Init), meaning (Packop (... op (Packop (Packop Init)))) 1 N−1 N
- We notice that it doesn’t necessarily mean Packop Init evaluates first; this N depends on evaluation order, as we reviewed in Lecture 1.

## Fold expression

- Exercise: write print by fold expression.
- Hint: comma expression.

() is necessary to raise precedence;Comma expression will evaluate in fold expression, pack should beone by one from left to right, so with precedence not lower than cast.the output is determined.

- Besides binary right fold, there also exists binary left fold:
- (Init OP … OP Pack), meaning ((((Init op Pack) op Pack) op ...) op Pack) 12N
- They differ when operation is not communicative: args == 1,2,3, args - ... – 0  (1-(2-(3-0))) = 1-(-1) = 2, 0 - ... – args == (((0-1)-2)-3) = -6,

## Fold expression

- And besides binary fold, there also exist unary fold:
- Unary right fold: (Pack OP …), meaning (Packop (... op (Packop Pack))) 1 N−1 N
- Unary left fold: (… OP Pack), meaning (((Packop Pack) op ...) op Pack) 1 2N
- Without Init, this expression is only valid when sizeof…(Pack) > 0! AddAll() will lead to compilation error.

- Exception 1: comma expression with 0 size is right, which means nothing.

print() is correct, nothing is done. (Formally result is void).

Notice that it may be astonishing when operator&&/|| is overloaded to return types other than bool. Then: sizeof...(values) == 0 ➔ return type is bool;

## Fold expression

sizeof...(values) > 0 ➔ return type is overloaded type.

- Exception 2: logical expression with 0 size is correct.
- Return a value that doesn’t interfere its operation result (i.e. not short-circuit).
- For &&, return true; for ||, return false.

## Fold expression

- Since C++26, it’s also allowed to reasonably use fold expression in constraints.
- Before that, it’s valid but doesn’t subsume.

# Advanced Template

SFINAE

## SFINAE

- We’ve already learnt concept to exclude unintended types.
- But how is it implemented before C++20?
- By SFINAE (Substitution Failure Is Not An Error)…
- Pronounced as si-fee-nay.
- As its name, for a template, when substituting corresponding template parameters leads to an invalid result…
- It’s just excluded from viable candidates during overload resolution, not causing compilation error.

Func<int>(1) is
- For example: valid though int::value_type doesn’t exist.

It’s just excluded from overload resolution.

## SFINAE

- But overload resolution only checks declaration (i.e. prototype), so invalid function body will still lead to compilation error.
- Normally, SFINAE cooperates with two structs in <type_traits>:
- std::enable_if<Cond, T = void> to check a Boolean;
- When Cond is true, it defines type = T;
- When Cond is false, it doesn’t define type;
- std::void_t<Args…> to check expressions;
- E.g. std::void_t<decltype(Exp1), decltype(Exp2)>, if Exp1/Exp2 is invalid, decltype(Exp) is invalid.
- If all types are valid, then it’s alias of void.
- These two can already convey most of require expressions.

## SFINAE

- Example 1: implement std::advance(it, n).
- For random access iterator, it should be it += n;
- Otherwise increment in a loop.
- First implement iterator tag check:

is_convertible is due to tag inheritance.

## SFINAE

- And then define different overloads:

Necessary, otherwise ambiguous call for non-random- access iterator.

*We notice that void* in test(void*) is necessary to make it has higher precedence than (…) when calling (i.e. … should match at least one param).

## SFINAE

See stackoverflow.

- Example 2: Implement std::is_default_constructible.
- We might as well use a method that differs in return type.

We can also choose to not use std::void_t here; but when there are more types, When U() is invalid, this is excluded.you need more typename = … if no Least preferred in overload resolution; if test<T> std::void_t. doesn’t match method above, it matches this one.

true  same  first overload is valid  T() is valid  default constructible

## SFINAE

- Class template can also use SFINAE by its template declaration.
- Solution2: Equiv. to define static constexpr bool value = false.

- We notice that void in typename = void is necessary.
- Reason:
- When T() is valid, the specialization is equiv. to specialize <T, void>.
- Users need to call IsDefaultConstructible<T>, and filling void can make it match specialization (if it’s not SFINAE’ed out).

## SFINAE

- Example 3: Implement Dictionary.
- When it supports operator<, use std::map;
- Otherwise when it supports std::hash, use std::unordered_map;
- Otherwise manually implement std::vector-based linear search.
- Is it correct to code like this?

## SFINAE

- Not really, since if a type supports both less-than and hash, two specializations will conflict.
- But unlike concept, we cannot negate the expression to “check it doesn’t work” in void_t.
- Solution: convert this check to bool value and use enable_if…
- So first write two struct that check this (try it yourself)!

It’s just like defining a concept, but in a very strange way.

## SFINAE

- Then specialize like this:

## SFINAE

- A small problem: what if Key isn’t default constructible?
- This is still excluded, which is not as expected.
- In concept, we introduce “hypothetic parameters” in requires expression to avoid that.
- Before that, we can use std::declval<T>().
- It returns T&&, but has no implementation.
- Yes, this function cannot be really called; it’s just used to pretend to return something, which can be used for expression.
- So in our example, it should be:

Of course, you can std::declval<std:: hash<Key>>() if you want.

## SFINAE

- But notice that since the function theoretically adds rvalue reference, it cannot e.g. call lvalue-only methods.
- You need e.g. std::declval<Key&> to make lvalue also work (due to reference collapsing).

## SFINAE-friendly Declaration

- Notice again that if error doesn’t happen in head, SFINAE won’t kick in.
- Thus, a SFINAE-friendly traits should expose all requirements there.
- Specifically:

- Example 1: if default argument is invalid, it will cause compilation error directly.

## SFINAE-friendly Declaration

- Example 2: addA will be SFINAE’ed out if t + u is invalid, but addB won’t.

- Example 3: if we want to write a generic operator+ for our Array<T>.

## SFINAE-friendly Declaration

- Possible solution:

- What if we use struct to implement it?

The result of Type doesn’t participate in SFINAE (as it has side effects of further instantiation of PlusResult);

SFINAE needs to make Type not defined instead of making it invalid.

*Actuallythe standard regulates that it should cause an error, but msvc doesn’t implement it.

## SFINAE-friendly Declaration

- So if we define an overload for e.g. Array<A> + Array<B>, when A+B is invalid, whether it compiles depends on whether PlusResult is instantiated… *
- It’s compiler-dependent.
- So if you really want to use struct, you’d better to expose it to head (e.g. with partial specialization):

## std::conditional

- Finally supplement some knowledge on std::conditional.
- Sometimes we may find it inconvenient to define specialization for if-then-else case.
- For example, in Dictionary example, the specializations are like (in pseudo code):

- Instead, we can write with std::conditional<Cond, TrueType, FalseType>:

## std::conditional

- And then:

- You can also use std::conditional_t:

- But not in all cases std::conditional can be used to directly substitute specialization, when TrueType or FalseType may be invalid.

## std::conditional

- For example, if we want to define a TryUnsignedType<T>…
- For integer (but not bool) T, make it unsigned;
- Otherwise original T.
- We may write it like:

- Problem: std::make_unsigned_t<T> is valid only when condition is true; but to check whether the expression is valid, both branches will be checked in any condition.

## std::conditional

- We’ve seen similar problem before, where we have to use specialization:

## std::conditional

- Instead, we can also defer type evaluation!

- Though make_unsigned<T>::type may not exist, make_unsigned<T> always exist! After selecting correct branch, we use ::type to get the actual type.
- std::type_identity<T> is just using type = T, as a wrapper in this case.
- *Our dictionary example doesn’t need to care about this problem since less-than operator is only needed when methods are called (due to lazy instantiation).

# Advanced Template

Commonplace Techniques

## Commonplace Techniques

- Commonplace Techniques
- CRTP
- Type Erasure

## CRTP

- Curiously Recurring Template Pattern (CRTP) is a technique for static polymorphism.
- For dynamic polymorphism, we use inheritance with virtual functions and usually need indirect jump by vtable.

- But in template, we can achieve it by defining the same set of members & template function:

## CRTP

- In dynamic polymorphism, we can use template method pattern to reduce implementation code in derived class.
- As we reviewed in Lecture 2…
- In static polymorphism, we can achieve it in this way:

## CRTP

- Of course, you can hide implementation by making base class friend: Type alias as friend doesn’t need “class”.

- What makes it “curious” is that a class can be used as template argument before it has complete definition, and inherits from that template.
- You can also return different types in methods of derived class, which is more powerful than covariance.
- Since C++23, you can use deducing this to simplify CRTP in some cases.

## CRTP

Intuitively, student.GetGPA()  GetGPA(student), so the template parameter can be automatically deduced (i.e. equiv. to student.GetGPA<Student>()).

## CRTP

- But you cannot write something like:
- Since it has dropped the concrete type, such call can only deduce T = StudentBase.

- Solution: Just use template<typename T> void Func(T&).
- And optionally add requires std::derived_from<T, StudentBase>.

## Commonplace Techniques

- Commonplace Techniques
- CRTP
- Type Erasure

## Type Erasure

- We can notice that dynamic & static polymorphism exhibit different pros & cons.

E.g. you can use std::vector<Base*>.

Credit: C++ Templates the Complete Guide, nd 2ed.. i.e. due to lazy instantiation.

## Type Erasure

- To bridge the gap between two polymorphism, type erasure is introduced.
- For example, for std::function, obviously e.g. function pointer cannot be derived from it, so direct dynamic polymorphism fails.
- On the other hand:
- It only records return type & param types instead of fully recording the original type;
- And it can be used as a normal object instead of always requiring template method to call the correct methods. which makes static polymorphism impossible.
- Type erasure is to keep dynamic polymorphism in a non-invasive way (i.e. not requiring direct inheritance).

These names (i.e. façade and proxy) are not official, just my personal naming.

## Type Erasure

- We still use Student example to give an overview, assuming that we just want the class has a GetGPACoeff(), and expose GetGPA().
- There are three key factors in type erasure: #1: A façade that exposes what you want, e.g. GetGPA() here. #2: A proxy abstract base that implements as if it’s a base in dynamic polymorphism; it has virtual functions that you want, e.g. GetGPACoeff() here. #3: A template proxy that inherits from the proxy base, which stores the concrete objects (e.g. Student or JuanWang here) and implements virtual functions.
- And façade needs to have: #4: A storage (e.g. pointer + new) that stores the template proxy while referred by the pointer of proxy base type. #5: A template constructor to accept any possible type to construct the template proxy.

## Type Erasure

- To be specific:
- #2, the proxy base:

- #3, the template proxy:

Of course, if you don’t want two overloads, you can use template<typename T> requires std::same_as<ConcreteStudentType, std::remove_cvref_t<T>> StudentProxy(T&&) and forward.

## Type Erasure

- #1, the façade:

#4 #5

## Type Erasure

- There are still obvious problems in façade, but it’s easy to fix:
1. No dtor to deallocate studentProxy;

2. No proper function to move or copy.
- You need to define clone in the proxy, and call it in façade’s copy assignment & ctor.
- Move assignment & ctor just operates on the pointer, no need to modify proxy.

## Type Erasure

- Of course, we can have lots of variation here:
1. Do we have to use heap memory for storage? We may utilize stack for small objects!
- That’s just what std::function and std::any do, called SBO and SOO respectively.
2. Do we have to use inheritance as “base”? We may drop vtable!
- For example, we may store the function pointer manually, as we done in homework of Lecture 5. Do it if you haven’t!
- We may even split some pointers to be shared among the same type while cache some metadata. That’s what the library proxy does and is faster!
3. etc.…
- We’ll use std::function as homework for you.

## Type Erasure

- To conclude, type erasure is a compromise between two ways:

E.g. std::vector <std::any>.

Between two polymorphism

Depends on impl., like proxy. But never faster than pure static polymorphism.

Depends on impl., but generally it’s impossible (as we said, virtual function may force instantiation, depending on compiler).

## Summary

- NTTP and template template• Now you can implement most of parameterclasses and functions in the standard library yourself!
- Type Deduction
- CTAD and user-defined deduction guide
- Friend
- Lazy instantiation
- Variadic Template
- Fold expression
- SFINAE
- std::conditional
- CRTP & Type Erasure

## Next lecture…

- We’ll talk about basics on parallelism.
- Threads;
- Locking mechanism, semaphores…
- Synchronization mechanism, condition variables…
- High-level abstraction, future-promise model…