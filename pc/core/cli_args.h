#pragma once
#include <string>
#include <vector>
#include <stdexcept>

class CliArgs {
public:
    CliArgs(int argc, char** argv) {
        args_.reserve(argc);
        for (int i = 0; i < argc; i++) args_.push_back(argv[i]);
    }

    bool has(const std::string& key) const {
        for (const auto& a : args_) if (a == key) return true;
        return false;
    }

    // Returns value after key, throws if missing
    std::string value(const std::string& key) const {
        for (size_t i = 0; i < args_.size(); i++) {
            if (args_[i] == key) {
                if (i + 1 >= args_.size()) throw std::runtime_error("Missing value for " + key);
                return args_[i + 1];
            }
        }
        throw std::runtime_error("Unknown argument: " + key);
    }

    // Returns list of raw args for "unknown arg" checking if needed later
    const std::vector<std::string>& raw() const { return args_; }

private:
    std::vector<std::string> args_;
};
