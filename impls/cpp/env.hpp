#pragma once

#include <map>
#include "mal_types.hpp"

using namespace std;

// allows us store all types of MalTypes
// used in step 2
using Env = map < string, MalType* >;

// used in step 3 and further
class Environ {
public:
    Environ(Environ* parent) : enclosing {parent} { }

    Environ(Environ* parent, vector < MalType* > binds, vector < MalType* > exprs) 
    : enclosing {parent} {
        if (binds.size() != exprs.size()) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "mismatched argument size. ";
            runExcep.errMessage += "expected " + to_string(binds.size()) + " arguments.";
            throw runExcep;
        }

        for (int i = 0; binds.size() > i; ++i) {
            // cout << binds[i]->inspect() << " " << exprs[i]->inspect() << endl;
            set(binds[i], exprs[i]);
        }
    }
    
    void set(MalType* id, MalType* val) {
        stored[id->inspect()] = val;
    }

    MalType* find(MalType* id) {
        auto searched = stored.find(id->inspect());

        if (searched == stored.end()) {
            if (enclosing != NULL) {
                return enclosing->find(id);
            } else {
                return NULL;
            }
        } else {
            return searched->second;
        }
    }

    MalType* get(MalType* id) {
        auto found = find(id);

        if (found != NULL) {
            return found;
        } else {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'" + id->inspect() + "' not found";
            throw runExcep;
        }
    }

private:
    map < string, MalType* > stored;
    Environ* enclosing;
};