PF_Test
======

This is a very simple unit test framework, for those of you who don't want to pull in a big, bloated library to do what should be a very simple job.

## Headline features

* Automatically run unit tests.
* Track allocations and flag memory leaks in your unit tests and fails if you leak.
* Display memory allocations per test.
* Has hooks for assert and log failures, which you can wire up easily.
* Tests can selectively ignore assert and log events, if these are expected to fire.

Note that PF_Test implements the standard symbols:

```cpp
int main(int, char**);
void* operator new(size_t);
void operator delete(void*)
void operator delete(void*, size_t)
```

When using the address sanitizer on Linux, you should build with `-DPFTEST_MEMORY_LEAK=0` or you will get multiple definition errors; this is a compiler bug. This may be used to remove the global new and deletes.

## Instructions

Refer to `tests/` for a minimal example.

1. Link to the static library PF_Test with your unit test binary.

```CMake
target_link_libraries(ExampleUnitTests PF_Test)
```

2. Include the correct header.

```cpp
#include <PF_Test/UnitTest.hpp>
```

3. Implement `void pf::test::unit_test_init(UnitTestResult)` and `void pf::test::unit_test_free()`. In these, you should set-up and tear-down your testing environment, as well as wiring up your assert and log handlers, if you have any.

```cpp
void (*assert_handler)(const char* msg);
void (*log_handler)(int sev, const char* msg);
static constexpr int SEV_ERROR = 5;

void pf::test::unit_test_init(UnitTestResult** result)
{
    static UnitTestResult** s_current_test = result;

    assert_handler = [](const char* msg)
    {
        if (!(*s_current_test)->ignore_asserts)
        {
            printf(" Failed assert %s", msg);
            fflush(stdout);
            (*s_current_test)->failed_assert = true;
            debug_break();
        }
    };

    log_handler = [](int sev, const char* msg)
    {
        if (!(*s_current_test)->ignore_log)
        {
            printf(" %d %s ", sev, msg);
            fflush(stdout);
            if (sev == SEV_ERROR)
            {
                (*s_current_test)->failed_log = true;
                debug_break();
            }
        }
    };
}

void pf::test::unit_test_free()
{
    free_testing_env();
}
```

3. Declare one or more tests using the `PFTEST_CREATE` macro.

```cpp
PFTEST_CREATE(ExampleTest)
{
    ...
}
```

4. Use the `PFTEST_EXPECT` and `PFTEST_FAIL` macros to handle conditions:

```cpp
PFTEST_CREATE(ExampleTest_Simple)
{
    int five = 2 + 3;
    PFTEST_EXPECT(five == 5);

    if (five == 2 + 2)
    {
        PFTEST_FAIL();
    }
}
```

5. You can create nested tests by using the `PFTEST_THIS` and `PFTEST_THIS_ARG` macros.

```cpp
void do_nested_test(PFTEST_THIS_ARG)
{
    PFTEST_EXPECT(true);
}

PFTEST_CREATE(ExampleTest_Nested)
{
    do_nested_test(PFTEST_THIS);
}
```

6. You may wish to ignore assert and log events by using the `PFTEST_IGNORE_ASSERTS` and `PFTEST_IGNORE_LOG` macros.

```cpp
PFTEST_CREATE(ExampleTest_Ignore)
{
    PFTEST_IGNORE_ASSERTS(true);
    PFTEST_IGNORE_LOG(true);
    assert_handler("uh oh I failed");
    log_handler(SEV_ERROR, "I totally failed hard");
    PFTEST_IGNORE_LOG(false);
    PFTEST_IGNORE_ASSERTS(false);
}
```
