#pragma once

#include <stddef.h>

namespace pf::test
{

struct UnitTestResult
{
    const char* failed_condition = nullptr;
    const char* failed_file;
    int failed_line;
    bool ignore_asserts = false;
    bool ignore_log = false;
    bool failed_assert = false;
    bool failed_log = false;
};

using UnitTestFunc = void(*)(UnitTestResult* _pf_result);

struct UnitTest
{
    const char* name;
    UnitTestFunc function;
};

void register_unit_test(const char* name, UnitTestFunc function);

struct ScopedUnitTest
{
    ScopedUnitTest(const char* name, UnitTestFunc function)
    {
        register_unit_test(name, function);
    }
};

void debug_break();

// When custom allocators are required, these should be used.
void* custom_alloc(size_t size);
void custom_free(void* data);

#define PFTEST_THIS _pf_result
#define PFTEST_THIS_ARG ::pf::test::UnitTestResult* PFTEST_THIS

#define PFTEST_CREATE(name) \
    void _##name(PFTEST_THIS_ARG); \
    static ::pf::test::ScopedUnitTest s__##name(#name, &_##name); \
    void _##name(PFTEST_THIS_ARG)

#define PFTEST_EXPECT(cond) \
    do \
    { \
        if (!(cond) && !_pf_result->failed_condition) \
        { \
            ::pf::test::debug_break(); \
            _pf_result->failed_condition = (#cond); \
            _pf_result->failed_file = __FILE__; \
            _pf_result->failed_line = __LINE__; \
        } \
    } while (0)

#define PFTEST_FAIL() PFTEST_EXPECT(false)

#define PFTEST_IGNORE_ASSERTS(status) _pf_result->ignore_asserts = status
#define PFTEST_IGNORE_LOG(status) _pf_result->ignore_log = status

// The user must implement the following functions:

extern void unit_test_init(UnitTestResult** result);
extern void unit_test_free();

}
