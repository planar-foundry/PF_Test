#include <PF_Test/UnitTest.hpp>

#if defined(WIN32)
    #include <Windows.h>
#endif

#include <atomic>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

std::vector<pf::test::UnitTest>& get_tests()
{
    static std::vector<pf::test::UnitTest> s_tests;
    return s_tests;
}

void pf::test::register_unit_test(const char* name, UnitTestFunc function)
{
    get_tests().push_back({ name, function });
}

void pf::test::debug_break()
{
#if defined(WIN32)
    if (IsDebuggerPresent())
    {
        __debugbreak();
    }
#endif
}

static std::atomic<int64_t> s_bytes_allocated = 0;
static std::atomic<int64_t> s_total_bytes_allocated = 0;

void* pf::test::custom_alloc(size_t size)
{
    void* data = ::malloc(size + 8);
    memcpy(data, &size, 8);
    s_bytes_allocated += size;
    s_total_bytes_allocated += size;
    return (unsigned char*)data + 8;
}

void pf::test::custom_free(void* data)
{
    if (data)
    {
        size_t size;
        void* data_start = (unsigned char*)data - 8;
        memcpy(&size, data_start, 8);
        ::free(data_start);
        s_bytes_allocated -= size;
    }
}

#if defined(PFTEST_MEMORY_LEAK_DETECTION)

void* operator new(size_t size)
{
    return pf::test::custom_alloc(size);
}

void operator delete(void* data)
{
    pf::test::custom_free(data);
}

void operator delete(void* data, size_t)
{
    pf::test::custom_free(data);
}

#endif

int main(int argc, char** argv)
{
    using namespace pf::test;

    char* whitelist = argc == 2 ? argv[1] : nullptr;

    int64_t overall_bytes_before = s_bytes_allocated;

    static UnitTestResult* s_current_test;
    unit_test_init(&s_current_test);

    bool any_failures = false;

    for (const UnitTest& test : get_tests())
    {
        if (whitelist && !strstr(test.name, whitelist))
        {
            printf("Skipping test %s\n", test.name);
            continue;
        }

        UnitTestResult result;
        s_current_test = &result;

        printf("Running test %s ...", test.name);
        fflush(stdout);

        int64_t total_bytes_before = s_total_bytes_allocated;
        int64_t bytes_before = s_bytes_allocated;

        test.function(&result);

        if (result.failed_condition)
        {
            printf(" FAILED!\n    %s:%d\n    %s\n", result.failed_file, result.failed_line, result.failed_condition);
            any_failures = true;
        }
        else if (s_current_test->failed_assert || s_current_test->failed_log)
        {
            printf(" FAILED!\n    Assert or log event in unit test.\n");
            any_failures = true;
        }
        else if (bytes_before != s_bytes_allocated)
        {
            printf(" FAILED!\n    %zu bytes before test, %zu bytes after test. Memory leak?\n", bytes_before, s_bytes_allocated.load());
            any_failures = true;
        }
        else
        {
            printf(" SUCCESS! (alloc: %zu)\n", s_total_bytes_allocated - total_bytes_before);
        }

        fflush(stdout);
        s_current_test = nullptr;
    }

    unit_test_free();

    if (s_bytes_allocated != overall_bytes_before)
    {
        printf("FAILED!\n    %zu bytes were still allocated at teardown; expected %zu.\n", s_bytes_allocated.load(), overall_bytes_before);
        fflush(stdout);
        any_failures = true;
    }

    if (any_failures)
    {
        debug_break();
        return 1;
    }
}
