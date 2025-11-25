/**
 * @file c_host.c
 * @brief Example C host application that loads the calculator plugin
 */

#include <stdio.h>
#include <stdlib.h>

/* We need to include the same ally_interface.h that the plugin uses */
#include "../c_plugin/ally_interface.h"
#include "ally/ally.h"

int main(int argc, char* argv[]) {
    const char* plugin_path = "./calculator_plugin";
    if (argc > 1) {
        plugin_path = argv[1];
    }

    printf("=== Ally Plugin System - C Host Example ===\n\n");

    /* Load the calculator plugin (uses standardized ally_get_interface symbol) */
    printf("Loading plugin from: %s\n", plugin_path);
    ally_handle_t handle = ally_load(plugin_path);

    if (!handle) {
        fprintf(stderr, "Failed to load plugin: %s\n", ally_last_error());
        return 1;
    }

    printf("Plugin loaded successfully from: %s\n\n", ally_plugin_path(handle));

    /* Get the plugin interface with helper */
    ally_calculator_type* calc = ally_get_calculator(handle);
    if (!calc) {
        fprintf(stderr, "Failed to get plugin interface\n");
        ally_unload(handle);
        return 1;
    }

    /* Display plugin information */
    printf("Plugin Version: %s\n\n", ally_calculator_version);

    /* Test calculator operations */
    printf("=== Testing Calculator Operations ===\n\n");

    double a = 10.0, b = 5.0;

    printf("%.2f + %.2f = %.2f\n", a, b, calc->add(a, b));
    printf("%.2f - %.2f = %.2f\n", a, b, calc->subtract(a, b));
    printf("%.2f * %.2f = %.2f\n", a, b, calc->multiply(a, b));
    printf("%.2f / %.2f = %.2f\n", a, b, calc->divide(a, b));

    /* Test division by zero */
    printf("%.2f / %.2f = %.2f (division by zero)\n\n", a, 0.0, calc->divide(a, 0.0));

    /* Test memory operations */
    printf("=== Testing Memory Operations ===\n\n");

    double value = 42.0;
    printf("Storing %.2f in memory\n", value);
    calc->store(calc, value);

    printf("Recalling from memory: %.2f\n", calc->recall(calc));

    printf("Clearing memory\n");
    calc->clear(calc);
    printf("Memory after clear: %.2f\n\n", calc->recall(calc));

    /* Unload the plugin */
    printf("Unloading plugin...\n");
    ally_unload(handle);

    printf("Done!\n");

    return 0;
}
