/**
 * @file test_c_api.c
 * @brief Unit tests for Ally C API
 *
 * Simple test framework without external dependencies.
 * Returns 0 on success, non-zero on failure.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "ally/ally.h"

#include "../examples/c_plugin/ally_interface.h"
#include "test_framework.h"

/* Test: Load plugin successfully */
TEST(load_plugin_success) {
    ally_handle_t handle = ally_load("./calculator_plugin");
    ASSERT_NOT_NULL(handle, "Plugin should load successfully");

    ASSERT(ally_is_loaded(handle), "Plugin should be marked as loaded");

    const char* path = ally_plugin_path(handle);
    ASSERT_NOT_NULL(path, "Plugin path should be available");

    ally_unload(handle);
    return 0;
}

/* Test: Load nonexistent plugin fails */
TEST(load_nonexistent_plugin) {
    ally_handle_t handle = ally_load("./nonexistent_plugin");
    if (handle != NULL) {
        fprintf(stderr, "\n    Assertion failed: Loading nonexistent plugin should fail\n");
        ally_unload(handle);
        return 1;
    }

    /* Verify error message is set */
    const char* error = ally_last_error();
    ASSERT_NOT_NULL(error, "Error message should be set");
    ASSERT(strlen(error) > 0, "Error message should not be empty");

    return 0;
}

/* Test: Get plugin interface */
TEST(get_plugin_interface) {
    ally_handle_t handle = ally_load("./calculator_plugin");
    ASSERT_NOT_NULL(handle, "Plugin should load");

    ally_calculator_type* calc = ally_get_calculator(handle);
    ASSERT_NOT_NULL(calc, "Plugin interface should be available");

    ally_unload(handle);
    return 0;
}

/* Test: Calculator add function */
TEST(calculator_add) {
    ally_handle_t handle = ally_load("./calculator_plugin");
    ASSERT_NOT_NULL(handle, "Plugin should load");

    ally_calculator_type* calc = ally_get_calculator(handle);
    ASSERT_NOT_NULL(calc, "Plugin interface should be available");

    double result = calc->add(2.5, 3.5);
    ASSERT_DOUBLE_EQ(result, 6.0, "2.5 + 3.5 should equal 6.0");

    ally_unload(handle);
    return 0;
}

/* Test: Calculator subtract function */
TEST(calculator_subtract) {
    ally_handle_t handle = ally_load("./calculator_plugin");
    ASSERT_NOT_NULL(handle, "Plugin should load");

    ally_calculator_type* calc = ally_get_calculator(handle);
    double result = calc->subtract(10.0, 3.0);
    ASSERT_DOUBLE_EQ(result, 7.0, "10.0 - 3.0 should equal 7.0");

    ally_unload(handle);
    return 0;
}

/* Test: Calculator multiply function */
TEST(calculator_multiply) {
    ally_handle_t handle = ally_load("./calculator_plugin");
    ASSERT_NOT_NULL(handle, "Plugin should load");

    ally_calculator_type* calc = ally_get_calculator(handle);
    double result = calc->multiply(4.0, 5.0);
    ASSERT_DOUBLE_EQ(result, 20.0, "4.0 * 5.0 should equal 20.0");

    ally_unload(handle);
    return 0;
}

/* Test: Calculator divide function */
TEST(calculator_divide) {
    ally_handle_t handle = ally_load("./calculator_plugin");
    ASSERT_NOT_NULL(handle, "Plugin should load");

    ally_calculator_type* calc = ally_get_calculator(handle);
    double result = calc->divide(20.0, 4.0);
    ASSERT_DOUBLE_EQ(result, 5.0, "20.0 / 4.0 should equal 5.0");

    ally_unload(handle);
    return 0;
}

/* Test: Calculator divide by zero */
TEST(calculator_divide_by_zero) {
    ally_handle_t handle = ally_load("./calculator_plugin");
    ASSERT_NOT_NULL(handle, "Plugin should load");

    ally_calculator_type* calc = ally_get_calculator(handle);
    double result = calc->divide(10.0, 0.0);
    ASSERT(isnan(result), "Division by zero should return NaN");

    ally_unload(handle);
    return 0;
}

/* Test: Calculator memory operations */
TEST(calculator_memory) {
    ally_handle_t handle = ally_load("./calculator_plugin");
    ASSERT_NOT_NULL(handle, "Plugin should load");

    ally_calculator_type* calc = ally_get_calculator(handle);

    /* Store value */
    calc->store(calc, 42.0);

    /* Recall value */
    double recalled = calc->recall(calc);
    ASSERT_DOUBLE_EQ(recalled, 42.0, "Recalled value should be 42.0");

    /* Clear memory */
    calc->clear(calc);
    recalled = calc->recall(calc);
    ASSERT_DOUBLE_EQ(recalled, 0.0, "Cleared memory should be 0.0");

    ally_unload(handle);
    return 0;
}

/* Test: Plugin version constant */
TEST(plugin_version) {
    ASSERT_NOT_NULL(ally_calculator_version, "Version constant should exist");
    ASSERT_STR_EQ(ally_calculator_version, "1.0.0", "Version should be 1.0.0");
    return 0;
}

/* Test: Unload NULL handle is safe */
TEST(unload_null_safe) {
    ally_unload(NULL);  /* Should not crash */
    return 0;
}

/* Test: Get from invalid handle returns NULL */
TEST(get_from_null_handle) {
    void* result = ally_get(NULL);
    ASSERT_NULL(result, "Getting from NULL handle should return NULL");
    return 0;
}

/* Test: is_loaded with NULL handle */
TEST(is_loaded_null_handle) {
    bool loaded = ally_is_loaded(NULL);
    ASSERT(!loaded, "NULL handle should not be loaded");
    return 0;
}

/* Test: plugin_path with NULL handle */
TEST(plugin_path_null_handle) {
    const char* path = ally_plugin_path(NULL);
    ASSERT_NULL(path, "Plugin path of NULL handle should be NULL");
    return 0;
}

/* Main test runner */
int main(void) {
    printf("=== Ally C API Unit Tests ===\n\n");

    /* Run all tests */
    RUN_TEST(load_plugin_success);
    RUN_TEST(load_nonexistent_plugin);
    RUN_TEST(get_plugin_interface);
    RUN_TEST(calculator_add);
    RUN_TEST(calculator_subtract);
    RUN_TEST(calculator_multiply);
    RUN_TEST(calculator_divide);
    RUN_TEST(calculator_divide_by_zero);
    RUN_TEST(calculator_memory);
    RUN_TEST(plugin_version);
    RUN_TEST(unload_null_safe);
    RUN_TEST(get_from_null_handle);
    RUN_TEST(is_loaded_null_handle);
    RUN_TEST(plugin_path_null_handle);

    PRINT_TEST_SUMMARY();

    if (tests_failed == 0) {
        printf("\nAll tests passed!\n");
        return 0;
    } else {
        printf("\n%d test(s) failed!\n", tests_failed);
        return 1;
    }
}
