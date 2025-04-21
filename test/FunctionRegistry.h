#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <memory>
#include <tuple>
#include <type_traits>
#include <stdexcept>
#include <utility>

// -------------------------------
// 1. 抽象基类：用于类型擦除
// -------------------------------
struct CallableBase
{
    virtual ~CallableBase() = default;
};

// -------------------------------
// 2. 实现类：封装具体函数对象
// -------------------------------
template <typename Ret, typename... Args>
class CallableImpl : public CallableBase
{
    std::function<Ret(Args...)> func;

public:
    explicit CallableImpl(std::function<Ret(Args...)> f)
        : func(std::move(f)) {}

    // 非 void 返回值调用
    template <typename R = Ret>
    std::enable_if_t<!std::is_void<R>::value, R>
    call(Args &&...args)
    {
        return func(std::forward<Args>(args)...);
    }

    // void 返回值调用
    template <typename R = Ret>
    std::enable_if_t<std::is_void<R>::value, void>
    call(Args &&...args)
    {
        func(std::forward<Args>(args)...);
    }
};

// -------------------------------
// 3. FunctionTraits：提取函数签名信息
// -------------------------------

// 针对函数指针
template <typename T>
struct FunctionTraits;

template <typename Ret, typename... Args>
struct FunctionTraits<Ret (*)(Args...)>
{
    using ReturnType = Ret;
    using ArgsTuple = std::tuple<Args...>;
};

// 针对 std::function
template <typename Ret, typename... Args>
struct FunctionTraits<std::function<Ret(Args...)>>
{
    using ReturnType = Ret;
    using ArgsTuple = std::tuple<Args...>;
};

// 针对 Lambda 与其他可调用对象：通过 &operator() 提取
template <typename Callable>
struct FunctionTraits : FunctionTraits<decltype(&Callable::operator())>
{
};

// 针对 Lambda 的 operator()（常量成员函数版本）
template <typename ClassType, typename Ret, typename... Args>
struct FunctionTraits<Ret (ClassType::*)(Args...) const>
{
    using ReturnType = Ret;
    using ArgsTuple = std::tuple<Args...>;
};

template <typename ClassType, typename Ret, typename... Args>
struct FunctionTraits<Ret (ClassType::*)(Args...)>
{
    using ReturnType = Ret;
    using ArgsTuple = std::tuple<Args...>;
};

// -------------------------------
// 4. FunctionRegistry：函数注册与调用平台
// -------------------------------
class FunctionRegistry
{
    std::unordered_map<std::string, std::shared_ptr<CallableBase>> funcMap;

public:
    // 4.1 显式注册：当已知函数签名且提供 std::function 时使用
    template <typename Ret, typename... Args>
    void registerFunc(const std::string &name, std::function<Ret(Args...)> func)
    {
        funcMap[name] = std::make_shared<CallableImpl<Ret, Args...>>(std::move(func));
    }

    // 4.2 自动推导注册：支持函数指针、lambda、函数对象
    template <typename Func>
    void registerFunc(const std::string &name, Func &&f)
    {
        using Traits = FunctionTraits<std::decay_t<Func>>;
        using Ret = typename Traits::ReturnType;
        // 此处仅调用下面针对 tuple 的重载，不需要构造 tuple 实例
        registerFunc<Ret, Func>(name, std::forward<Func>(f), static_cast<typename Traits::ArgsTuple *>(nullptr));
    }

    template <typename Ret, typename... Args>
    Ret invoke(const std::string &name, Args &&...args)
    {
        auto it = funcMap.find(name);
        if (it == funcMap.end())
            throw std::runtime_error("Function not found: " + name);
        auto derived = std::static_pointer_cast<CallableImpl<Ret, Args...>>(it->second);
        return derived->call(std::forward<Args>(args)...);
    }

// 4.3 invoke：调用函数，利用 std::invoke_result_t 自动推导返回类型
#if 1
    template <typename FuncSig, typename... Args>
    auto invoke(const std::string &name, Args &&...args)
        -> std::invoke_result_t<FuncSig, Args...>
    {
        using Ret = std::invoke_result_t<FuncSig, Args...>;
        if (funcMap.find(name) == funcMap.end())
            throw std::runtime_error("Function not found: " + name);
        // 注意：调用时要求注册时传入的参数类型和调用时一致，否则 static_pointer_cast 会失败
        auto derived = std::static_pointer_cast<CallableImpl<Ret, Args...>>(funcMap[name]);
        return derived->call(std::forward<Args>(args)...);
    }
#else
    template <typename FuncSig, typename... Args>
    auto invoke(const std::string &name, Args &&...args)
        -> std::invoke_result_t<FuncSig, std::decay_t<Args>...>
    {
        using Ret = std::invoke_result_t<FuncSig, std::decay_t<Args>...>;
        auto it = funcMap.find(name);
        if (it == funcMap.end())
            throw std::runtime_error("Function not found: " + name);
        // 这里使用 decay 后的类型，如果注册时保存时也做了类似处理，类型匹配更容易保证
        auto derived = std::static_pointer_cast<CallableImpl<Ret, std::decay_t<Args>...>>(it->second);
        return derived->call(std::forward<Args>(args)...);
    }
#endif

private:
    // 4.4 参数展开注册实现：针对 tuple 类型进行展开
    template <typename Ret, typename Func, typename... Args>
    void registerFunc(const std::string &name, Func &&f, std::tuple<Args...> * /*dummy*/)
    {
        std::function<Ret(Args...)> func = std::forward<Func>(f);
        registerFunc<Ret, Args...>(name, std::move(func));
    }
};