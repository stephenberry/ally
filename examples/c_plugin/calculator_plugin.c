/**
 * @file calculator_plugin.c
 * @brief Implementation of the C calculator plugin
 */

#include "ally/ally.h"
#include "ally_interface.h"
#include <math.h>

/* Forward declarations of our implementation functions */
static double calc_add(double a, double b);
static double calc_subtract(double a, double b);
static double calc_multiply(double a, double b);
static double calc_divide(double a, double b);
static void calc_store(ally_calculator_type* self, double value);
static double calc_recall(ally_calculator_type* self);
static void calc_clear(ally_calculator_type* self);

/* Implementation functions */

static double calc_add(double a, double b) {
    return a + b;
}

static double calc_subtract(double a, double b) {
    return a - b;
}

static double calc_multiply(double a, double b) {
    return a * b;
}

static double calc_divide(double a, double b) {
    if (b == 0.0) {
        return NAN;  /* Return NaN for division by zero */
    }
    return a / b;
}

static void calc_store(ally_calculator_type* self, double value) {
    self->memory = value;
}

static double calc_recall(ally_calculator_type* self) {
    return self->memory;
}

static void calc_clear(ally_calculator_type* self) {
    self->memory = 0.0;
}

/* Plugin factory function - manually defined to show initialization */
static ally_calculator_type calculator_instance = {
    .add = calc_add,
    .subtract = calc_subtract,
    .multiply = calc_multiply,
    .divide = calc_divide,
    .memory = 0.0,
    .store = calc_store,
    .recall = calc_recall,
    .clear = calc_clear
};

/* Export using standardized symbol name */
#ifdef __cplusplus
extern "C" {
#endif

ALLY_EXPORT ally_calculator_type* ALLY_CALL ally_get_interface(void) {
    return &calculator_instance;
}

#ifdef __cplusplus
}
#endif
