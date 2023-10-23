#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Testable/Singleton.hpp"

#include <thread>

namespace tests
{

namespace
{
    class ISingleObj : public testable::Singleton<ISingleObj>
    {
    public:
        virtual bool doSmth() = 0;
        virtual ~ISingleObj() = default;
    };

    class RealSingleObjImpl : public ISingleObj
    {
        friend class Singleton<ISingleObj>;
    public:
        bool doSmth() override { return true; }
        unsigned getCounter() { return counter_; }
        static void resetCounter() { counter_ = 0; }
    private:
        RealSingleObjImpl(int, int) { counter_++;}
        RealSingleObjImpl() { counter_++;};
        inline static unsigned counter_{0};
    };

    struct Dummy {};
}

TEST(TestableSingleton, CheckUserOfSingleton)
{
    // ISingleObj::create<Dummy>(); compilation error - SFINAE - Dummy is not a base of ISingleObj

    RealSingleObjImpl::create<RealSingleObjImpl>(223, 123);

    ASSERT_NO_THROW(ISingleObj::getInstance().doSmth());

    auto& obj = ISingleObj::getInstance();
    // auto obj2 = obj; // compilation error - deleted copy ctor and operator
    // auto obj2 = std::move(obj); // compilation error - implicitly deleted move ctor and operator
    RealSingleObjImpl::create<RealSingleObjImpl>(3, 4); // does nothing

    auto& obj3 = ISingleObj::getInstance();
    ASSERT_EQ(&obj, &obj3);

    RealSingleObjImpl::create<RealSingleObjImpl>(); // does nothing
    ASSERT_EQ(&obj, &ISingleObj::getInstance());

    ASSERT_EQ(dynamic_cast<RealSingleObjImpl&>(ISingleObj::getInstance()).getCounter(), 1);

    ISingleObj::getInstance().reset();
    ASSERT_THROW(ISingleObj::getInstance().doSmth(), std::logic_error);

    RealSingleObjImpl::create<RealSingleObjImpl>();
    ASSERT_TRUE(ISingleObj::getInstance().doSmth());

    ASSERT_EQ(dynamic_cast<RealSingleObjImpl&>(ISingleObj::getInstance()).getCounter(), 2);

    ISingleObj::getInstance().reset();
}

TEST(TestableSingleton, ConcurrentInitialization)
{
    RealSingleObjImpl::resetCounter();

    std::vector<std::thread> threads;
    {
        int i;
        double d;
        const auto action = [&i, &d]{
            RealSingleObjImpl::create<RealSingleObjImpl>(i++, d++);
            ASSERT_NO_THROW(ISingleObj::getInstance());
        };
        for (unsigned i = 0; i < 50; i++)
        {
            threads.emplace_back(action);
        }
        for(auto& thread : threads)
        {
            thread.join();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        ASSERT_EQ(dynamic_cast<RealSingleObjImpl&>(ISingleObj::getInstance()).getCounter(), 1);
    }
}

}