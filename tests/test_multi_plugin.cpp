/**
 * @file test_multi_plugin.cpp
 * @brief Unit tests for loading multiple plugin types simultaneously
 *
 * Tests the namespace-based deconfliction system by loading both
 * calculator and string_processor plugins in the same application.
 *
 * Returns 0 on success, non-zero on failure.
 */

#include <iostream>
#include <string>
#include <cmath>

#include "../examples/c_plugin/ally_interface.h"
#include "../examples/cpp_plugin/ally_interface.hpp"
#include "ally/ally.h"
#include "ally/ally.hpp"
#include "test_framework.h"

/* Test: Load both plugins simultaneously using C API */
TEST(load_both_plugins_c_api) {
    /* Load calculator plugin */
    ally_handle_t calc_handle = ally_load("./calculator_plugin");
    ASSERT_TRUE(calc_handle != NULL, "Calculator plugin should load");

    /* Load string processor plugin */
    ally_handle_t str_handle = ally_load("./string_processor_plugin");
    ASSERT_TRUE(str_handle != NULL, "String processor plugin should load");

    /* Get both interfaces */
    ally_calculator_type* calc = ally_get_calculator(calc_handle);
    ASSERT_TRUE(calc != NULL, "Calculator interface should be available");

    ally_string_processor_type* str_proc = ally_get_string_processor(str_handle);
    ASSERT_TRUE(str_proc != NULL, "String processor interface should be available");

    /* Use both plugins */
    double calc_result = calc->add(2.0, 3.0);
    ASSERT_DOUBLE_EQ(calc_result, 5.0, "Calculator add should work");

    std::string str_result = str_proc->to_upper("hello");
    ASSERT_STR_EQ(str_result, "HELLO", "String processor to_upper should work");

    /* Clean up */
    ally_unload(calc_handle);
    ally_unload(str_handle);

    return 0;
}

/* Test: Load both plugins simultaneously using C++ API */
TEST(load_both_plugins_cpp_api) {
    /* Load calculator plugin */
    ally::plugin_loader<ally_calculator_type> calc_loader;
    ASSERT_TRUE(calc_loader.load("./calculator_plugin"), "Calculator plugin should load");

    /* Load string processor plugin */
    ally::plugin_loader<ally_string_processor_type> str_loader;
    ASSERT_TRUE(str_loader.load("./string_processor_plugin"), "String processor plugin should load");

    /* Use both plugins */
    double calc_result = calc_loader->add(10.0, 5.0);
    ASSERT_DOUBLE_EQ(calc_result, 15.0, "Calculator add should work");

    std::string str_result = str_loader->reverse("test");
    ASSERT_STR_EQ(str_result, "tset", "String processor reverse should work");

    return 0;
}

/* Test: Load both plugins, mixed C and C++ API */
TEST(load_both_plugins_mixed_api) {
    /* Load calculator with C API */
    ally_handle_t calc_handle = ally_load("./calculator_plugin");
    ASSERT_TRUE(calc_handle != NULL, "Calculator plugin should load");
    ally_calculator_type* calc = ally_get_calculator(calc_handle);

    /* Load string processor with C++ API */
    ally::plugin_loader<ally_string_processor_type> str_loader;
    ASSERT_TRUE(str_loader.load("./string_processor_plugin"), "String processor plugin should load");

    /* Use both */
    double multiply_result = calc->multiply(4.0, 3.0);
    ASSERT_DOUBLE_EQ(multiply_result, 12.0, "Calculator multiply should work");

    size_t word_count = str_loader->word_count("hello world");
    ASSERT_EQ(word_count, 2, "String processor word_count should work");

    /* Clean up */
    ally_unload(calc_handle);

    return 0;
}

/* Test: Verify no namespace collisions in type names */
TEST(verify_namespace_separation) {
    /* Both type definitions should coexist */
    ally_calculator_type* calc_ptr = nullptr;
    ally_string_processor_type* str_ptr = nullptr;

    /* Suppress unused warnings */
    (void)calc_ptr;
    (void)str_ptr;

    /* If we got here, types don't collide */
    ASSERT_TRUE(true, "Types should be distinct");

    return 0;
}

/* Test: Verify version constants don't collide */
TEST(verify_version_constants) {
    /* Both version constants should exist */
    std::string calc_version(ally_calculator_version);
    std::string str_version(ally_string_processor_version);

    ASSERT_STR_EQ(calc_version, "1.0.0", "Calculator version should be accessible");
    ASSERT_STR_EQ(str_version, "1.0.0", "String processor version should be accessible");

    return 0;
}

/* Test: Complex interaction between both plugins */
TEST(complex_interaction) {
    /* Load both plugins */
    ally::plugin_loader<ally_calculator_type> calc;
    ally::plugin_loader<ally_string_processor_type> str_proc;

    ASSERT_TRUE(calc.load("./calculator_plugin"), "Calculator should load");
    ASSERT_TRUE(str_proc.load("./string_processor_plugin"), "String processor should load");

    /* Use calculator */
    double a = 10.0, b = 3.0;
    double sum = calc->add(a, b);
    double product = calc->multiply(a, b);

    /* Use string processor */
    std::string input = "Hello World";
    std::string upper = str_proc->to_upper(input);
    std::string lower = str_proc->to_lower(upper);

    /* Verify results */
    ASSERT_DOUBLE_EQ(sum, 13.0, "Sum should be correct");
    ASSERT_DOUBLE_EQ(product, 30.0, "Product should be correct");
    ASSERT_STR_EQ(upper, "HELLO WORLD", "Uppercase should be correct");
    ASSERT_STR_EQ(lower, "hello world", "Lowercase should be correct");

    /* Store calculator result */
    calc->store(calc.get(), sum);
    double recalled = calc->recall(calc.get());
    ASSERT_DOUBLE_EQ(recalled, 13.0, "Recalled value should match stored value");

    /* Add to string processor history */
    str_proc->add_to_history(upper);
    str_proc->add_to_history(lower);
    auto history = str_proc->get_history();
    ASSERT_EQ(history.size(), 2, "History should have 2 entries");

    return 0;
}

/* Test: Reload same plugin type multiple times */
TEST(reload_same_plugin_type) {
    ally::plugin_loader<ally_calculator_type> loader;

    /* Load first time */
    ASSERT_TRUE(loader.load("./calculator_plugin"), "First load should succeed");
    double result1 = loader->add(1.0, 2.0);
    ASSERT_DOUBLE_EQ(result1, 3.0, "First plugin should work");

    /* Unload */
    loader.unload();
    ASSERT_TRUE(!loader.is_loaded(), "Plugin should be unloaded");

    /* Reload */
    ASSERT_TRUE(loader.load("./calculator_plugin"), "Reload should succeed");
    double result2 = loader->add(5.0, 7.0);
    ASSERT_DOUBLE_EQ(result2, 12.0, "Reloaded plugin should work");

    return 0;
}

/* Main test runner */
int main() {
    std::cout << "=== Ally Multi-Plugin Loading Tests ===\n\n";

    /* Run all tests */
    RUN_TEST(load_both_plugins_c_api);
    RUN_TEST(load_both_plugins_cpp_api);
    RUN_TEST(load_both_plugins_mixed_api);
    RUN_TEST(verify_namespace_separation);
    RUN_TEST(verify_version_constants);
    RUN_TEST(complex_interaction);
    RUN_TEST(reload_same_plugin_type);

    PRINT_TEST_SUMMARY();

    if (tests_failed == 0) {
        std::cout << "\nAll tests passed!\n";
        return 0;
    } else {
        std::cout << "\n" << tests_failed << " test(s) failed!\n";
        return 1;
    }
}
