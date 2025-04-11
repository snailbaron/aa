#include <aa.hpp>

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    auto help = aa::flag("-h", "--help")
        .help("print help and exit");
    auto verbose = aa::flag("-v", "--verbose")
        .help("print verbose messages");
    auto message = aa::opt<std::string>("-m", "--message")
        .metavar("MESSAGE")
        .required()
        .help("message to print");
    auto times = aa::opt<int>("-n")
        .metavar("N")
        .init(3)
        .help("number of times to repeat the message");
    aa::parse(argc, argv);

    if (help) {
        aa::printHelp(std::cout);
        return 0;
    }

    if (verbose) {
        std::cerr << "I will now print the message " << times << " times\n";
    }
    for (int i = 0; i < times; i++) {
        std::cout << message << "\n";
    }
}
