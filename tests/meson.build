gtest_dep = dependency('gtest', main : true, required : true)
gmock_dep = dependency('gmock', main : true, required : true)

if gtest_dep.found() and gmock_dep.found()
    tests_src = []
    tests_src += files('ThreadSafeSingletonTests.cpp')
    subdir('TestableSingleton')
    e = executable('SingletonTests', tests_src, dependencies : [gtest_dep, gmock_dep], include_directories : inc)
    test('gtest test', e)
endif