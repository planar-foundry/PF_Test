#include <PF_Test/UnitTest.hpp>
#include <stdio.h>

static bool s_init = false;

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

    s_init = true;
}

void pf::test::unit_test_free()
{
    s_init = false;
}

PFTEST_CREATE(ExampleTest)
{
    PFTEST_EXPECT(s_init);
}

PFTEST_CREATE(ExampleTest_Simple)
{
    int five = 2 + 3;
    PFTEST_EXPECT(five == 5);

    if (five == 2 + 2)
    {
        PFTEST_FAIL();
    }
}

void do_nested_test(PFTEST_THIS_ARG)
{
    PFTEST_EXPECT(true);
}

PFTEST_CREATE(ExampleTest_Nested)
{
    do_nested_test(PFTEST_THIS);
}

PFTEST_CREATE(ExampleTest_Ignore)
{
    PFTEST_IGNORE_ASSERTS(true);
    PFTEST_IGNORE_LOG(true);
    assert_handler("uh oh I failed");
    log_handler(SEV_ERROR, "I totally failed hard");
    PFTEST_IGNORE_LOG(false);
    PFTEST_IGNORE_ASSERTS(false);
}