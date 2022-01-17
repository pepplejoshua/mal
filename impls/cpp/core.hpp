#pragma once

#include <map>
#include <string>
#include <cmath>
#include "mal_types.hpp"
#include "printer.hpp"
#include "env.hpp"

using namespace std;

Env CONSTANTS;

namespace Core {
    using BuiltIns = map < string, Function >;
    
    void assertTypeCheck(Type A, Type Expected) {
        assert(A == Expected);
    }

    bool typeCheck(Type A, Type Expected) {
        return A == Expected;
    }

    bool typeChecksOneOf(Type A, Type ExpectedA, Type ExpectedB) {
        return A == ExpectedA ? true : A == ExpectedB ? true : false;
    }
    
    MalType* add(MalType** args, size_t argc) {
        assert(argc > 0);
        // check the type of the first argument
        // to set the precedence, else throw on
        // type deviation
        auto f = args[0];
        Type calcType = f->type();

        if (calcType == Int) {
            long sum = 0;
            for (int i = 0; argc > i; ++i) {
                auto rhs = args[i];
                if (typeCheck(rhs->type(), Int)) {
                    sum += rhs->as_int()->to_long();
                } else {
                    auto typeExcep = TypeException();
                    typeExcep.errMessage = "'+' not defined for operands of varying types.";
                    throw typeExcep;
                }
            }
            return new MalInt(sum);
        } else if (calcType == String) {
            string res = "";
            for (int i = 0; argc > i; ++i) {
                auto rhs = args[i];
                if (typeCheck(rhs->type(), String)) {
                    res += rhs->as_string()->content();
                } else {
                    auto typeExcep = TypeException();
                    typeExcep.errMessage = "'+' not defined for operands of varying types.";
                    throw typeExcep;
                }
            }
            return new MalString(res);
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'+' not defined for operands.";
            throw typeExcep;
        }

        // assert(argc == 2);
        // auto l = args[0];
        // auto r = args[1];

        // assertTypeCheck(l->type(), r->type());
        // if (typeCheck(l->type(), Int)) {
        //     long sum = l->as_int()->to_long() + r->as_int()->to_long();
        //     return new MalInt(sum);
        // } else if (typeCheck(l->type(), String)) {
        //     string concat = "";
        //     auto lhs = l->as_string()->content();
        //     auto rhs = r->as_string()->content();
        //     concat +=  "\"" + lhs + rhs + "\""; 
        //     return new MalString(concat);
        // } else {
        //     auto typeExcep = TypeException();
        //     typeExcep.errMessage = "'+' not defined for operands.";
        //     throw typeExcep;
        // }
    }

    MalType* sub(MalType** args, size_t argc) {
        assert(argc == 2);
        auto l = args[0];
        auto r = args[1];

        assertTypeCheck(l->type(), r->type());
        if (typeCheck(l->type(), Int)) {
            long diff = l->as_int()->to_long() - r->as_int()->to_long();
            return new MalInt(diff);
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'-' not defined for operands.";
            throw typeExcep;
        }
    }

    MalType* mult(MalType** args, size_t argc) {
        assert(argc == 2);
        auto l = args[0];
        auto r = args[1];

        assertTypeCheck(l->type(), r->type());
        if (typeCheck(l->type(), Int)) {
            long sum = l->as_int()->to_long() * r->as_int()->to_long();
            return new MalInt(sum);
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'*' not defined for operands.";
            throw typeExcep;
        }
    }

    MalType* div(MalType** args, size_t argc) {
        assert(argc == 2);
        auto l = args[0];
        auto r = args[1];

        assertTypeCheck(l->type(), r->type());
        if (typeCheck(l->type(), Int)) {
            long rhs = r->as_int()->to_long();

            if (rhs == 0) {
                auto runExcep = RuntimeException();
                runExcep.errMessage = "division by 0 is illegal.";
                throw runExcep;
            }
            else {
                long sum = l->as_int()->to_long() / rhs;
                return new MalInt(sum);
            }
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'/' not defined for operands.";
            throw typeExcep;
        }
    }

    MalType* or_(MalType** args, size_t argc) {
        assert(argc == 2);
        auto l = args[0];
        auto r = args[1];

        assertTypeCheck(l->type(), r->type());
        if (typeCheck(l->type(), Boolean)) {
            bool lhs = l->as_boolean()->val();
            // short circuit eval:
            // true or false: if lhs is true, return it
            if (lhs) {
                return l;
            }
            // false or true: if lhs isnt true, and rhs is, return it
            bool rhs = r->as_boolean()->val();
            if (rhs) {
                return r;
            }
            else return l; // false or false: if neither is true, return either
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'or' not defined for non-boolean operands.";
            throw typeExcep;
        }
    }

    MalType* and_(MalType** args, size_t argc) {
        assert(argc == 2);
        auto l = args[0];
        auto r = args[1];

        assertTypeCheck(l->type(), r->type());
        if (typeCheck(l->type(), Boolean)) {
            bool lhs = l->as_boolean()->val();
            // short circuit eval:
            // false and true: if lhs is false, return it
            if (!lhs) {
                return l;
            }
            // true and false: if lhs is true, and rhs is false, return rhs
            bool rhs = r->as_boolean()->val();
            if (!rhs) {
                return r;
            }
            else return l; // true and true: if neither is false, return either
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'and' not defined for non-boolean operands.";
            throw typeExcep;
        }
    }

    MalType* exponent(MalType** args, size_t argc) {
        assert(argc == 2);
        auto l = args[0];
        auto r = args[1];

        assertTypeCheck(l->type(), r->type());
        if (typeCheck(l->type(), Int)) {
            long val = pow(l->as_int()->to_long(), r->as_int()->to_long());
            return new MalInt(val);
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'*' not defined for operands.";
            throw typeExcep;
        }
    }

    MalType* list(MalType** args, size_t argc) {
        auto newList = new MalList();
        for (int i = 0; argc > i; ++i) {
            newList->append(args[i]);
        }
        return newList;
    }

    MalType* isList(MalType** args, size_t argc) {
        assert(argc == 1);
        return args[0]->type() == List ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    MalType* isVector(MalType** args, size_t argc) {
        assert(argc == 1);
        return args[0]->type() == Vector ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    MalType* isListOrVecEmpty(MalType** args, size_t argc) {
        assert(argc == 1);
        auto item = args[0];
        bool isEmpty = false;
        if (item->type() == List) {
            auto list = item->as_list()->items();
            isEmpty = list.empty();
        } else if (item->type() == Vector) {
            auto vec = item->as_vector()->items();
            isEmpty = vec.empty();            
        }
        return isEmpty ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    MalType* sequenceCount(MalType** args, size_t argc) {
        assert(argc == 1);
        auto item = args[0];
        size_t count = 0;
        if (item->type() == List) {
            auto list = item->as_list()->items();
            count = list.size();
        } else if (item->type() == Vector) {
            auto vec = item->as_vector()->items();
            count = vec.size();            
        } else if (item->type() == String) {
            auto str = item->as_string()->content();
            count = str.size();
        } else {
            count = 1;
        }
        return new MalInt(count);
    }

    bool compareSequenceItems(vector <MalType* > A, vector < MalType* > B) {
        for (int i = 0; A.size() > i; ++i) {
            auto l = A[i];
            auto r = B[i];

            if (typeChecksOneOf(l->type(), List, Vector) && typeChecksOneOf(r->type(), List, Vector)) {
                auto l_con = l->as_sequence()->contents(false);
                auto r_con = r->as_sequence()->contents(false);
                if (l_con != r_con)
                    return false;
            } else {
                if (A[i]->inspect() != B[i]->inspect())
                    return false;
            }
        }
        return true;
    }

    MalType* isEqual(MalType** args, size_t argc) { 
        assert(argc == 2);
        auto l = args[0];
        auto r = args[1];

        bool equal = false;
        if (typeCheck(l->type(), r->type())) {
            switch (l->type()) {
                case Nil:
                    return CONSTANTS["true"];
                case Boolean: {
                    equal = l->as_boolean()->val() == r->as_boolean()->val();
                    break;
                }
                case Int: {
                    equal = l->as_int()->to_long() == r->as_int()->to_long();
                    break;
                }
                case Keyword: {
                    equal = l->as_keyword()->inspect() == r->as_keyword()->inspect();
                    break;
                }
                case String: {
                    equal = l->as_string()->content() == r->as_string()->content();
                    break;
                }
                case Symbol: {
                    equal = l->as_symbol()->str() == r->as_symbol()->str();
                    break;
                }
                case List:
                case Vector:
                case Func:
                case HashMap: {
                    equal = l->inspect() == r->inspect();
                    break;
                }
                default: 
                    equal = false;
            }
        } else if (typeChecksOneOf(l->type(), List, Vector) && typeChecksOneOf(r->type(), List, Vector)) {
            if (typeCheck(l->type(), List)) { // l is List so r is a Vector
                auto lhs = l->as_list();
                auto rhs = r->as_vector();
                if (lhs->items().size() != rhs->items().size()) {
                    equal = false;
                } else {
                    auto litems = lhs->items();
                    auto ritems = rhs->items();
                    equal = compareSequenceItems(litems, ritems);
                }
            } else { // l is a Vector and r is a List
                auto lhs = l->as_vector();
                auto rhs = r->as_list();

                if (lhs->items().size() != rhs->items().size()) {
                    equal = false;
                } else {
                    auto litems = lhs->items();
                    auto ritems = rhs->items();
                    equal = compareSequenceItems(litems, ritems);
                }
            }
        }
        return equal ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    MalType* lessThan(MalType** args, size_t argc) { 
        assert(argc == 2);
        auto l = args[0];
        auto r = args[1];

        assertTypeCheck(l->type(), r->type());
        if (typeCheck(l->type(), Int)) {
            long lhs = l->as_int()->to_long();
            long rhs = r->as_int()->to_long();
            return lhs < rhs ? CONSTANTS["true"] : CONSTANTS["false"];
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'<' not defined for non-Int operands.";
            throw typeExcep;
        }
    }

    MalType* lessOrEqual(MalType** args, size_t argc) { 
        assert(argc == 2);
        auto l = args[0];
        auto r = args[1];

        assertTypeCheck(l->type(), r->type());
        if (typeCheck(l->type(), Int)) {
            long lhs = l->as_int()->to_long();
            long rhs = r->as_int()->to_long();
            return lhs <= rhs ? CONSTANTS["true"] : CONSTANTS["false"];
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'>=' not defined for non-Int operands.";
            throw typeExcep;
        }
    }

    MalType* greaterThan(MalType** args, size_t argc) { 
        assert(argc == 2);
        auto l = args[0];
        auto r = args[1];

        assertTypeCheck(l->type(), r->type());
        if (typeCheck(l->type(), Int)) {
            long lhs = l->as_int()->to_long();
            long rhs = r->as_int()->to_long();
            return lhs > rhs ? CONSTANTS["true"] : CONSTANTS["false"];
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'>' not defined for non-Int operands.";
            throw typeExcep;
        }
    }

    MalType* greaterOrEqual(MalType** args, size_t argc) { 
        assert(argc == 2);
        auto l = args[0];
        auto r = args[1];

        assertTypeCheck(l->type(), r->type());
        if (typeCheck(l->type(), Int)) {
            long lhs = l->as_int()->to_long();
            long rhs = r->as_int()->to_long();
            return lhs >= rhs ? CONSTANTS["true"] : CONSTANTS["false"];
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'>=' not defined for non-Int operands.";
            throw typeExcep;
        }
    }

    MalType* str(MalType** args, size_t argc) { 
        // calls pr_str(true) on all args, concatenates with " " and then returns it
        string out = "";
        for (int i = 0; argc > i; ++i) {
            out += pr_str(args[i]);
            if (i < argc && i + 1 != argc) {
                out += " ";
            }
        }
        return new MalString(out);
    }

    MalType* str_readable(MalType** args, size_t argc) { 
        // calls pr_str(false) on all args, concatenates with " " and then returns it
        string out = "";
        for (int i = 0; argc > i; ++i) {
            out += pr_str(args[i]);
            if (i < argc && i + 1 != argc) {
                out += " ";
            }
        }
        return new MalString(out);
    }

    // pr_str(true)
    MalType* prn(MalType** args, size_t argc) {
        for (int i = 0; argc > i; ++i) {
            cout << pr_str(args[i]);
            if (i < argc && i + 1 != argc) {
                cout << " ";
            }
        }
        cout << endl;
        return CONSTANTS["nil"];
    }

    // pr_str(false)
    MalType* println(MalType** args, size_t argc) { 
        for (int i = 0; argc > i; ++i) {
            cout << pr_str(args[i]);
            if (i < argc && i + 1 != argc) {
                cout << " ";
            }
        }
        cout << endl;
        return CONSTANTS["nil"];
    }

    BuiltIns getCoreBuiltins() {
        BuiltIns core;
        core["+"] = add;
        core["-"] = sub;
        core["*"] = mult;
        core["/"] = div;
        core["or"] = or_;
        core["and"] = and_;
        core["**"] = exponent;
        core["prn"] = prn;
        core["println"] = println;
        core["str"] = str;
        core["pr-str"] = str_readable;
        core["list"] = list;
        core["list?"] = isList;
        core["vector?"] = isVector;
        core["empty?"] = isListOrVecEmpty;
        core["count"] = sequenceCount;
        core["="] = isEqual;
        core["<"] = lessThan;
        core["<="] = lessOrEqual;
        core[">"] = greaterThan;
        core[">="] = greaterOrEqual;
        return core;
    }
}
