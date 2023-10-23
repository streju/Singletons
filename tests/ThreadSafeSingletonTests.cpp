#include <gtest/gtest.h>

#include "ThreadSafe/Singleton.hpp"
#include <vector>
#include <thread>

namespace tests
{

namespace test_1
{
    class CustomCtor : public thread_safe::SingletonCustomCtor<CustomCtor>
    {
        friend class thread_safe::Singleton<CustomCtor, true>;
    public:
        int i_;
        double d_;
    protected:
        CustomCtor(int i, double d) : i_(i), d_(d) {}
        CustomCtor() = default;
    };

    class DefaultCtor : public thread_safe::Singleton<DefaultCtor>
    {
        friend class thread_safe::Singleton<DefaultCtor>;
    protected:
        DefaultCtor() = default;
    public:
        int c_;
    };
}

TEST(ThreadSafeSingletion, BasicUseCase)
{
    using namespace test_1;
    ASSERT_THROW(CustomCtor::getInstance(), thread_safe::SingletonAccessExc);
    ASSERT_NO_THROW(DefaultCtor::getInstance());

    CustomCtor::create(1, 3);
    ASSERT_EQ(CustomCtor::getInstance().i_, 1);
    ASSERT_EQ(CustomCtor::getInstance().d_, 3);

    auto& custom = CustomCtor::getInstance();
    // auto custom2 = custom; // compilation error - deleted copy ctor and operator
    // auto custom2 = std::move(custom); // compilation error - implicitly deleted move ctor and operator
    CustomCtor::create(3, 4); // does nothing

    auto& custom3 = CustomCtor::getInstance();
    ASSERT_EQ(&custom, &custom3);

    CustomCtor::create(); // does nothing
    ASSERT_EQ(&custom, &CustomCtor::getInstance());

    // DefaultCtor::create(); // compilation error - SFINAE
    auto& defaultCtor = DefaultCtor::getInstance();
    defaultCtor.c_ = 11;
    DefaultCtor::getInstance().c_ = 42;
    ASSERT_EQ(DefaultCtor::getInstance().c_, defaultCtor.c_);
}

namespace test_2
{
    class CustomCtor : public thread_safe::SingletonCustomCtor<CustomCtor>
    {
        friend class thread_safe::Singleton<CustomCtor, true>;
    public:
        inline static int counter_{0};
    protected:
        CustomCtor(int, double) {counter_++;}
    };

    class DefaultCtor : public thread_safe::Singleton<DefaultCtor>
    {
        friend class thread_safe::Singleton<DefaultCtor>;
    protected:
        DefaultCtor() {counter_++;}
    public:
        inline static int counter_{0};
    };
}

TEST(ThreadSafeSingletion, ConcurrentInitialization)
{
    using namespace test_2;
    std::vector<std::thread> threads;
    { // custom ctor check
        int i;
        double d;
        const auto customCtorsAction = [&i, &d]{
            CustomCtor::create(i++, d++);
            ASSERT_NO_THROW(CustomCtor::getInstance());
        };
        for (unsigned i = 0; i < 50; i++)
        {
            threads.emplace_back(customCtorsAction);
        }
        for(auto& thread : threads)
        {
            thread.join();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        ASSERT_EQ(CustomCtor::getInstance().counter_, 1);
    }
    threads.clear();
    { // default ctor check
        for (unsigned i = 0; i < 100; i++)
        {
            threads.emplace_back([]{
                ASSERT_NO_THROW(DefaultCtor::getInstance());
            });
        }
        for(auto& thread : threads)
        {
            thread.join();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        ASSERT_EQ(DefaultCtor::getInstance().counter_, 1);
    }
}

}