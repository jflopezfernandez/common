
#include "common.h"

#include <check.h>

START_TEST(NonexistentFileDoesNotExist)
{
    const char* nonexistent_filename = "Made-up-file.txt";
    ck_assert_int_eq(file_exists(nonexistent_filename), FALSE);
}
END_TEST

START_TEST(ActualFileExists)
{
    const char* actual_filename = "Makefile";
    ck_assert_int_eq(file_exists(actual_filename), TRUE);
}
END_TEST

Suite* file_exists_suite(void)
{
    Suite* suite = suite_create("FileExistsSuite");
    
    /* Core test case */
    TCase* test_case_file_exists = tcase_create("FakeFileIsFake");
    tcase_add_test(test_case_file_exists, NonexistentFileDoesNotExist);
    tcase_add_test(test_case_file_exists, ActualFileExists);
    
    suite_add_tcase(suite, test_case_file_exists);
    
    return suite;
}

int main(void)
{
    Suite* file_exists_test_suite = file_exists_suite();
    SRunner* runner = srunner_create(file_exists_test_suite);

    srunner_run_all(runner, CK_NORMAL);
    int tests_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (tests_failed) ? EXIT_FAILURE : EXIT_SUCCESS;
}
