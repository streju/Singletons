# Singleton

### Is singleton an anti-pattern?
It violetes SOLID rules and wrongly used can increase complexity of the project.

In the other hand in some projects Singleton can have significant benefits in memory use compared to dependency injection.

In this repository there are two approches of implementing singleton:
- thread-safe singleton - Meyer's Singleton and singleton supporting custom construction, both implemented with use of CRTP. Hard to test/mock.
- testable singleton - special singleton that do not create pointed instance in first call, user have to do it in the begining of the program e.g. main. Could be initialized by mock/stub in tests.

The use of both versions is shown in the tests.

### Requirements
meson (build system generator)\
gtest\
gmock

To compile:
````
$ meson setup builddir && cd builddir && meson compile
````

To run tests from builddir:
````
$ ./tests/SingletonTests
````