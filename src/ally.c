/**
 * @file ally.c
 * @brief Implementation of the Ally plugin system
 */

#define ALLY_INTERNAL_API
#include "ally/ally.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Platform-specific includes */
#ifdef _WIN32
    #include <windows.h>
    typedef HMODULE native_handle_t;
#else
    #include <dlfcn.h>
    #include <limits.h>
    #include <unistd.h>
    typedef void* native_handle_t;
#endif

/* Maximum path length */
#ifndef MAX_PATH_LEN
    #ifdef _WIN32
        #define MAX_PATH_LEN MAX_PATH
    #else
        #define MAX_PATH_LEN PATH_MAX
    #endif
#endif

/* Error message buffer size */
#define ERROR_BUFFER_SIZE 1024

/* Thread-local storage compatibility */
#ifdef _MSC_VER
    #define THREAD_LOCAL __declspec(thread)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define THREAD_LOCAL _Thread_local
#elif defined(__GNUC__)
    #define THREAD_LOCAL __thread
#else
    #define THREAD_LOCAL
#endif

/* Thread-local error message buffer */
static THREAD_LOCAL char error_buffer[ERROR_BUFFER_SIZE];

/* Plugin handle implementation */
struct ally_handle_impl {
    native_handle_t native_handle;
    void* plugin_interface;
    char plugin_path[MAX_PATH_LEN];
};

/**
 * @brief Set the last error message
 */
static void set_error(const char* message) {
    strncpy(error_buffer, message, ERROR_BUFFER_SIZE - 1);
    error_buffer[ERROR_BUFFER_SIZE - 1] = '\0';
}

/**
 * @brief Get platform-specific error message
 */
static void set_platform_error(const char* context) {
#ifdef _WIN32
    DWORD error = GetLastError();
    if (error == 0) {
        set_error(context);
        return;
    }

    char* buffer = NULL;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&buffer, 0, NULL);

    if (size > 0) {
        snprintf(error_buffer, ERROR_BUFFER_SIZE, "%s: %s", context, buffer);
        LocalFree(buffer);
    } else {
        snprintf(error_buffer, ERROR_BUFFER_SIZE, "%s: Error code %lu", context, error);
    }
#else
    const char* error = dlerror();
    if (error) {
        snprintf(error_buffer, ERROR_BUFFER_SIZE, "%s: %s", context, error);
    } else {
        set_error(context);
    }
#endif
}

/**
 * @brief Load a dynamic library
 */
static native_handle_t load_library(const char* path) {
#ifdef _WIN32
    return LoadLibraryA(path);
#else
    return dlopen(path, RTLD_NOW | RTLD_LOCAL);
#endif
}

/**
 * @brief Get a symbol from a dynamic library
 */
static void* get_symbol(native_handle_t handle, const char* name) {
#ifdef _WIN32
    return (void*)GetProcAddress(handle, name);
#else
    return dlsym(handle, name);
#endif
}

/**
 * @brief Close a dynamic library
 */
static void close_library(native_handle_t handle) {
    if (handle) {
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
    }
}

static size_t directory_prefix_length(const char* path) {
    const char* last_forward = strrchr(path, '/');
#ifdef _WIN32
    const char* last_backslash = strrchr(path, '\\');
    if (!last_forward || (last_backslash && last_backslash > last_forward)) {
        last_forward = last_backslash;
    }
#endif
    if (!last_forward) {
        return 0;
    }
    return (size_t)(last_forward - path + 1);
}

/**
 * @brief Try to load library with various path extensions
 */
static native_handle_t try_load_variants(const char* plugin_path, char* resolved_path, size_t path_size) {
    if (!plugin_path || !resolved_path || path_size == 0) {
        return NULL;
    }

#if defined(_WIN32)
    const char* prefixes[] = { "" };
    const char* suffixes[] = { ".dll" };
#elif defined(__APPLE__)
    const char* prefixes[] = { "", "lib" };
    const char* suffixes[] = { ".dylib" };
#else
    const char* prefixes[] = { "", "lib" };
    const char* suffixes[] = { ".so" };
#endif

    native_handle_t handle = load_library(plugin_path);
    if (handle) {
        strncpy(resolved_path, plugin_path, path_size - 1);
        resolved_path[path_size - 1] = '\0';
        return handle;
    }

    /* Try simple prefix/suffix permutations next. */
    char candidate[MAX_PATH_LEN];
    const size_t prefix_count = sizeof(prefixes) / sizeof(prefixes[0]);
    const size_t suffix_count = sizeof(suffixes) / sizeof(suffixes[0]);
    const size_t dir_len = directory_prefix_length(plugin_path);

    for (size_t i = 0; i < prefix_count; ++i) {
        const char* prefix = prefixes[i];
        for (size_t j = 0; j < suffix_count; ++j) {
            const char* suffix = suffixes[j];
            snprintf(candidate, sizeof(candidate), "%.*s%s%s%s",
                     (int)dir_len, plugin_path, prefix, plugin_path + dir_len, suffix);

            handle = load_library(candidate);
            if (handle) {
                strncpy(resolved_path, candidate, path_size - 1);
                resolved_path[path_size - 1] = '\0';
                return handle;
            }
        }
    }

    return NULL;
}

/* Public API implementation */

ally_handle_t ally_load(const char* plugin_path) {
    if (!plugin_path) {
        set_error("Invalid arguments: plugin_path must not be NULL");
        return NULL;
    }

    /* Allocate handle */
    ally_handle_t handle = (ally_handle_t)malloc(sizeof(struct ally_handle_impl));
    if (!handle) {
        set_error("Failed to allocate memory for plugin handle");
        return NULL;
    }

    handle->native_handle = NULL;
    handle->plugin_interface = NULL;
    handle->plugin_path[0] = '\0';

    typedef void* (ALLY_CALL *factory_fn)(void);
    factory_fn factory = NULL;

    /* Try to load the library */
    char resolved_path[MAX_PATH_LEN];
    handle->native_handle = try_load_variants(plugin_path, resolved_path, MAX_PATH_LEN);

    if (!handle->native_handle) {
        char temp_msg[ERROR_BUFFER_SIZE];
        snprintf(temp_msg, ERROR_BUFFER_SIZE, "Failed to load plugin from '%s'", plugin_path);
        set_platform_error(temp_msg);
        goto fail;
    }

    /* Get the factory function using standardized symbol name */
    factory = (factory_fn)get_symbol(handle->native_handle, "ally_get_interface");

    if (!factory) {
        set_platform_error("Failed to find symbol 'ally_get_interface'");
        goto fail;
    }

    /* Get the interface instance */
    handle->plugin_interface = factory();
    if (!handle->plugin_interface) {
        set_error("Factory function returned NULL");
        goto fail;
    }

    /* Store the resolved path */
    strncpy(handle->plugin_path, resolved_path, MAX_PATH_LEN - 1);
    handle->plugin_path[MAX_PATH_LEN - 1] = '\0';

    /* Clear error on success */
    error_buffer[0] = '\0';

    return handle;

fail:
    if (handle->native_handle) {
        close_library(handle->native_handle);
    }
    free(handle);
    return NULL;
}

void* ally_get(ally_handle_t handle) {
    if (!handle) {
        return NULL;
    }
    return handle->plugin_interface;
}

void ally_unload(ally_handle_t handle) {
    if (!handle) {
        return;
    }

    if (handle->native_handle) {
        close_library(handle->native_handle);
        handle->native_handle = NULL;
    }

    handle->plugin_interface = NULL;
    free(handle);
}

bool ally_is_loaded(ally_handle_t handle) {
    return handle != NULL && handle->plugin_interface != NULL;
}

const char* ally_last_error(void) {
    return error_buffer;
}

const char* ally_plugin_path(ally_handle_t handle) {
    if (!handle) {
        return NULL;
    }
    return handle->plugin_path;
}
