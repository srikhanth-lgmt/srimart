#pragma once

#include <cstdlib>
#include <fstream>
#include <string>
#include <algorithm>

namespace srikanth::srikanthmart {

inline std::string TrimEnvironmentValue(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    value = value.substr(first, last - first + 1);
    if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                              (value.front() == '\'' && value.back() == '\''))) {
        value = value.substr(1, value.size() - 2);
    }
    return value;
}

// Local .env values fill gaps only; host/platform variables always take precedence.
inline void LoadLocalEnvironment(const std::string& file_path = ".env") {
    std::ifstream file(file_path);
    std::string line;
    while (std::getline(file, line)) {
        line = TrimEnvironmentValue(line);
        if (line.empty() || line.front() == '#') continue;
        const auto equals = line.find('=');
        if (equals == std::string::npos) continue;
        const std::string key = TrimEnvironmentValue(line.substr(0, equals));
        const std::string value = TrimEnvironmentValue(line.substr(equals + 1));
        if (key.empty() || std::getenv(key.c_str()) != nullptr) continue;
#ifdef _WIN32
        _putenv_s(key.c_str(), value.c_str());
#else
        setenv(key.c_str(), value.c_str(), 0);
#endif
    }
}

inline std::string GetEnvironmentValue(const char* key, const std::string& fallback) {
    const char* value = std::getenv(key);
    return value ? value : fallback;
}

} // namespace srikanth::srikanthmart
