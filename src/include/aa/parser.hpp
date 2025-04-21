#pragma once

#include <aa/error.hpp>
#include <aa/internal.hpp>
#include <aa/options.hpp>

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace aa {

class Parser {
public:
    template <
        class... Names,
        class = std::enable_if<
            internal::conjunction<
                std::is_convertible<Names, std::string>...>::value>>
    Flag flag(Names&&... names)
    {
        return Flag{addData<void>(false, std::forward<Names>(names)...)};
    }

    template <
        class T,
        class... Names,
        class = std::enable_if<
            internal::conjunction<
                std::is_convertible<Names, std::string>...>::value>>
    Option<T> opt(Names&&... names)
    {
        return Option<T>{addData<T>(true, std::forward<Names>(names)...)};
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
        bool processingFlags = true;

        for (auto arg = args.begin(); arg != args.end(); ) {
            if (!processingFlags) {
                _args.push_back(*arg++);
            } else if (*arg == "--") {
                processingFlags = false;
                ++arg;
            } else if (arg->length() > 2 && internal::startsWith(*arg, "--")) {
                arg = parseLongOption(arg, args.end());
            } else if (arg->length() > 1 && internal::startsWith(*arg, "-")) {
                arg = parseShortOption(arg, args.end());
            } else {
                _args.push_back(*arg++);
            }
        }

        for (const auto& option : _optionList) {
            if (option->required && option->count == 0) {
                _errors << "option " << internal::join(option->flags, ",") <<
                    " is required, but not provided\n";
            }
        }

        auto allErrorText = std::move(_errors).str();
        if (!allErrorText.empty()) {
            std::cerr << allErrorText;
            FAIL("parsing failed");
        }
    }

    void printHelp(std::ostream& out) const
    {
        out << "usage: " << _programName;
        for (const auto& option : _optionList) {
            bool required = option->required;

            out << " ";
            if (!required) {
                out << "[";
            }
            out << internal::join(option->flags, "|");
            if (option->expectsValue) {
                out << " " << option->metavar;
            }
            if (!required) {
                out << "]";
            }
        }
        out << "\n";

        out << "options:\n";
        for (const auto& option : _optionList) {
            out << "  " << internal::join(option->flags, ", ") << " " <<
                option->help << "\n";
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

    template <class T>
    void breakers(T&& bs)
    {
        for (auto&& breaker : bs) {
            _breakers.insert(std::forward<decltype(bs)>(bs));
        }
    }

private:
    template <class T, class... Names>
    std::shared_ptr<TypedOptionData<T>> addData(
        bool expectsValue, Names&&... names)
    {
        auto data = std::make_shared<TypedOptionData<T>>();
        data->flags = {std::forward<Names>(names)...};
        data->expectsValue = expectsValue;

        for (const auto& flag : data->flags) {
            _optionList.push_back(data);
            if (flag.length() == 2 && flag.at(0) == '-' && flag.at(1) != '-') {
                _shortOptions.emplace(flag.at(1), data);
            } else if (flag.length() > 2 && internal::startsWith(flag, "--")) {
                _longOptions.emplace(flag, data);
            } else {
                FAIL("invalid option: " + flag);
            }
        }

        return data;
    };

    template <class I>
    I parseLongOption(I arg, I end)
    {
        auto equ = arg->find('=');
        auto key = arg->substr(0, equ);

        auto optionItr = _longOptions.find(key);
        if (optionItr == _longOptions.end()) {
            _errors << "unknown option: " << key << "\n";
            return std::next(arg);
        }
        auto& option = optionItr->second;

        // TODO: check for "values" of flags here, right away. And below.

        option->count++;
        if (equ != std::string::npos) {
            option->parseValue(arg->substr(equ + 1));
        }
        ++arg;

        if (equ == std::string::npos && arg != end) {
            option->parseValue(*arg++);
        }

        return arg;
    }

    template <class I>
    I parseShortOption(I arg, I end)
    {
        for (size_t i = 1; i < arg->length(); i++) {
            char key = arg->at(i);

            auto optionItr = _shortOptions.find(key);
            if (optionItr == _shortOptions.end()) {
                _errors << "unknown option: -" << key << " in " <<
                    *arg << "\n";
                return std::next(arg);
            }
            auto& option = optionItr->second;

            option->count++;
            if (option->expectsValue && i + 1 < arg->length()) {
                option->parseValue(arg->substr(i + 1));
                return std::next(arg);
            }

            if (option->expectsValue) {
                ++arg;
                if (arg != end) {
                    option->parseValue(*arg++);
                }
                return arg;
            }
        }

        return std::next(arg);
    }

    void checkRestrictions()
    {
    }

    std::string _programName = "PROGRAM";
    std::vector<std::string> _args;
    std::map<char, std::shared_ptr<OptionData>> _shortOptions;
    std::map<std::string, std::shared_ptr<OptionData>> _longOptions;
    std::vector<std::shared_ptr<OptionData>> _optionList;
    std::ostringstream _errors;
    std::set<std::string> _breakers;
};

namespace internal {

inline Parser& parser()
{
    static Parser p;
    return p;
}

} // namespace internal

inline void parse(int argc, char* argv[])
{
    internal::parser().parse(argc, argv);
}

inline void printHelp(std::ostream& out)
{
    internal::parser().printHelp(out);
}

template <
    class... Names,
    class = std::enable_if<
        internal::conjunction<
            std::is_convertible<Names, std::string>...>::value>>
Flag flag(Names&&... names)
{
    return internal::parser().flag(std::forward<Names>(names)...);
}

template <
    class T,
    class... Names,
    class = std::enable_if<
        internal::conjunction<
            std::is_convertible<Names, std::string>...>::value>>
Option<T> opt(Names&&... names)
{
    return internal::parser().opt<T>(std::forward<Names>(names)...);
}

} // namespace aa
