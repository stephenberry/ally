/**
 * @file test_cpp_api.cpp
 * @brief Unit tests for Ally C++ API
 *
 * Simple test framework without external dependencies.
 * Returns 0 on success, non-zero on failure.
 */

#include <iostream>
#include <string>
#include <cmath>
#include <stdexcept>

#include "../examples/c_plugin/ally_interface.h"
#include "../examples/cpp_plugin/ally_interface.hpp"
#include "ally/ally.hpp"
#include "test_framework.h"

/* Test: Load plugin with plugin_loader */
TEST(load_plugin_with_loader) {
    ally::plugin_loader<ally_string_processor_type> loader;
    bool success = loader.load("./string_processor_plugin");
    ASSERT_TRUE(success, "Plugin should load successfully");
    ASSERT_TRUE(loader.is_loaded(), "Loader should report plugin as loaded");
    return 0;
}

/* Test: Load nonexistent plugin fails */
TEST(load_nonexistent_plugin) {
    ally::plugin_loader<ally_string_processor_type> loader;
    bool success = loader.load("./nonexistent_plugin");
    ASSERT_FALSE(success, "Loading nonexistent plugin should fail");
    ASSERT_FALSE(loader.is_loaded(), "Loader should report not loaded");

    std::string error = loader.last_error();
    ASSERT_TRUE(!error.empty(), "Error message should not be empty");
    return 0;
}

/* Test: Get plugin interface */
TEST(get_plugin_interface) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");

    auto* plugin = loader.get();
    ASSERT_TRUE(plugin != nullptr, "Plugin interface should be available");
    return 0;
}

/* Test: Arrow operator access */
TEST(arrow_operator_access) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");

    std::string result = loader->to_upper("hello");
    ASSERT_STR_EQ(result, "HELLO", "to_upper should convert to uppercase");
    return 0;
}

/* Test: String to_upper function */
TEST(string_to_upper) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");

    auto* plugin = loader.get();
    std::string result = plugin->to_upper("hello world");
    ASSERT_STR_EQ(result, "HELLO WORLD", "Should convert to uppercase");
    return 0;
}

/* Test: String to_lower function */
TEST(string_to_lower) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");

    auto* plugin = loader.get();
    std::string result = plugin->to_lower("HELLO WORLD");
    ASSERT_STR_EQ(result, "hello world", "Should convert to lowercase");
    return 0;
}

/* Test: String reverse function */
TEST(string_reverse) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");

    auto* plugin = loader.get();
    std::string result = plugin->reverse("hello");
    ASSERT_STR_EQ(result, "olleh", "Should reverse the string");
    return 0;
}

/* Test: String replace_all function */
TEST(string_replace_all) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");

    auto* plugin = loader.get();
    std::string replaced = plugin->replace_all("foo-bar-bar", "bar", "baz");
    ASSERT_STR_EQ(replaced, "foo-baz-baz", "Should replace all matches");

    std::string removed = plugin->replace_all("abc123abc", "abc", "");
    ASSERT_STR_EQ(removed, "123", "Should support removal when replacement is empty");
    return 0;
}

/* Test: Word count function */
TEST(word_count) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");

    auto* plugin = loader.get();
    size_t count = plugin->word_count("the quick brown fox");
    ASSERT_EQ(count, 4, "Should count 4 words");
    return 0;
}

/* Test: Split function */
TEST(string_split) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");

    auto* plugin = loader.get();
    auto tokens = plugin->split("apple,banana,cherry", ',');

    ASSERT_EQ(tokens.size(), 3, "Should split into 3 tokens");
    ASSERT_STR_EQ(tokens[0], "apple", "First token should be 'apple'");
    ASSERT_STR_EQ(tokens[1], "banana", "Second token should be 'banana'");
    ASSERT_STR_EQ(tokens[2], "cherry", "Third token should be 'cherry'");
    return 0;
}


/* Test: Load C calculator plugin via C++ loader */
TEST(load_c_calculator_with_cpp_loader) {
    ally::plugin_loader<ally_calculator_type> loader;
    ASSERT_TRUE(loader.load("./calculator_plugin"),
                "Calculator plugin should load via C++ plugin_loader");
    ASSERT_TRUE(loader.is_loaded(), "Loader should report calculator plugin as loaded");

    ally_calculator_type* calc = loader.get();
    ASSERT_TRUE(calc != nullptr, "Calculator interface should be available");

    double sum = calc->add(4.0, 6.0);
    ASSERT_EQ(sum, 10.0, "Calculator add should work via C++ loader");

    double diff = calc->subtract(9.0, 2.0);
    ASSERT_EQ(diff, 7.0, "Calculator subtract should work");

    double product = calc->multiply(3.0, 5.0);
    ASSERT_EQ(product, 15.0, "Calculator multiply should work");

    double quotient = calc->divide(12.0, 4.0);
    ASSERT_EQ(quotient, 3.0, "Calculator divide should work");

    calc->store(calc, 42.0);
    ASSERT_EQ(calc->recall(calc), 42.0, "Calculator store/recall should work");
    calc->clear(calc);
    ASSERT_EQ(calc->recall(calc), 0.0, "Calculator clear should reset memory");

    return 0;
}

/* Test: load_plugin helper works with C calculator plugin */
TEST(load_c_calculator_with_helper) {
    auto loader = ally::load_plugin<ally_calculator_type>("./calculator_plugin");
    ASSERT_TRUE(loader.is_loaded(), "C calculator plugin should load via helper");

    double result = loader->divide(20.0, 5.0);
    ASSERT_EQ(result, 4.0, "Helper-loaded calculator should divide correctly");

    loader->store(loader.get(), 99.0);
    ASSERT_EQ(loader->recall(loader.get()), 99.0, "Helper loader should expose state functions");

    std::string path = loader.plugin_path();
    ASSERT_TRUE(!path.empty(), "Plugin path should not be empty for calculator plugin");

    return 0;
}

/* Test: History management */
TEST(history_management) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");

    auto* plugin = loader.get();

    /* Add entries */
    plugin->add_to_history("first");
    plugin->add_to_history("second");
    plugin->add_to_history("third");

    /* Get history */
    auto history = plugin->get_history();
    ASSERT_EQ(history.size(), 3, "Should have 3 history entries");
    ASSERT_STR_EQ(history[0], "first", "First entry should be 'first'");
    ASSERT_STR_EQ(history[2], "third", "Third entry should be 'third'");

    /* Clear history */
    plugin->clear_history();
    history = plugin->get_history();
    ASSERT_EQ(history.size(), 0, "History should be empty after clear");

    return 0;
}

/* Test: Plugin version constant */
TEST(plugin_version) {
    std::string version(ally_string_processor_version);
    ASSERT_STR_EQ(version, "1.0.0", "Version should be 1.0.0");
    return 0;
}

/* Test: Plugin path */
TEST(plugin_path) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");

    std::string path = loader.plugin_path();
    ASSERT_TRUE(!path.empty(), "Plugin path should not be empty");
    ASSERT_TRUE(path.find("string_processor_plugin") != std::string::npos,
                "Path should contain plugin name");
    return 0;
}

/* Test: Move constructor */
TEST(move_constructor) {
    ally::plugin_loader<ally_string_processor_type> loader1;
    ASSERT_TRUE(loader1.load("./string_processor_plugin"), "Plugin should load");
    ASSERT_TRUE(loader1.is_loaded(), "loader1 should be loaded");

    /* Move construct */
    ally::plugin_loader<ally_string_processor_type> loader2(std::move(loader1));
    ASSERT_TRUE(loader2.is_loaded(), "loader2 should be loaded after move");
    ASSERT_FALSE(loader1.is_loaded(), "loader1 should not be loaded after move");

    /* Verify moved plugin works */
    std::string result = loader2->to_upper("test");
    ASSERT_STR_EQ(result, "TEST", "Moved plugin should still work");

    return 0;
}

/* Test: Move assignment */
TEST(move_assignment) {
    ally::plugin_loader<ally_string_processor_type> loader1;
    ally::plugin_loader<ally_string_processor_type> loader2;

    ASSERT_TRUE(loader1.load("./string_processor_plugin"), "Plugin should load");
    ASSERT_TRUE(loader1.is_loaded(), "loader1 should be loaded");

    /* Move assign */
    loader2 = std::move(loader1);
    ASSERT_TRUE(loader2.is_loaded(), "loader2 should be loaded after move");
    ASSERT_FALSE(loader1.is_loaded(), "loader1 should not be loaded after move");

    return 0;
}

/* Test: Bool conversion operator */
TEST(bool_conversion) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_FALSE((bool)loader, "Empty loader should convert to false");

    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");
    ASSERT_TRUE((bool)loader, "Loaded plugin should convert to true");

    return 0;
}

/* Test: Dereference operator */
TEST(dereference_operator) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");

    auto& plugin = *loader;
    std::string result = plugin.to_upper("test");
    ASSERT_STR_EQ(result, "TEST", "Dereferenced plugin should work");

    return 0;
}

/* Test: Manual unload */
TEST(manual_unload) {
    ally::plugin_loader<ally_string_processor_type> loader;
    ASSERT_TRUE(loader.load("./string_processor_plugin"), "Plugin should load");
    ASSERT_TRUE(loader.is_loaded(), "Plugin should be loaded");

    loader.unload();
    ASSERT_FALSE(loader.is_loaded(), "Plugin should not be loaded after unload");

    return 0;
}

/* Test: load_plugin helper throws on failure */
TEST(load_plugin_helper_throws) {
    try {
        auto loader = ally::load_plugin<ally_string_processor_type>("./nonexistent");
        ASSERT_FALSE(true, "Should have thrown exception");
    } catch (const std::runtime_error& e) {
        /* Expected */
        std::string msg(e.what());
        ASSERT_TRUE(!msg.empty(), "Exception message should not be empty");
    }
    return 0;
}

/* Test: load_plugin helper success */
TEST(load_plugin_helper_success) {
    auto loader = ally::load_plugin<ally_string_processor_type>("./string_processor_plugin");
    ASSERT_TRUE(loader.is_loaded(), "Plugin should be loaded");

    std::string result = loader->to_upper("test");
    ASSERT_STR_EQ(result, "TEST", "Plugin should work");

    return 0;
}

/* Main test runner */
int main() {
    std::cout << "=== Ally C++ API Unit Tests ===\n\n";

    /* Run all tests */
    RUN_TEST(load_plugin_with_loader);
    RUN_TEST(load_nonexistent_plugin);
    RUN_TEST(get_plugin_interface);
    RUN_TEST(arrow_operator_access);
    RUN_TEST(string_to_upper);
    RUN_TEST(string_to_lower);
    RUN_TEST(string_reverse);
    RUN_TEST(string_replace_all);
    RUN_TEST(word_count);
    RUN_TEST(string_split);
    RUN_TEST(load_c_calculator_with_cpp_loader);
    RUN_TEST(load_c_calculator_with_helper);
    RUN_TEST(history_management);
    RUN_TEST(plugin_version);
    RUN_TEST(plugin_path);
    RUN_TEST(move_constructor);
    RUN_TEST(move_assignment);
    RUN_TEST(bool_conversion);
    RUN_TEST(dereference_operator);
    RUN_TEST(manual_unload);
    RUN_TEST(load_plugin_helper_throws);
    RUN_TEST(load_plugin_helper_success);

    PRINT_TEST_SUMMARY();

    if (tests_failed == 0) {
        std::cout << "\nAll tests passed!\n";
        return 0;
    } else {
        std::cout << "\n" << tests_failed << " test(s) failed!\n";
        return 1;
    }
}
