#pragma once

#include <algorithm>
#include <cctype>
#include <fstream>
#include <locale>

namespace _helper {
    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }));
    }

    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }).base(),
                s.end());
    }

    static inline void trim(std::string &s) {
        rtrim(s);
        ltrim(s);
    }
} // namespace _helper

class iniparser {
  public:
    typedef std::map<std::string, std::vector<std::map<std::string, std::string>>> value_map_t;

    iniparser(const std::string filename) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error(filename + " could not be opened");
        }

        std::string current_section;
        for (std::string line; std::getline(file, line);) {
            _helper::trim(line);

            if (line.empty()) {
                continue;
            }

            size_t comment_start = line.find_first_of(';');
            if (comment_start != std::string::npos) {
                line = line.substr(0, comment_start - 1);
            }

            if (line[0] == '[') {
                size_t end = line.find_first_of(']');
                if (end == std::string::npos) {
                    throw new std::runtime_error("invalid section");
                }
                current_section = line.substr(1, end - 1);
                values[current_section].emplace_back();
                continue;
            }

            size_t assignment_start = line.find_first_of('=');

            std::string key = line.substr(0, assignment_start - 1);
            _helper::trim(key);

            std::string value = line.substr(assignment_start + 1, line.length());
            _helper::trim(value);

            values[current_section].back().emplace(key, value);
        }
    }

    const std::vector<std::map<std::string, std::string>> &get(const std::string &key) {
        return values[key];
    }

  private:
    value_map_t values;
};