#include <aa/parser.hpp>

namespace aa {

namespace internal {

Parser parser;

} // namespace

void parse(int argc, char* argv[])
{
    internal::parser.parse(argc, argv);
}

void printHelp(std::ostream& out)
{
    internal::parser.printHelp(out);
}

} // namespace aa
