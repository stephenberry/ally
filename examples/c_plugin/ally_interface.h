/**
 * @file ally_interface.h
 * @brief Plugin interface definition for C calculator plugin
 *
 * This file defines the interface for the calculator plugin using the namespace
 * "calculator" to avoid collisions with other plugin types.
 *
 * Naming convention:
 *   - ally_calculator_type: The plugin interface struct type
 *   - ally_calculator_version: Plugin version (static const)
 *
 * Use ally_get_calculator() to retrieve the typed interface from ally_get().
 *
 * Note: Plugin name is derived from the shared library filename.
 */

#ifndef ALLY_INTERFACE_H
#define ALLY_INTERFACE_H

#include "ally/ally.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Plugin version - required
 */
static const char* const ally_calculator_version = "1.0.0";

/**
 * @brief Calculator plugin interface
 *
 * This struct defines the API that the calculator plugin exposes.
 */
typedef struct calculator_plugin {
    /* Function pointers for calculator operations */
    double (*add)(double a, double b);
    double (*subtract)(double a, double b);
    double (*multiply)(double a, double b);
    double (*divide)(double a, double b);

    /* State management */
    double memory;
    void (*store)(struct calculator_plugin* self, double value);
    double (*recall)(struct calculator_plugin* self);
    void (*clear)(struct calculator_plugin* self);
} calculator_plugin;

/**
 * @brief Define namespaced type for this plugin
 *
 * This allows multiple different plugin types to coexist in the same application.
 *
 * @example
 * ally_handle_t h = ally_load("calculator");
 * ally_calculator_type* calc = ally_get_calculator(h);
 * double result = calc->add(2, 3);
 */
typedef calculator_plugin ally_calculator_type;

/**
 * @brief Type-safe helpers for retrieving the calculator interface
 */
static inline ally_calculator_type* ally_get_calculator(ally_handle_t handle) {
    return (ally_calculator_type*)ally_get(handle);
}

#ifdef __cplusplus
}
#endif

#endif /* ALLY_INTERFACE_H */
