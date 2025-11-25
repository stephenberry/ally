/**
 * @file ally.h
 * @brief Ally Plugin System - Pure C Interface
 *
 * This is the core plugin loading interface for the Ally plugin system.
 * It provides a pure C API for loading shared library plugins.
 *
 * The library is fully generic and type-agnostic. Plugin interfaces are defined
 * in user-provided headers (ally_interface.h or ally_interface.hpp) using a
 * namespace convention based on the plugin filename.
 *
 * Example:
 *   - calculator.so defines: ally_calculator_type, ally_calculator_version
 *   - renderer.so defines: ally_renderer_type, ally_renderer_version
 *
 * This allows multiple different plugin types to be loaded in the same application
 * without name collisions.
 */

#ifndef ALLY_H
#define ALLY_H

#include <stddef.h>
#include <stdbool.h>

/* Platform-specific export/import macros */
#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef ALLY_BUILDING_LIBRARY
        #define ALLY_API __declspec(dllexport)
    #else
        #define ALLY_API __declspec(dllimport)
    #endif
    #define ALLY_CALL __cdecl
    #define ALLY_EXPORT __declspec(dllexport)
#else
    #ifdef ALLY_BUILDING_LIBRARY
        #define ALLY_API __attribute__((visibility("default")))
    #else
        #define ALLY_API
    #endif
    #define ALLY_CALL
    #define ALLY_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle to a loaded plugin
 */
typedef struct ally_handle_impl* ally_handle_t;

/**
 * @brief Load a plugin from a shared library
 *
 * Attempts to load a plugin from the specified path and retrieve the
 * standardized factory function (ally_get_interface) that returns the plugin interface.
 *
 * The loader will automatically try platform-specific variants:
 * - Exact path as given
 * - Path with platform extension (.so, .dylib, .dll)
 * - Path with lib prefix and extension (Unix only)
 *
 * All Ally plugins must export the standardized symbol name: ally_get_interface
 *
 * @param plugin_path Path to the plugin library (can omit extension)
 * @return Handle to the loaded plugin, or NULL on failure
 *
 * @example
 * ally_handle_t handle = ally_load("./my_plugin");
 * if (handle) {
 *     ally_type* plugin = ally_get(handle);
 *     // Use plugin...
 *     ally_unload(handle);
 * }
 */
ALLY_API ally_handle_t ALLY_CALL ally_load(const char* plugin_path);

/**
 * @brief Get the plugin interface from a loaded plugin
 *
 * Returns a void* that should be cast to the appropriate plugin type.
 * For type-safe access, use the helper functions provided in your
 * plugin's ally_interface.h header (e.g., ally_get_calculator()).
 *
 * @param handle Handle returned by ally_load()
 * @return Pointer to the plugin interface (void*), or NULL if handle is invalid
 */
ALLY_API void* ALLY_CALL ally_get(ally_handle_t handle);

/**
 * @brief Unload a plugin and free associated resources
 *
 * After calling this function, the handle becomes invalid and should not be used.
 * The plugin interface pointer returned by ally_get() also becomes invalid.
 *
 * @param handle Handle to unload (can be NULL, in which case this is a no-op)
 */
ALLY_API void ALLY_CALL ally_unload(ally_handle_t handle);

/**
 * @brief Check if a plugin is currently loaded
 *
 * @param handle Handle to check
 * @return true if the plugin is loaded, false otherwise
 */
ALLY_API bool ALLY_CALL ally_is_loaded(ally_handle_t handle);

/**
 * @brief Get the last error message from a failed operation
 *
 * Returns a pointer to a thread-local buffer containing the error message.
 * This string is valid until the next call to any ally_* function on the
 * same thread. Calls from other threads do not affect this thread's error.
 *
 * @return Error message string, or empty string if no error
 */
ALLY_API const char* ALLY_CALL ally_last_error(void);

/**
 * @brief Get the full path to the loaded plugin library
 *
 * Returns a pointer to internal storage. This pointer is valid only while
 * the handle remains loaded. After ally_unload(), the pointer becomes invalid.
 *
 * @param handle Handle to query
 * @return Full path to the plugin library, or NULL if handle is invalid
 */
ALLY_API const char* ALLY_CALL ally_plugin_path(ally_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* ALLY_H */
