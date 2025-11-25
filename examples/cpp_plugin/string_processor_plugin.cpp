/**
 * @file string_processor_plugin.cpp
 * @brief Implementation of the C++ string processor plugin
 */

#include "ally/ally.h"
#include "ally_interface.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

/**
 * @brief Concrete implementation of the string processor plugin interface
 */
class string_processor_impl : public string_processor_plugin {
public:
    /* String transformation methods */
    std::string to_upper(const std::string& str) const override {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return result;
    }

    std::string to_lower(const std::string& str) const override {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return result;
    }

    std::string reverse(const std::string& str) const override {
        std::string result = str;
        std::reverse(result.begin(), result.end());
        return result;
    }

    std::string replace_all(const std::string& str, const std::string& from,
                            const std::string& to) const override {
        if (from.empty()) {
            return str;
        }

        std::string result = str;
        std::size_t pos = 0;

        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.length(), to);
            pos += to.length();
        }

        return result;
    }

    /* String analysis methods */
    std::size_t word_count(const std::string& str) const override {
        if (str.empty()) {
            return 0;
        }

        std::size_t count = 0;
        bool in_word = false;

        for (char c : str) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                in_word = false;
            } else if (!in_word) {
                in_word = true;
                ++count;
            }
        }

        return count;
    }

    std::vector<std::string> split(const std::string& str, char delimiter) const override {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream token_stream(str);

        while (std::getline(token_stream, token, delimiter)) {
            tokens.push_back(token);
        }

        return tokens;
    }

    /* History management methods */
    void add_to_history(const std::string& entry) override {
        history.push_back(entry);
    }

    std::vector<std::string> get_history() const override {
        return history;
    }

    void clear_history() override {
        history.clear();
    }
};

/* Global plugin instance */
static string_processor_impl plugin_instance;

/* Plugin factory function using standardized symbol name */
extern "C" ALLY_EXPORT ally_string_processor_type* ALLY_CALL ally_get_interface(void) {
    return &plugin_instance;
}
