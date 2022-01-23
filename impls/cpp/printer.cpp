#include "printer.hpp"
#include "mal_types.hpp"
#include <string>

using std::string;

string pr_str(MalType* t, MalString* Newline, bool readable) {
    if (t == Newline) {
        return "\n";
    }
    return t->inspect(readable);
}