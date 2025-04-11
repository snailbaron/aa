#pragma once

#include <aa/error.hpp>
#include <aa/options.hpp>

#include <concepts>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace aa {

class OptionStorage {
public:
    void emplace(char flag, std::shared_ptr<FlagData> data)
    {
        _shortFlags.emplace(flag, std::move(data));
    }

    void emplace(std::string flag, std::shared_ptr<FlagData> data)
    {
        _longFlags.emplace(std::move(flag), std::move(data));
    }

    void emplace(char flag, std::shared_ptr<OptionDataBase> data)
    {
        _shortOptions.emplace(flag, std::move(data));
    }

    void emplace(std::string flag, std::shared_ptr<OptionDataBase> data)
    {
        _longOptions.emplace(std::move(flag), std::move(data));
    }

    std::shared_ptr<FlagData> flag(char c) const
    {
        if (auto it = _shortFlags.find(c); it != _shortFlags.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::shared_ptr<FlagData> flag(const std::string& s) const
    {
        if (auto it = _longFlags.find(s); it != _longFlags.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::shared_ptr<OptionDataBase> option(char c) const
    {
        if (auto it = _shortOptions.find(c); it != _shortOptions.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::shared_ptr<OptionDataBase> option(const std::string& s) const
    {
        if (auto it = _longOptions.find(s); it != _longOptions.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    std::map<char, std::shared_ptr<FlagData>> _shortFlags;
    std::map<std::string, std::shared_ptr<FlagData>> _longFlags;
    std::map<char, std::shared_ptr<OptionDataBase>> _shortOptions;
    std::map<std::string, std::shared_ptr<OptionDataBase>> _longOptions;
};

struct OptionDescription {
    std::vector<std::string> flags;
    std::variant<
        std::shared_ptr<FlagData>,
        std::shared_ptr<OptionDataBase>> data;
};

class Parser {
public:
    template <std::convertible_to<std::string>... Names>
    Flag flag(Names&&... names)
    {
        auto data = std::make_shared<FlagData>();
        (addData(names, data), ...);
        _descriptions.push_back(OptionDescription{{names...}, data});
        return Flag{std::move(data)};
    }

    template <class T, class... Names>
    Option<T> opt(Names&&... names)
    {
        auto data = std::make_shared<OptionData<T>>();
        (addData(names, data), ...);
        _descriptions.push_back(OptionDescription{{names...}, data});
        return Option<T>{std::move(data)};
    }

    void parse(int argc, char* argv[])
    {
        if (argc >= 1) {
            programName(argv[0]);
        }

        std::vector<std::string> args;
        for (int i = 1; i < argc; i++) {
            args.emplace_back(argv[i]);
        }
        parse(args);
    }

    void parse(const std::vector<std::string>& args)
    {
        std::ostringstream errors;

        bool processingFlags = true;
        std::string awaitingOptionName;
        std::shared_ptr<OptionDataBase> awaitingArgument;

        for (const auto& arg : args) {
            if (!processingFlags) {
                _args.push_back(std::move(arg));
            } else if (awaitingArgument) {
                awaitingArgument->parseValue(arg);
                awaitingArgument.reset();
                awaitingOptionName = "";
            } else if (arg == "--") {
                processingFlags = false;
            } else if (arg.length() > 2 && arg.starts_with("--")) {
                auto equ = arg.find('=', 2);
                auto name = arg.substr(0, equ);

                if (auto flag = _ops.flag(name)) {
                    if (equ != std::string::npos) {
                        errors << "aa: option " << name <<
                            " is a flag, and cannot " "accept a value with =\n";
                    }
                    flag->count++;
                } else if (auto op = _ops.option(name)) {
                    if (equ == std::string::npos) {
                        awaitingOptionName = name;
                        awaitingArgument = op;
                    } else {
                        op->parseValue(arg.substr(equ + 1));
                    }
                } else {
                    errors << "unknown option: " << name << "\n";
                }
            } else if (arg.length() >= 2 && arg.front() == '-') {
                for (size_t i = 1; i < arg.length(); i++) {
                    char key = arg.at(i);

                    if (auto op = _ops.flag(key)) {
                        op->count++;
                    } else if (auto op = _ops.option(key)) {
                        if (i + 1 < arg.length()) {
                            op->parseValue(arg.substr(i + 1));
                        } else {
                            awaitingOptionName = std::string{"-"} + key;
                            awaitingArgument = op;
                        }
                        break;
                    } else {
                        errors << "unknown option: -" << key << "\n";
                    }
                }
            } else {
                _args.push_back(arg);
            }
        }

        if (awaitingArgument) {
            errors << "error: option " << awaitingOptionName <<
                " is missing an argument\n";
        }

        for (const auto& description : _descriptions) {
            if (auto p = std::get_if<std::shared_ptr<OptionDataBase>>(
                    &description.data)) {
                if ((*p)->required && !(*p)->hasValue) {
                    errors << "required option is not set: " <<
                        join(description.flags, "|") << "\n";
                }
            }
        }

        if (auto text = std::move(errors).str(); !text.empty()) {
            std::cerr << text;
            throw Error{"parsing failed"};
        }
    }

    void printHelp(std::ostream& out) const
    {
        out << "usage: " << _programName;
        for (const auto& description : _descriptions) {
            const auto& data = description.data;
            bool required =
                std::holds_alternative<std::shared_ptr<OptionDataBase>>(data) &&
                std::get<std::shared_ptr<OptionDataBase>>(data)->required;

            out << " ";
            if (!required) {
                out << "[";
            }
            out << join(description.flags, "|");
            if (auto p = std::get_if<std::shared_ptr<OptionDataBase>>(&data)) {
                out << " " << (*p)->metavar;
            }
            if (!required) {
                out << "]";
            }
        }
        out << "\n";

        out << "options:\n";
        for (const auto& description : _descriptions) {
            out << "  " << join(description.flags, ", ") << " ";
            std::visit([&out] (const auto& d) {
                out << d->help << "\n";
            }, description.data);
        }
    }

    std::string programName() const
    {
        return _programName;
    }

    void programName(std::string name)
    {
        _programName = std::move(name);
    }

private:
    void addData(const std::string& name, const auto& data)
    {
        if (name.length() == 2 && name.at(0) == '-' && name.at(1) != '-') {
            _ops.emplace(name.at(1), data);
        } else if (name.length() > 2 && name.starts_with("--")) {
            _ops.emplace(std::move(name), data);
        } else {
            throw Error{"invalid flag: " + name};
        }
    };

    std::string _programName = "PROGRAM";
    std::vector<std::string> _args;
    OptionStorage _ops;
    std::vector<OptionDescription> _descriptions;
};

namespace internal {

extern Parser parser;

} // namespace internal

void parse(int argc, char* argv[]);
void printHelp(std::ostream& out);

template <std::convertible_to<std::string>... Names>
Flag flag(Names&&... names)
{
    return internal::parser.flag(std::forward<Names>(names)...);
}

template <class T, std::convertible_to<std::string>... Names>
Option<T> opt(Names&&... names)
{
    return internal::parser.opt<T>(std::forward<Names>(names)...);
}

} // namespace aa
