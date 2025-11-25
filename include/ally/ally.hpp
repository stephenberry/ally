/**
 * @file ally.hpp
 * @brief Ally Plugin System - C++ Wrapper
 *
 * This provides a C++ RAII wrapper around the C interface for convenient
 * and safe plugin management in C++ applications.
 *
 * The wrapper is template-based to support loading multiple different plugin
 * types in the same application.
 *
 * @example
 * #include "calculator/ally_interface.h"
 * #include "ally/ally.hpp"
 *
 * ally::plugin_loader<ally_calculator_type> loader;
 * if (loader.load("calculator")) {
 *     auto* calc = loader.get();
 *     calc->add(1, 2);
 * }
 */

#ifndef ALLY_HPP
#define ALLY_HPP

#include "ally/ally.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace ally {

/**
 * @brief RAII wrapper for plugin loading and management
 *
 * This class template automatically handles plugin lifecycle, unloading the plugin
 * when the object is destroyed. It provides a type-safe, exception-based
 * interface for C++ users.
 *
 * @tparam PluginType The plugin interface type (e.g., ally_calculator_type)
 *
 * @example
 * ally::plugin_loader<ally_calculator_type> loader;
 * if (loader.load("./calculator")) {
 *     auto* plugin = loader.get();
 *     // Use plugin...
 * } else {
 *     std::cerr << loader.last_error() << "\n";
 * }
 */
template<class PluginType>
struct plugin_loader {
    /**
     * @brief Construct an empty plugin loader
     */
    plugin_loader() = default;

    /**
     * @brief Load a plugin from path
     *
     * The plugin must export the standardized symbol: ally_get_interface
     *
     * @param plugin_path Path to the plugin (can omit extension)
     * @return true on success, false on failure (check last_error())
     */
    [[nodiscard]] bool load(const std::string& plugin_path) {
        handle_.reset(ally_load(plugin_path.c_str()));
        return static_cast<bool>(handle_);
    }

    /**
     * @brief Unload the plugin
     */
    void unload() { handle_.reset(); }

    /**
     * @brief Get the plugin interface
     * @return Pointer to plugin interface, or nullptr if not loaded
     */
    [[nodiscard]] PluginType* get() const {
        return static_cast<PluginType*>(ally_get(handle_.get()));
    }

    /**
     * @brief Check if plugin is loaded
     */
    [[nodiscard]] bool is_loaded() const { return ally_is_loaded(handle_.get()); }

    /**
     * @brief Get the last error message
     */
    [[nodiscard]] std::string last_error() const { return std::string{ally_last_error()}; }

    /**
     * @brief Get the path to the loaded plugin
     */
    [[nodiscard]] std::string plugin_path() const {
        const char* path = ally_plugin_path(handle_.get());
        return path ? path : "";
    }

    /**
     * @brief Arrow operator for convenient access to plugin interface
     */
    PluginType* operator->() const {
        return get();
    }

    /**
     * @brief Dereference operator
     */
    PluginType& operator*() const {
        return *get();
    }

    /**
     * @brief Bool conversion operator
     */
    explicit operator bool() const {
        return is_loaded();
    }

private:
    using handle_type = std::unique_ptr<std::remove_pointer_t<ally_handle_t>, decltype(&ally_unload)>;
    handle_type handle_{nullptr, ally_unload};
};

/**
 * @brief Load a plugin and throw exception on failure
 *
 * Convenience function that loads a plugin and throws std::runtime_error
 * if loading fails.
 *
 * The plugin must export the standardized symbol: ally_get_interface
 *
 * @tparam PluginType The plugin interface type
 * @param plugin_path Path to the plugin (can omit extension)
 * @return plugin_loader object with loaded plugin
 * @throws std::runtime_error if loading fails
 *
 * @example
 * auto loader = ally::load_plugin<ally_calculator_type>("./calculator");
 * loader->add(1, 2);  // Use plugin
 */
template<class PluginType>
[[nodiscard]] inline plugin_loader<PluginType> load_plugin(const std::string& plugin_path) {
    plugin_loader<PluginType> loader;
    if (!loader.load(plugin_path)) {
        throw std::runtime_error("Failed to load plugin: " + loader.last_error());
    }
    return loader;
}

} // namespace ally

#endif /* ALLY_HPP */
