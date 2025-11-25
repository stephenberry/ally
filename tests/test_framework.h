/**
 * @file test_framework.h
 * @brief Lightweight C test macros shared across Ally unit tests
 */

#ifndef ALLY_TEST_FRAMEWORK_H
#define ALLY_TEST_FRAMEWORK_H

#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef __cplusplus
#include <stdbool.h>
#else
#include <string>

struct ally_test_string_value {
    std::string value;
    bool is_null;
};

static inline ally_test_string_value ally_test_make_string_value(const std::string& input) {
    return ally_test_string_value{input, false};
}

static inline ally_test_string_value ally_test_make_string_value(const char* input) {
    return ally_test_string_value{input ? std::string(input) : std::string(), input == nullptr};
}
#endif

/* Simple shared test state */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    static int test_##name(void); \
    static void run_##name(void) { \
        tests_run++; \
        printf("  Running: %s ... ", #name); \
        fflush(stdout); \
        if (test_##name() == 0) { \
            printf("PASS\n"); \
            tests_passed++; \
        } else { \
            printf("FAIL\n"); \
            tests_failed++; \
        } \
    } \
    static int test_##name(void)

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "\n    Assertion failed: %s\n", message); \
            return 1; \
        } \
    } while(0)

#define ASSERT_EQ(a, b, message) \
    do { \
        if ((a) != (b)) { \
            fprintf(stderr, "\n    Assertion failed: %s (expected %d, got %d)\n", \
                    message, (int)(b), (int)(a)); \
            return 1; \
        } \
    } while(0)

#define ASSERT_DOUBLE_EQ(a, b, message) \
    do { \
        if (fabs((a) - (b)) > 0.0001) { \
            fprintf(stderr, "\n    Assertion failed: %s (expected %.4f, got %.4f)\n", \
                    message, (double)(b), (double)(a)); \
            return 1; \
        } \
    } while(0)

#ifdef __cplusplus
#define ASSERT_STR_EQ(a, b, message) \
    do { \
        ally_test_string_value _ally_expected = ally_test_make_string_value(b); \
        ally_test_string_value _ally_actual = ally_test_make_string_value(a); \
        if ((_ally_expected.is_null != _ally_actual.is_null) || \
            (!_ally_expected.is_null && _ally_actual.value != _ally_expected.value)) { \
            fprintf(stderr, "\n    Assertion failed: %s (expected '%s', got '%s')\n", \
                    message, \
                    _ally_expected.is_null ? "(null)" : _ally_expected.value.c_str(), \
                    _ally_actual.is_null ? "(null)" : _ally_actual.value.c_str()); \
            return 1; \
        } \
    } while(0)
#else
#define ASSERT_STR_EQ(a, b, message) \
    do { \
        const char* _ally_expected = (b); \
        const char* _ally_actual = (a); \
        if ((_ally_expected == NULL && _ally_actual != NULL) || \
            (_ally_expected != NULL && _ally_actual == NULL) || \
            (_ally_expected && _ally_actual && strcmp(_ally_actual, _ally_expected) != 0)) { \
            fprintf(stderr, "\n    Assertion failed: %s (expected '%s', got '%s')\n", \
                    message, \
                    _ally_expected ? _ally_expected : "(null)", \
                    _ally_actual ? _ally_actual : "(null)"); \
            return 1; \
        } \
    } while(0)
#endif

#define ASSERT_NULL(ptr, message) \
    do { \
        if ((ptr) != NULL) { \
            fprintf(stderr, "\n    Assertion failed: %s (expected NULL, got %p)\n", \
                    message, (void*)(ptr)); \
            return 1; \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr, message) \
    do { \
        if ((ptr) == NULL) { \
            fprintf(stderr, "\n    Assertion failed: %s (got NULL)\n", message); \
            return 1; \
        } \
    } while(0)

#define ASSERT_TRUE(condition, message) ASSERT((condition), (message))
#define ASSERT_FALSE(condition, message) ASSERT(!(condition), (message))

#define RUN_TEST(name) run_##name()

#define PRINT_TEST_SUMMARY() \
    do { \
        printf("\n=== Test Summary ===\n"); \
        printf("Total:  %d\n", tests_run); \
        printf("Passed: %d\n", tests_passed); \
        printf("Failed: %d\n", tests_failed); \
    } while(0)

#endif /* ALLY_TEST_FRAMEWORK_H */
