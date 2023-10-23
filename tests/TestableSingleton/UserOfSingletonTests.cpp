#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Testable/Singleton.hpp"

namespace tests
{

namespace
{
    class IDummy : public testable::Singleton<IDummy>
    {
    public:
        virtual bool doSmth() = 0;
        virtual int foo(int, double) = 0;
        virtual ~IDummy() = default;
    };

    class Dummy : public IDummy
    {
        friend class Singleton<IDummy>;
    public:
        bool doSmth() override { return true; }
        int foo (int i, double) override { return i; }
    private:
        Dummy() = default;
    };

    class UserOfDummy
    {
    public:
        bool useDummy() { return IDummy::getInstance().doSmth(); }
        int callFoo(int i, double d) { return IDummy::getInstance().foo(i, d); }
    };

    class DummyMock : public IDummy 
    {
    public:
        MOCK_METHOD0(doSmth, bool());
        MOCK_METHOD2(foo, int(int, double));
    };

    template <typename T>
    class SignletonBaseMock
    {
        public:
        template <typename ...Args>
        SignletonBaseMock(Args...args)
        {
            T::template create<T, Args...>(std::forward<Args>(args)...);
        }
        T& get() { return dynamic_cast<T&>(T::getInstance()); }
        ~SignletonBaseMock() { T::reset(); }
    };
}

TEST(UserOfSingleton, CheckUseOfRealSingletonImpl)
{
    Dummy::create<Dummy>(); // singletion initialization, could be done in fixture SetUp
    UserOfDummy user;

    ASSERT_TRUE(user.useDummy());

    Dummy::getInstance().reset(); // invalidate singleton, could be done in fixture TearDown
    ASSERT_THROW(user.useDummy(), std::logic_error);
}

TEST(UserOfSingleton, MockUseOfSingleton)
{
    SignletonBaseMock<DummyMock> mock;
    UserOfDummy user;

    EXPECT_CALL(mock.get(), doSmth()).WillOnce(testing::Return(false));
    ASSERT_FALSE(user.useDummy());
}

TEST(UserOfSingleton, MockUseOfSingletonInMultipleTests)
{
    SignletonBaseMock<DummyMock> mock;
    UserOfDummy user;
    int mockedVal = 42;
    int i = 1;
    double d = 2.32;

    EXPECT_CALL(mock.get(), foo(i, d)).WillOnce(testing::Return(mockedVal));
    ASSERT_EQ(user.callFoo(i, d), mockedVal);
}

}