#include "printer.hpp"

using std::string;

string pr_str(MalType* t, MalString* Newline, bool readable) {
    if (t == Newline) {
        return "\n";
    }
    return t->inspect(readable);
}