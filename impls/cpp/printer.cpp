#include "printer.hpp"
#include "mal_types.hpp"
#include <string>

using std::string;

string pr_str(MalType* t, bool readable) {
    if (t->type() == String && t->inspect(false) == "\n") {
        return "\n";
    }
    return t->inspect(readable);
}