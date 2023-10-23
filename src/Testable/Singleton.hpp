#pragma once

#include <exception>
#include <memory>
#include <mutex>
#include <type_traits>

namespace testable
{

template <typename T>
class Singleton
{
public:
    template <typename CreateT, typename ...Args>
    std::enable_if_t<std::is_base_of_v<T, CreateT>, void>
    static create(Args... args)
    {
        std::lock_guard<std::mutex> lock(accessMutex_); // TODO use atomics instead mutexes
        if (!instance_)
        {
            instance_ = std::unique_ptr<CreateT>(new CreateT(std::forward<Args>(args)...));
        }
    }

    static T& getInstance()
    {
        std::lock_guard<std::mutex> lock(accessMutex_);
        if (!instance_)
        {
            throw std::logic_error("Singleton::get should be called after initialization!");
        }
        return *instance_;
    }

    // can be replaced by destructor in case when instance_ is stored and set
    // from some well defined place e.g. main scope 
    // NOTE: redundant if real impl not needed in test and only one instance of mock created
    static void reset()
    {
        std::lock_guard<std::mutex> lock(accessMutex_);
        instance_ = nullptr;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
private:
    inline static std::mutex accessMutex_;
    inline static std::unique_ptr<T> instance_{nullptr};
};

}
