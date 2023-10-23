#pragma once

#include <exception>
#include <memory>
#include <mutex>
#include <type_traits>

namespace thread_safe
{

struct SingletonAccessExc : std::exception
{
    const char* what() const noexcept override
    {
        return "Singleton::getInstance should be called after Singleton::create method!";
    };
};

template <typename T, bool IsCustomConstructible = false>
class Singleton
{
    struct data
    {
        inline static std::once_flag initializationFlag_;
        inline static std::atomic_bool isInitialized{false};
        inline static std::unique_ptr<T> instance_{nullptr};
    };
    struct empty {};

    public:
    template <typename T_ = T>
    typename std::enable_if<!T_::isCustomConstructible_, T&>::type
    static getInstance()
    {
        static std::unique_ptr<T> instance{new T};
        return *instance;
    }

    template <typename T_ = T, typename ...Args>
    typename std::enable_if<T_::isCustomConstructible_, void>::type
    static create(Args...args)
    {
        std::call_once(data_.initializationFlag_,
            [](Args...args)
            {
                data_.instance_ = std::unique_ptr<T>(new T(std::forward<Args>(args)...));
                data_.isInitialized.store(true);
            }, std::forward<Args>(args)...);
    }

    template <typename T_ = T>
    typename std::enable_if<T_::isCustomConstructible_, T&>::type
    static getInstance()
    {
        if (data_.isInitialized.load())
        {
            return *data_.instance_;
        }
        throw SingletonAccessExc();
        return *data_.instance_;
    }

    protected:
    Singleton() = default;
    ~Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    private:
    constexpr static bool isCustomConstructible_{IsCustomConstructible};
    inline static std::conditional_t<IsCustomConstructible, data, empty> data_; // used only by custom constructible types, empty struct otherwise
};

template <typename T>
struct SingletonCustomCtor : public Singleton<T, true> {};
}
