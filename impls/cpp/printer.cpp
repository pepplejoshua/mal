#include "printer.hpp"
#include "mal_types.hpp"
#include <string>

using std::string;

string pr_str(MalType* t) {
    return t->inspect();
}