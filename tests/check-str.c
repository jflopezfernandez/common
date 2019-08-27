
#include <check.h>

#include "common.h"

START_TEST(EmptyStringsMatch)
{
    const char* a = "";
    const char* b = "";

    ck_assert_int_eq(strings_match(a, b), TRUE);
}
END_TEST

__attribute__((returns_nonnull))
Suite* string_suite(void)
{
    Suite* suite = suite_create("String Suite");

    /* Create core test case */
    TCase* core_test_case = tcase_create("Core Test Case");
    tcase_add_test(core_test_case, EmptyStringsMatch);
    suite_add_tcase(suite, core_test_case);

    return suite;
}

int main(void)
{
    Suite* string_test_suite = string_suite();
    SRunner* runner = srunner_create(string_test_suite);

    srunner_run_all(runner, CK_NORMAL);
    int failed_tests = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (failed_tests) ? EXIT_FAILURE : EXIT_SUCCESS;
}
