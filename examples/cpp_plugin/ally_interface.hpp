/**
 * @file ally_interface.hpp
 * @brief Plugin interface definition for C++ string processor plugin
 *
 * This file defines the interface for the string processor plugin using the namespace
 * "string_processor" to avoid collisions with other plugin types.
 *
 * Naming convention:
 *   - ally_string_processor_type: The plugin interface struct type
 *   - ally_string_processor_version: Plugin version (static const)
 *
 * For C++, use the template-based plugin_loader for type-safe loading:
 *   ally::plugin_loader<ally_string_processor_type> loader;
 *
 * Note: Plugin name is derived from the shared library filename.
 */

#pragma once

#include "ally/ally.h"
#include <cstddef>
#include <string>
#include <vector>

/**
 * @brief Plugin version - required
 */
static const char* const ally_string_processor_version = "1.0.0";

/**
 * @brief String processor plugin interface
 *
 * This struct defines the API that the string processor plugin exposes.
 * It demonstrates a more C++-style interface with std::string and methods.
 */
struct string_processor_plugin {
    /* Virtual destructor for proper cleanup */
    virtual ~string_processor_plugin() = default;

    /* String transformation methods - const because they don't modify plugin state */
    virtual std::string to_upper(const std::string& str) const = 0;
    virtual std::string to_lower(const std::string& str) const = 0;
    virtual std::string reverse(const std::string& str) const = 0;
    virtual std::string replace_all(const std::string& str, const std::string& from, const std::string& to) const = 0;

    /* String analysis methods - const because they don't modify plugin state */
    virtual std::size_t word_count(const std::string& str) const = 0;
    virtual std::vector<std::string> split(const std::string& str, char delimiter) const = 0;

    /* Processing history */
    std::vector<std::string> history;
    virtual void add_to_history(const std::string& entry) = 0;
    virtual std::vector<std::string> get_history() const = 0;
    virtual void clear_history() = 0;
};

/**
 * @brief Define namespaced type for this plugin
 *
 * This allows multiple different plugin types to coexist in the same application.
 *
 * @note For C++, use the template-based plugin_loader instead of manual casting:
 *       ally::plugin_loader<ally_string_processor_type> loader;
 */
using ally_string_processor_type = string_processor_plugin;

/**
 * @brief Type-safe helpers for retrieving the string processor interface via C API
 */
inline ally_string_processor_type* ally_get_string_processor(ally_handle_t handle) {
    return static_cast<ally_string_processor_type*>(ally_get(handle));
}
