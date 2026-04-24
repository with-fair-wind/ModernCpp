模板基础与移动语义 Template Basics and Move Semantics

## 现代C++基础

## Modern C++ Basics

Jiaming Liang, undergraduate from Peking University

### • Template Basics

- Compile-time Evaluation
- Compile-time Branch Selection
- Specialization
- Overload resolution
- Tricky Details
- Concepts

### • Final part of Move Semantics

- Universal Reference and Perfect Forwarding

# Supplementary

- Before starting, we’d supplement some basic knowledge.
1. Since C++14, it’s allowed to define variable template.
- E.g.

- Though it’s const, it has external linkage (since it’s a template).
- Non-static variable template cannot be defined inside a class.
2. Member function template cannot be virtual.
- Intuitive reason: compiler need to determine the size of vtable; so it needs to know number of virtual functions.
- But function template can be instantiated freely.

# Template Basics

Compile-time Evaluation

# Motivation

- const has two functionalities in C++:
- Not changeable;
- Possibly can be determined in compile time.
- For example:

- Sometimes we hope to force the variable to be determined in compile time.

# constexpr variable

- So we need constexpr!

- constexpr implies const, so it has internal linkage in global field.
- Normally in header file/class it will also be decorated with inline.
- So what if we want the initial value determined in compile time, while make it changeable afterwards?
- That is, constexpr without const!

# constinit

- That’s constinit (since C++20).
- You can only use constinit for global / static / thread-local variables.

- constinit can help to solve static initialization order fiasco.
- That is, the initialization order of global variables in different TUs is not determined; so you cannot let one initialization rely on the other.

# constinit

- Example:

- But constinit ensures by compilers that when it’s used, it’s definitely initialized.
- For non-compile-time initialization, you still need singleton pattern.

# constexpr function

- We may want complex compile-time computation, so we need functions executed in compile time…
- And that’s constexpr function.
- The history:
- C++11 – only one line; constexpr function can only contain a return statement & type aliases & static_assert.
- C++14 – allow multiple lines, e.g. branches like loop & condition;
- C++20 – allow try – catch block, virtual function.
- throwing an exception will lead to compile error.
- C++23 – allow goto, use non-constexpr variables, static & thread-local variable.

Before C++23, constexpr ctor has less requirements than other constexpr functions; but C++23 makes them unified.

# constexpr function

- So C++23 requirements are quite simple:

- For example, we want to know whether a number is prime at compile time.
- In C++11, you have to use recursion to do it in a single return:

# constexpr function

- And in C++14, you can use loop to do it:

# constexpr & consteval function

- Unlike constexpr variables, constexpr functions are allowed to not get the value at the compile time.

But normally it is, due to compiler optimization.

- If you want to force the function to be evaluated at compile time, you need consteval.

# constexpr & consteval lambda

- These two specifiers can also be added in lambda.
- constexpr since C++17, consteval since C++20.
- E.g.

- Notice that if all operations in lambda are possible to be in constexpr, constexpr is implied (so explicit specification can be omitted).
- E.g.

# Template Basics

Compile-time Branch Selection

# Compile-time Branch Selection

- We’ve known branch since we’re novices…
- However, code path selection only happens at runtime, depending on the value of the condition.
- But we’ve already learnt compile-time evaluation…
- Correspondingly, we could choose to execute some code or not at compile time. Non-taken branch can be completely eliminated!
- There are several ways to do so:
- By specialization, so when some conditions are met, only one of the specializations will be chosen.
- And for functions, there is no partial specialization so overload may be needed.
- By control statement in a code block, e.g. constexpr if.

# Template Basics

- Compile-time branch selection
- Function overload resolution and specialization
- Class specialization
- Selection in code block

# Template specialization

- A simple template function:

- What if we want to output “There!” when T is int?
- By specialization!

- From int arg, compilers can deduce the specialized type and we can just write:

# Template specialization

- Note1: don’t mistake it from explicit template instantiation.
- Instantiation: no <> after template.
- Instantiation can also eliminate <int> here since it could be deduced.
- Note2: a specialization must be declared before it’s used, otherwise the behavior is implementation-defined.
- For example, the compiler could generate according to the primary template since it doesn’t see specialization here.
- Or, it could search for specialization globally and use it directly.
- Note3: a full specialization isn’t a template anymore; thus you cannot define it in header file (re-definition).
- You can either use inline, or only write the specialization prototype.

# Template specialization

- You can add new specifiers (e.g. inline, constexpr) since it can be viewed as a new function.
- For class specialization, since class can be written directly in header file, it’s Okay.
- Note4: default template parameter can be omitted when writing specialization.

# Template specialization

- Note5: you can also specialize template member function, but it’s not allowed to be defined inside class.
- *But msvc and clang allow to do so.

- These notes also apply on class template specialization.

# Type Deduction

- Similarly, you don’t need to write <…> when calling it if they could be deduced from the parameters.
- Non-deducible template type parameters may be written first to minimize explicit ones.

Notice that the returned type cannot be deduced from the caller, e.g. double b = Func2<…>(…) cannot deduce V as double.

- By contrast: here T must be specified explicitly.

# Overload and specialization

- But there are some special conditions…
- For example, we want to use some code when “T is pointer”.
- For class, you can use partial specialization; but functions don’t provide it.
- Reason & Solution: functions can use overloads!

- What if we use nullptr as parameter?
- Oops!

# Overload resolution

- So in fact we have two candidates:
- template<typename T> void Func(T); (named as F1)
- template<typename T> void Func(T*); (named as F2)
- When we use int*, F2 is preferred over F1; when we use nullptr, F1 is preferred over F2.
- So there is an inner matching order; that’s overload resolution.

# Overload resolution

- So which function is called is determined in these procedures:
1. Names are looked up to find all possible functions to form an overload set.
- ADL helps in this step but we don’t cover it.
2. Discard illegal functions by judging from their prototypes to form viable function candidates.
- E.g. non-deducible templates;
- E.g. Func(int, double) cannot be called by Func(1) due to wrong parameter number.
3. Perform overload resolution to find the best candidate. If there isn’t the best one, compile error.
4. Check whether the candidate compiles.
- E.g. if it’s =delete, then compile error (yes, it’s not excluded in step2);
- E.g. there is static_assert in function body that’s not satisfied.

# Overload resolution

- To put it simply, overload resolution just tries to find “the most precise one” determined by parameters. The order is:
1. Perfect match or match with minimal adjustments (i.e. decay, add cv- qualifier).
2. Match with promotion, e.g. short->int, float->double.
3. Match with standard conversions (pre-defined ones), e.g. int->short.
- For a conversion sequence (at most s-u-s), match the shorter.
4. Match with user-defined conversions.

# Overload resolution

- If there are still more than one candidates, more rules will apply:
1. More specialized ones are preferred, including considering value category;
2. Non-template ones are preferred than template ones;
3. For pointers, conversion order is: Derived-to-base > void* > bool.
4. For initializer_list, when using universal initialization, it’s preferred over other ones.
- And that’s why std::vector<int>(5, 1) ≠ std::vector<int>{5, 1}.
5. Functors are preferred over surrogate functions (i.e. need conversion to become callable functor).
- …
- It’s very complicated and we don’t cover it more; it you’re interested, see [over.match].

# Overload resolution

- So now we know why:

- int*: both exact match, but F2 is more specialized than F1.
- nullptr: F1 exact match (T is nullptr_t), while F2 doesn’t.
- Exercise: what if we add void Func(int*); as F3?
- int*: F3 > F2 > F1, since non-template is preferred when all exact match.
- nullptr: F1 exact match, F3 needs conversion, so F1 > F3. To use only template, you need to explicitly write Func<…>() (so non- template won’t be a candidate due to syntax error).

# “More specialized”

- One more concern: what is “more specialized”?
- Formally, we say template A is more specialized than B if:
- Hypothesize that there exist concrete types U1, U2, … to substitute all template parameters in A, if it couldn’t be deduced by B, then we say A isn’t more specialized by B.
- “More specialized” is a partial ordering, so maybe neither template is more specialized than the other, which causes ambiguous call.
- Notice that this will only be judged when calling, not when functions are defined.

# “More specialized”

- Example:

- It seems that #2 is more specialized than #1, but:
- #1 from hypothetic #2: for f(U1, int*), #1 will:
- For first parameter, deduce T as U1;
- For second parameter, deduce T as int.
- U1 ≠ int, thus deduction fails -> #2 is not more specialized than #1.
- #2 from hypothetic #1: for f(U1, U1*), #2 will:
- For first parameter, deduce T as U1;
- For second parameter, fail to call -> #1 is not more specialized than #2.
- Thus, ambiguous call: Notice thatf(0.0, double*) isn’t ambiguous since #1 is exact match while #2 isn’t.

# Template Basics

- Compile-time branch selection
- Function overload resolution and specialization
- Class specialization
- Selection in code block

# Class template specialization

- Similarly, you can define full specialization for class:

- Typical example in standard library: std::vector and std::vector<bool>.
- Specialized class is a separate class, which can have completely different data member and member functions.
- You may just see it as a normal class.

No template<> when split member function definition, just like a normal class.

# Class template specialization

- And, you can also define partial specialization!

- Unlike functions, you cannot “overload” a class, like define non-template class A; template<typename T, typename U> class A; etc.
- Matching order is just choosing the most specialized one among all candidates.
- E.g. A<int*> can match both A<T*> and A<T>, but the former is more specialized (formally, you can use a hypothetic type to deduce it).
- A<int> can only match A<T>, so it’s A<T>.

# Partial specialization

- Note1: partial specialization is not allowed to have default template parameter.
- Reason: specialization only determines “whether a type matches it”; it doesn’t determine “what a type is”.
- Example:

- A<int> is determined to be A<int, int> by the primary template; then A<int, int> is judged to match the specialized one.

# Partial specialization

- Note2: NTTP partial specialization cannot depend on other template parameters.
- Forbidden cases:

# Partial specialization

- Note3: variable template can also be specialized.
- Partial specialization of variable template isn’t regulated in the standard but all compilers implement it.
- Particularly, the type of specialized variable can be different from the primary template.
- Note4: partial specialization is allowed to be defined inside the class.
- Example:

# Template Basics

- Compile-time branch selection
- Function overload resolution and specialization
- Class specialization
- Selection in code block

# constexpr if

- Sometimes it’s too troublesome to define all special cases by specialization…
- For example:

- It can’t be better if we can code them together:

# constexpr if

- However, it won’t compile when instantiation.
- E.g. N == 0, then M<0>::value2 is invalid.
- Though this branch is always not taken, but that’s runtime thing!
- What we want: when some compile-time condition isn’t met, the code segment isn’t checked and generated at all.
- That’s constexpr if!
- Since C++17.

- Notice that else if should use constexpr too; only else can omit it.

# constexpr if

- Example: previous homework on variant
- For std::variant<int, double, std::string>, convert to a string.

# consteval if

- There exists a special condition – when it’s evaluated at compile time, do something.
- For example, we want to write a constexpr sin 𝑥.
- At runtime, it’s better to use std::sin directly, which may utilize hardware utility to accelerate. +∞ 𝑖𝑖
- At compile time, we may use Taylor expansion sin 𝑥 = σ−1 𝑥/ 2𝑖 + 1 ! to 𝑖=0 evaluate; it is slow but can at least be evaluated at compile time.
- That’s consteval if since C++23.
- No parentheses!
- Negate: if !consteval {…}.

For more compile-time math function implementations, see C++：constexpr的数学库 - 知乎; C++23 will also make e.g. std::sin to be constexpr directly.

# is_constant_evaluated

- C++20 introduces std::is_constant_evaluated, which is same as:

- Notice that you cannot use if constexpr (std::is_constant_evaluated()), since the condition of if constexpr is always evaluated at compile time, which means it’s always true here.
- Since it can only be used in runtime Cannot be substituted with if branch, it’s less powerful than(std::is_constant_evaluated()); if consteval.

# Template Basics

Tricky Details

# Name lookup

- Names could be divided into two parts:
- Dependent / Non-dependent name: if a name depends on template parameter, then it’s dependent name.
- Qualified / Non-qualified name: if a name is specified by ::, ., ->, then it’s qualified. A fully qualified name is like ::a.b.
- Compilers need to automatically determine the identity of non-qualified name. ADL helps here (but we don’t cover it).
- Since C++ allows name reuse (in different blocks), so it will be looked up upwards.
- In templates, two-phase lookup is performed.
- Non-dependent names are looked up when template is defined.
- Dependent names are looked up when template is instantiated.

# this->

1. When the base class is dependent name, then this-> is needed to access members of base class.
- E.g.

- Reason: a is non-dependent name, so it’s looked up when the template is defined, not when instantiated. Thus return a; will then be returning the global one.
- If the global a is not defined, return a; will cause compilation error.

# Two-phase lookup

- Why don’t we lookup all names when templates are defined?
- Reason: template may be specialized afterwards, which needs lookup in the second phase to know whether the identity exists.
- For example: Then define

You cannot know whether a exists when B is defined!
- Thus C++ requires to access data member by this-> in this case; this is dependent so its name lookup is performed in the second stage.

# Two-phase lookup

- Why don’t we lookup all names when templates are instantiated?
- We can, but C++ hopes to expose syntax error as early as possible. Thus statements with only non-dependent names can be checked directly without instantiation.

- Notice that some compilers may check all things only at instantiation, e.g. msvc.
- Since VS2017 (msvc 15.3), msvc adds /permissive- to enable two-phase lookup.

# typename

2. A::B could be either a type or a variable.
- For non-dependent names, it could be easily determined; but for dependent names, it’s still ambiguous…

int* b in Test. 1 * b in Test.

# typename

- C++ chooses to always regard it as variable!
- When it’s actually type, you need to add a typename.
- For example:

Dependent name

- Note: You cannot have an identity that’s possibly data or types; it must be determined before instantiation.

# typename

- To be specific, you can use typename when:
- The type is a qualified name;
- It’s not after keywords class/struct/union/enum;
- It’s not Base class appears at inheritance specification and ctor.
- i.e. class A: typename B<T> is wrong;
- A(int a) : typename B<T>{a} {} is wrong.
- And you must use typename when:
- Rules above;
- The type is a dependent name;
- It’s not the current instantiation.
- i.e. just the current type itself.

# typename

- Exercise: which typename is wrong / correct but unnecessary / correct & must?

# typename

- Since C++20, many rules are relaxed.
- To be short, when a dependent name appears where only type is possible, typename can be omitted.
- To be specific:
- Return type of functions and lambda;
- Aliasing declarations, e.g. using Type = A<U>::Type;
- Target type of C++-style cast (e.g. static_cast<A<U>::Type>(…));
- Type of new expression (e.g. new A<U>::Type{1});
- Parameter type in requires expression, covered later.
- Data member type, NTTP type;
- Parameter types of member function & lambda;
- Default value of template type parameter (e.g. template <typename T, typename U = A<T>::Type>)

# typename

- Notice that member function parameter and global function parameter differ here.

Okay too.

# typename

- A full example adopted from C++20 – the Complete Guide.

Rarely used.

# template

3. Template parameter specification is also ambiguous…
- E.g. std::function<int()> f;
- It could be parsed as (std::function < int()) > f, where std::function and f are interpreted as variables.

- So how to parse is determined by the identity again (whether it’s a template or not).
- C++ regulates that if the name is a template, < is always interpreted as the beginning of parameter specification; otherwise less-than operator.

# template

- Again, dependent name cannot determine its identity…
- So it will always be interpreted as less-than operator; when it’s actually a template, you need to use template keyword explicitly.

- Here n is dependent and thus to_string cannot know whether it’s a template. When < follows, it would be interpreted as less-than.

# template

- Another horrible example:

This typename can be omitted since C++20, as it’s parameter of member function.

nd Credit: C++ Templates – The Complete Guide 2ed. by David Vandevoorde, Nicolai M. Josuttis, Douglas Gregor

# template

- Note1: in template parameter specification, the first closing > will always be interpreted as the ending.
- It’s parsed as M<a> b > m.
- You need additional parentheses to make it right:

- Note2: nested template (e.g. vector<vector<int>>) needs additional space (i.e. int> >) before C++11, to prevent ambiguity with operator >>.
- Since C++11, it’s also specially regulated.

# template

- Note3: C++ exists digraph and trigraph (e.g. <: equiv. to [), which makes e.g. S<::i> ambiguous.
- Since C++11, it’s specially regulated that <:: is never treated as <: + : (which makes it [:), but as a whole.
- And since C++17, trigraph is removed.

# Nested Specialization*

- Sometimes we may specialize a template in a template class…
- It has many restrictions and is not always possible.
- Very complicated and not commonly used, so we make it optional.
1. A fully specialized template cannot be defined in a not-fully specialized enclosing template.

Primary template & partially specialized template aren’t allowed.

# Nested Specialization*

- But when a fully specialized enclosing class is explicitly defined, this template<> isn’t needed anymore…
- As we’ve said, fully specialized class is just a normal class.

- template<> is only needed when explicit specialization isn’t specified.
- Similarly, to define func here:
- Since B<int> is already a normal class.

# Nested Specialization*

2. Partial specialization can be normally defined regardless of the specialization status of the enclosing class.

- So sometimes to bypass the full-specialization restriction, you could add a dummy type parameter.

# Template Basics

Concept

# Template Basics

- Concept
- require clause and concept
- Subsumption
- Some exercises on concept

# Motivation

- Templates are good at reducing code replication.
- What if we instantiate it by some unintended types?
- For example:

- E.g. We don’t want users to compare pointers.
- It will also report error when T isn’t comparable, and users have to read the full source code / the error message to know all illegal operations…
- So why not add explicit constraints on types? Then users just need to read these constraints directly!

# Concept

- That’s what concept for.
- Before C++20, users can add constraint in an obscure way called “SFINAE”, which is miserable for both users to read and programmers to write.
- Let’s show an example for concept directly:

The parentheses are necessaryrequires clause when it’s an expression instead of a pure value.

# Concept

- Any template parameter can be constrained by requires clause:

- You can also connect multiple constraints by && and ||.

- But we may hope to reuse these constraints, instead of writing it over and over again…

# Concept

- So we can declare a concept!

- That’s still naïve; we need more constraints…
- For example, what kind of expressions should be legal?
- Then you need requires expression.

# Concept

- For example:

- It checks whether x < x is legal, i.e. whether T can be compared by <. It never uses its runtime result.
- T x is just a hypothetic parameter; it doesn’t matter whether it has an accessible ctor. If parameters aren’t needed, requires { … } is enough.
- You can also combine a requires expression with logical operators.

# Concept

- Sometimes we just use a concept once, so we can insert it into the requires clause anonymously.

requires expression requires clause

- Such constraint is same as requires MyLessThan<T>.
- Small exercise: can we combine two concepts like

# Concept

- Nope.
- As we’ve said, it just checks whether the expression is legal, instead of whether it’s true.
- E.g. sizeof(char) >= 8 results in false, but it’s legal to write this expression. So it doesn’t check anything.
- Similarly, this doesn’t check anything either:

- Remember: it doesn’t check something valid but not true!

# Requires expression

- So what we write before in requires expression is called simple requirement.
- Beyond that, there exist other three types of requirements.
- Type requirement: typename xxx; satisfy
- E.g.

- Notice that if typename isn’t added, then it’s simple requirement that checks whether there exists a static data member.

satisfy

# Requires expression

- Compound requirement: check whether the expression is legal and whether the type of result satisfies some constraints.

- The noexcept requires the operation to be noexcept.
- And std::same_as<bool> is equivalent to std::same_as<decltype(x < x), bool>; the result type will be fed as the first type parameter directly.

# Requires expression

- Nested requirement: requires BooleanExpression;
- This checks both whether the expression is legal and whether it’s true.

# Concept

- Note1: for any type deduction & template parameter, you can add a single constraint by abbr.:

When the deduced type doesn’t match the constraint, compile error.

The deduced parameter will be the first parameter of the concept, same as compound requirements. Notice that this is equivalent to template<MyMaxConstraint T, MyMaxConstraint U> auto Max(const T& a, const U& b)

# Concept

- You can also combine it with requires clause:
- Equivalent to combine with &&.

- Note2: concept can be used as boolean expression.
- The result is whether the concept is fulfilled.

- Note3: NTTP can also use concept:
- It cannot use abbr.

# Concept

- Note4: concept only adds syntactic constraints, not semantic constraints.
- E.g. “.size() should be 𝑂(1)”; this cannot be checked by compilers (halting problem).
- Semantic constraints should be documented explicitly.
- E.g. in the standard library, std::invocable<F, Args…> and std::regular_invocable<F, Args…>.
- They have the same syntactic constraints, i.e. check whether F is callable with Args.
- However, the latter has semantic constraints equality preserving; that is, it shouldn’t change the status of function objects and parameters.
- Concept cannot check whether a callable satisfies it, so they’re equivalent from the compiler’s view; the semantic difference is only documented.

# Concept

- Sometimes, the semantic constraint is added by manual tags.
- The most typical one is iterator categories; you cannot distinguish contiguous iterators with random iterators syntactically.
- That is, you cannot know whether continuous memory is occupied by iterators.
- So, standard library just adds a type alias iterator_concept since C++20.
- For example, for iterator of vector, using iterator_concept = std::contiguous_iterator_tag.
- Before C++20, it’s iterator_category = std::random_iterator_tag; that’s not accurate so it’s recommended to:

Notice that requirements of iterators are also slightly changed in C++20, e.g. input iterators is allowed to not provide copying.

# Concept

- Note5: There exist lots of concepts in the standard library.
- As we’ve seen before, std::same_as<T, U>.
- We’re not going to talk about them in details; basically the functionality can be deduced from the name.

Credit: C++20 – The Complete Guide by Nicolai M. Josuttis. For more details, see Chapter 5.

You can borrow it from PKU library.

# Template Basics

- Concept
- require clause and concept
- Subsumption
- Some exercises on concept

# Concept subsumption

- We know that specialization just means “implement a different version for special cases”.
- While concepts just describe “some special cases”.
- So of course we can use concept to do specialization!

Is specialization of
- For example:
- But specialization has matching order; the “most specialized” one will be the selected one among all templates.
- So if we add multiple specializations by concept, we need to know their order.
- For example:

More specialized than

# Concept subsumption

- So how is the concept order determined?
- We can notice that concept is just logical expressions.
- Some atomic constraints, with &&, ||, !.
- And logical expressions can imply（蕴含）!
- E.g. a && b → a.
- So generally, concept A is more specialized than / subsumes concept B  logical expressions A imply expressions B.
- But, implication is not easy to deduce.
- From mathematical logic, we know 𝑎 → 𝑏 can be transformed as 𝑎 ^ ¬𝑏.
- Assuming that for any type 𝑇, 𝑎𝑇 , 𝑏(𝑇) may be true or false freely, then it 𝑖 𝑗 will be SAT problem to judge whether it could be satisfied. That’s NP- complete!

# Concept subsumption

- So to solve that, compilers don’t do full logical judgement; instead, it only considers equivalence by concept name.
- Non-concepts (including !Concept) will be always considered not related.
- So we could build a concept subsumption graph;
- And A subsumes B  graph B is subgraph of graph A.
- For example:

Unknown0

IsPointer<T> Unknown1Not a concept, so always unrelated.

IsPointer is subgraph of the latter. Anonymous1<T>

# Concept subsumption

- What if:

- We can deduce the first one is more specialized than the second one, but compilers don’t know. Unknown1 and Unknown2 are always
- Graph: seen as unrelated, so they don’t subsume.

IsPointer<T> Unknown1 Unknown2

Anonymous1<T> Anonymous2<T>

# Concept subsumption

- Part of subsumption graph of concepts in the standard library:

Credit: C++20 – The Complete Guide by Nicolai M. Josuttis.

# Concept subsumption

- Exercise: implement communicative concept SameAs by type traits std::is_same_v.
- Is it correct to use:

- No, since it’s not communicative, SameAs<T, U> isn’t considered equivalent as SameAs<U, T>.
- Now is it correct?

- No, since non-concepts will always be considered unrelated; the atomic constraint is just SameAs<T, U> and SameAs<U, T>, which is considered not equivalent.
- Solution:

# Concept subsumption

- Now, the atomic constraint is SameAsHelper, so the graph is like:

SameAsHelper<T, U> SameAsHelper<U, T>

SameAs<T, U> SameAs<U, T>

- So SameAs<T, U> and Same<U, T> have the same graph, and they’re considered equivalent!

# Concept subsumption

- Concept also matters in overloading resolution (“more specialized”).
- When a concept subsumes another, then it’s preferred in resolution.
- For example:

# “More specialized”

- Notice that the second one cannot be changed as Max(T a, T b) since it breaks “more specialized”. If you want it, you need:

- Reason: when both of two candidates are not more specialized than the other with rules before, more rules kick in:
- If their template parameters or function parameters differ in length, ambiguous;
- Otherwise, if template parameters are not equivalent or function parameters are not of same type, ambiguous;
- Otherwise, if one template is more constrained (determined by concept subsumption) than the other, it’s regarded as more specialized. Notice that the actual rules are slightly more complex since some function call can be reordered (e.g. a == b can be rewritten as b == a to make it compile since C++20), don’t cover it here.

# “More specialized”

- Now you can explain why we need additional constraint before!

- We know that and are not more specialized than the other when calling…
- So they’re first checked by parameter length, which is same.
- Then their parameter forms are checked; though template parameters are equivalent, but functions parameter are not, so ambiguous.
- Concepts aren’t checked yet, so IsPointer<T> doesn’t help to make the function “more specialized”.

# Template Basics

- Concept
- require clause and concept
- Subsumption
- Some exercises on concept

# Concept

- By concepts, we could do more interesting things.
- E.g. more special compile-time computation; more type traits.
- Exercise1: implement std::is_nothrow_move_constructible and std::is_nothrow_move_assignable by concept.

# Concept

- Exercise2: utilize concept & class specialization to write IsPrime.
- Consider: Isn’t it enough to implement it like this? Where is concept and specialization?

# Concept

- Let’s try it:

- Reason: expressions need to be checked whether it’s valid.
- IsPrime<4> → DoIsPrime<4,2> → DoIsPrime<4,1>
- It should stop now, but there is no short circuit for expression validity check, so it will continue to instantiate…
- DoIsPrime<4,1> → DoIsPrime<4,0> → DoIsPrime<4,(unsigned)-1> → …
- So the recursion never stops.
- To make it stop early, we need specialization; only the most specialized one will be instantiated.

# Concept

- Solution:

IsPrime<4> → DoIsPrime<4,2> → DoIsPrime<4,1>

And then only the most specialized one is instantiated, so only the second one is used. No infinite recursion!

# Move Semantics

Universal Reference and Perfect Forwarding

# Move semantics

- Universal Reference and Perfect Forwarding
- Basics
- Reference Collapsing Rule

# Motivation of Universal Reference

- Consider a function that doesn’t care about the reference type, but just wants to transfer as is to a new function.
- E.g. std::vector::emplace_back(args…); it should just transfer args to the ctor and maintain its reference type. 𝑁
- Each parameter makes it need to overload for 3 or 4 times, so it needs 4 overloads, what??
- And template is an important way to reduce replicate code.
- So, can we use template to keep the type of reference?
- That is universal reference!

Note: universal reference is called “forwarding reference” in C++ standard. Universal reference is named by Scott Meyers before entering the draft formally and widely used.

# Universal Reference

- Example:

- Yes, when && cooperates with template type parameter directly, it’s never rvalue reference, but universal reference.

- When accepting lvalue of type MyType, T&& is deduced as MyType&; const lvalue as const MyType&; rvalue as MyType&&.
- For example:

# Universal Reference

- Note: T&& is universal reference only when T is exactly a direct template type parameter.
- Let’s see some misleading cases…
1. The type is not the parameter itself.
- Example1:

- value_type is a normal type under T, so this is just rvalue reference of it.
- Example2: try to specialize universal reference?
- This specialization is just rvalue reference of std::string, not universal reference of std::string!
- i.e. it only accepts rvalue.

# Universal Reference

2. The template type isn’t provided by the function directly.
- Example: template class
- When instantiating, T will be assigned to a specific type, so T&& will be rvalue reference of T.

3. The reference specifier isn’t &&, e.g. T& and T.
4. It contains const and volatile (cv-qualifiers).
- const T&& only automatically deduces const rvalue reference of T.

# Universal Reference

- A special form of universal reference is auto&&.
- For template:
- We’ve said auto is short for a new template type parameter.
- For type deduction, it’s still universal reference!

# Universal Reference

- Exercise: write a template function that accepts vector<T>& and assigns every element to the default constructed one.
- Is it correct?

- There exists a special case: std::vector<bool>…
- We’ve said iterating over it will get a proxy that represents a bit.
- And that proxy is of value type, which cannot be referred by auto&.
- Solution: use auto&&/decltype(auto).
- Then it will either be deduced as rvalue reference or value.

# Perfect forwarding

- Now we can accept any reference, but we still need to transfer it as is.
- However, arg is always lvalue, so func_ cannot know its original type.
- Of course, we can use if constexpr to do it manually.
- But it can’t be better if we can process uniformly…

- Target: make arg to have type decltype(arg) again.
- That is what perfect forwarding for!

# Perfect forwarding

- std::forward<T> is essentially std::move for rvalue reference, and no effects for lvalue reference.
- Usually we use std::forward when you don’t distinguish lvalue reference from rvalue reference (so you cannot tell whether it’s a std::move).

- Note1: universal reference is always a reference; it cannot be a value type.
- Note2: <T> in std::forward<T> is necessary; it cannot be deduced automatically.
- The reason will be covered later.

# Overload resolution on references

- How is overload resolution changed when universal reference is considered?
- Universal reference is always second-best choice.
- A thorough summary of overload resolution (v is lvalue while c is const lvalue):

# Overload resolution on references

- ATTENTION: fallback will be disabled if universal reference is used!
- That is, non-const lvalue and const rvalue cannot fallback on const& in copy ctor.
- Example: implement std::any.
- You can use any type for its ctor, so it’s like:

- Problem: non-const lvalue & const rvalue won’t call copy & move ctor…

# Overload resolution on references

- Solution: still by concept!

- But in some cases it’s still problematic; e.g. if some class inherits Any, then calling copy/move ctor in inheritance (e.g. Base{ std::move(another) }) is still wrong…
- Anyway, pay special attention to universal reference ctor!
- Similarly, for any function with universal reference parameter, it’s usually not a good idea to overload.
- That will disable many implicit conversions since universal reference is an exact match.
- e.g. define a std::string overload, but const char[] doesn’t use it.

# Move semantics

- Universal Reference and Perfect Forwarding
- Basics
- Reference Collapsing Rule

# Reference Collapsing

- We’ve known what T&& is when matching different expressions, but what is T?
- The standard regulates the deduction should be:
- For rvalues, T is the value type (keep cv-qualifiers).
- For lvalues, T is lvalue reference (keep cv-qualifiers).
- Wait, what’s T& &&?
- Reference Collapsing Rule: when multiple reference appears at the same time,
- If all references are rvalue reference, the collapsed reference is rvalue reference.
- If there is at least one lvalue reference, the collapsed reference is lvalue reference.

# Reference Collapsing

- Example: how is std::move implemented?
- all references -> rvalue reference, keep cv-qualifier
- static_cast<RightReference>(exp);

- So is it like this?

- For lvalue, T is deduced as lvalue reference, and so T&& is still lvalue reference, that’s wrong.
- So, we need to strip out the reference and add &&, then it’s always rvalue reference.

# Reference Collapsing

- Solution:
- You can also use decltype(auto) in return type.

- Exercise: implement std::forward<T>.
- First analyze why <T> is always needed?
- What if we just call std::forward(arg)?
- arg is lvalue, thus caller cannot know its original reference type…
- While T keeps that!
- lvalue -> T is lvalue reference -> std::forward<T> should do nothing.
- rvalue -> T is value type -> std::forward<T> should convert arg to rvalue ref.

# Reference Collapsing

- Answer:

- And there is a simpler solution!

- BTW, users basically just call std::forward<T>(arg), but to prevent users from calling e.g. std::forward<T>(std::move(arg)), it provides an overload on rvalue reference, which is same as std::move.

# Reference Collapsing

- Note1: pay attention to deduction conflicts.
- Example: Is it legal to insert(v, s);?

- No. From v, T is deduced as std::string; but from s, T is deduced as std::string&, which causes conflicts.
- Solution: use std::vector<std::remove_reference_t<T>>& or two template type parameters, e.g. T1 and T2.

# Reference Collapsing

- Note2: how should auto&& arg be prefectly forwarded?
- We’ve said std::forward<T> is same as static_cast<T&&>…

- When arg is rvalue reference T0&&, decltype(arg) is T0&&.
- So reference collapsing tells us that static_cast<T&&> = static_cast<T0&& &&>  static_cast<T0&&>, which converts it to rvalue reference (and thus normally move it).
- When arg is lvalue reference T0&, decltype(arg) is T0&.
- static_cast<T&&> = static_cast<T0& &&>  static_cast<T0&>, which converts it to lvalue reference (and thus have no effect).

# Deducing this

- We can also use universal reference on deducing this.
- For wrapper classes, e.g. std::optional, when we call .value(), it will return reference to the underlying value.
- For rvalue std::optional, return rvalue reference;
- For lvalue std::optional, return lvalue reference.
- Before C++23, we need to write four overloads by reference qualifier.

- By deducing this, it could be simplified as:

If self is rvalue reference, then std::forward<T> (self) is same as std::move(self), which creates xvalue; and data member of xvalue is still xvalue, so it’s right.

# Summary

- In this lecture, we’ve talked about:
- For basic templates part:
- constexpr function / variable, consteval function, constinit variable;
- Full specialization, partial specialization, overload resolution;
- “More specialized”
- constexpr if, consteval if;
- Tricky details, including this->, typename, template and nested specialization.
- Concepts
- Requires clause;
- Requires expression (simple, type, compound, nested);
- Concept subsumption
- Concept-based specialization & overloading

# Summary

- For move semantics part:
- Universal reference, its usage and some misleading non-universal reference examples.
- Pay attention to ctor with universal reference; concept is needed to add constraint. (Functions with universal reference should avoid function overloading or at least use with extreme care).
- Perfect forwarding.
- Overload resolution of references.
- How std::move and std::forward are implemented.
- Reference collapsing rules.
- Deducing this

# Next lecture…

- We’ll go into more advanced topics about template.
- More about template parameter that’s not a type;
- More about type deduction;
- Friend in class template;
- Variadic templates and unpacking;
- SFINAE
- Type erasure
- EBCO
- CRTP
- That’s a lot…