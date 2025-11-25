/**
 * @file cpp_host.cpp
 * @brief Example C++ host application that loads the string processor plugin
 */

#include <iostream>
#include <string>

/* We need to include the same ally_interface.hpp that the plugin uses */
#include "../cpp_plugin/ally_interface.hpp"
#include "ally/ally.hpp"

int main(int argc, char* argv[]) {
    const char* plugin_path = "./string_processor_plugin";
    if (argc > 1) {
        plugin_path = argv[1];
    }

    std::cout << "=== Ally Plugin System - C++ Host Example ===\n\n";

    /* Load the string processor plugin using C++ wrapper (uses standardized ally_get_interface symbol) */
    std::cout << "Loading plugin from: " << plugin_path << "\n";

    ally::plugin_loader<ally_string_processor_type> loader;
    if (!loader.load(plugin_path)) {
        std::cerr << "Failed to load plugin: " << loader.last_error() << "\n";
        return 1;
    }

    std::cout << "Plugin loaded successfully from: " << loader.plugin_path() << "\n\n";

    /* Get the plugin interface */
    auto* processor = loader.get();
    if (!processor) {
        std::cerr << "Failed to get plugin interface\n";
        return 1;
    }

    /* Display plugin information */
    std::cout << "Plugin Version: " << ally_string_processor_version << "\n\n";

    /* Test string transformation operations */
    std::cout << "=== Testing String Transformations ===\n\n";

    std::string test_str = "Hello, World!";
    std::cout << "Original: " << test_str << "\n";
    std::cout << "Upper: " << processor->to_upper(test_str) << "\n";
    std::cout << "Lower: " << processor->to_lower(test_str) << "\n";
    std::cout << "Reversed: " << processor->reverse(test_str) << "\n\n";

    /* Test string analysis operations */
    std::cout << "=== Testing String Analysis ===\n\n";

    std::string sentence = "The quick brown fox jumps over the lazy dog";
    std::cout << "Sentence: " << sentence << "\n";
    std::cout << "Word count: " << processor->word_count(sentence) << "\n\n";

    std::string csv = "apple,banana,cherry,date";
    std::cout << "CSV: " << csv << "\n";
    std::cout << "Split by comma: ";
    auto tokens = processor->split(csv, ',');
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << tokens[i];
        if (i < tokens.size() - 1) std::cout << " | ";
    }
    std::cout << "\n\n";

    /* Test history management */
    std::cout << "=== Testing History Management ===\n\n";

    processor->add_to_history("First operation");
    processor->add_to_history("Second operation");
    processor->add_to_history("Third operation");

    std::cout << "History:\n";
    auto history = processor->get_history();
    for (size_t i = 0; i < history.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << history[i] << "\n";
    }

    processor->clear_history();
    std::cout << "\nHistory after clear: " << processor->get_history().size() << " entries\n\n";

    /* Test using arrow operator */
    std::cout << "=== Testing Arrow Operator ===\n\n";
    std::cout << "Using loader->to_upper(): " << loader->to_upper("testing arrow operator") << "\n\n";

    /* Plugin is automatically unloaded when loader goes out of scope */
    std::cout << "Plugin will be automatically unloaded\n";
    std::cout << "Done!\n";

    return 0;
}
