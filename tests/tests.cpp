#include <aa.hpp>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

std::vector<char*> toArgv(std::vector<std::string>& args)
{
    std::vector<char*> results;
    for (auto& arg : args) {
        results.push_back(arg.data());
    }
    return results;
}

TEST_CASE("option types")
{
    auto integer = aa::opt<int>("-i");
    auto floating = aa::opt<float>("-f");
    auto string = aa::opt<std::string>("-s");

    auto args = std::vector<std::string>{
        "program", "-i", "10", "-f", "1.5", "-s", "abc",
    };
    auto argv = toArgv(args);
    aa::parse((int)args.size(), argv.data());

    REQUIRE(integer == 10);
    REQUIRE(floating == 1.5f);
    // TODO: add comparisons to Option?
    REQUIRE(*string == "abc");
}
