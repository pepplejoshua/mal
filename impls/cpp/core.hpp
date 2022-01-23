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
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'+' requires at least 2 arguments.";
            throw runExcep;
        }

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
    }

    MalType* sub(MalType** args, size_t argc) {
        if (argc < 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'-' requires at least 2 arguments.";
            throw runExcep;
        }

        // check the type of the first argument
        // to set the precedence, else throw on
        // type deviation
        auto f = args[0];
        Type calcType = f->type();

        if (calcType == Int) {
            long diff = f->as_int()->to_long();
            if (argc == 1) {
                diff = -diff;
            } else {
                for (int i = 1; argc > i; ++i) {
                    auto rhs = args[i];
                    if (typeCheck(rhs->type(), Int)) {
                        diff -= rhs->as_int()->to_long();
                    } else {
                        auto typeExcep = TypeException();
                        typeExcep.errMessage = "'-' not defined for operands of varying types.";
                        throw typeExcep;
                    }
                }
            }
            return new MalInt(diff);
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'-' not defined for operands.";
            throw typeExcep;
        }
    }

    MalType* mult(MalType** args, size_t argc) {
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'*' requires at least 2 arguments.";
            throw runExcep;
        }

        // check the type of the first argument
        // to set the precedence, else throw on
        // type deviation
        auto f = args[0];
        Type calcType = f->type();

        if (calcType == Int) {
            long prod = f->as_int()->to_long();
            for (int i = 1; argc > i; ++i) {
                auto rhs = args[i];
                if (typeCheck(rhs->type(), Int)) {
                    prod *= rhs->as_int()->to_long();
                } else {
                    auto typeExcep = TypeException();
                    typeExcep.errMessage = "'*' not defined for operands of varying types.";
                    throw typeExcep;
                }
            }
            return new MalInt(prod);
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'*' not defined for operands.";
            throw typeExcep;
        }
    }

    MalType* div(MalType** args, size_t argc) {
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'/' requires at least 2 arguments.";
            throw runExcep;
        }

        // check the type of the first argument
        // to set the precedence, else throw on
        // type deviation
        auto f = args[0];
        Type calcType = f->type();

        if (calcType == Int) {
            long div_a = f->as_int()->to_long();
            for (int i = 1; argc > i; ++i) {
                auto rhs = args[i];
                if (typeCheck(rhs->type(), Int)) {
                    if (rhs->as_int()->to_long() == 0) {
                        auto runExcep = RuntimeException();
                        runExcep.errMessage = "division by 0 is illegal.";
                        throw runExcep;
                    }
                    div_a /= rhs->as_int()->to_long();
                } else {
                    auto typeExcep = TypeException();
                    typeExcep.errMessage = "'/' not defined for operands of varying types.";
                    throw typeExcep;
                }
            }
            return new MalInt(div_a);
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'/' not defined for operands.";
            throw typeExcep;
        }
    }

    MalType* mod(MalType** args, size_t argc) {
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'%' requires at least 2 arguments.";
            throw runExcep;
        }

        // check the type of the first argument
        // to set the precedence, else throw on
        // type deviation
        auto f = args[0];
        Type calcType = f->type();

        if (calcType == Int) {
            long mod_a = f->as_int()->to_long();
            for (int i = 1; argc > i; ++i) {
                auto rhs = args[i];
                if (typeCheck(rhs->type(), Int)) {
                    if (rhs->as_int()->to_long() == 0) {
                        auto runExcep = RuntimeException();
                        runExcep.errMessage = "modulo by 0 is illegal.";
                        throw runExcep;
                    }
                    mod_a %= rhs->as_int()->to_long();
                } else {
                    auto typeExcep = TypeException();
                    typeExcep.errMessage = "'%' not defined for operands of varying types.";
                    throw typeExcep;
                }
            }
            return new MalInt(mod_a);
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'%' not defined for operands.";
            throw typeExcep;
        }
    }

    MalType* or_(MalType** args, size_t argc) {
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'or' requires at least 2 arguments.";
            throw runExcep;
        }

        // check the type of the first argument
        // to set the precedence, else throw on
        // type deviation
        auto f = args[0];
        Type calcType = f->type();

        if (calcType == Boolean) {
            bool first = f->as_boolean()->val();
            for (int i = 1; argc > i; ++i) {
                auto rhs = args[i];
                if (typeCheck(rhs->type(), Boolean)) {
                    first = first || rhs->as_boolean()->val();
                } else {
                    auto typeExcep = TypeException();
                    typeExcep.errMessage = "'or' not defined for operands of varying types.";
                    throw typeExcep;
                }
            }
            return first ? CONSTANTS["true"] : CONSTANTS["false"];
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'or' not defined for non-boolean operands.";
            throw typeExcep;
        }
    }

    MalType* and_(MalType** args, size_t argc) {
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'and' requires at least 2 arguments.";
            throw runExcep;
        }

        // check the type of the first argument
        // to set the precedence, else throw on
        // type deviation
        auto f = args[0];
        Type calcType = f->type();

        if (calcType == Boolean) {
            bool first = f->as_boolean()->val();
            for (int i = 1; argc > i; ++i) {
                auto rhs = args[i];
                if (typeCheck(rhs->type(), Boolean)) {
                    first = first && rhs->as_boolean()->val();
                } else {
                    auto typeExcep = TypeException();
                    typeExcep.errMessage = "'and' not defined for operands of varying types.";
                    throw typeExcep;
                }
            }
            return first ? CONSTANTS["true"] : CONSTANTS["false"];
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'and' not defined for non-boolean operands.";
            throw typeExcep;
        }
    }

    MalType* exponent(MalType** args, size_t argc) {
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'**' requires at least 2 arguments.";
            throw runExcep;
        }

        // check the type of the first argument
        // to set the precedence, else throw on
        // type deviation
        auto f = args[0];
        Type calcType = f->type();

        if (calcType == Int) {
            long pow_a = f->as_int()->to_long();
            for (int i = 1; argc > i; ++i) {
                auto rhs = args[i];
                if (typeCheck(rhs->type(), Int)) {
                    pow_a = pow(pow_a, rhs->as_int()->to_long());
                } else {
                    auto typeExcep = TypeException();
                    typeExcep.errMessage = "'**' not defined for operands of varying types.";
                    throw typeExcep;
                }
            }
            return new MalInt(pow_a);
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'**' not defined for operands.";
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

    MalType* cons(MalType** args, size_t argc) {
        // not variadic, requires 2 arguments
        if (argc != 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'cons' requires 2 arguments.";
            throw runExcep;
        }
        auto lhs = args[0];
        auto rhs = args[1];

        // allow prepending to list or a vector
        if (typeChecksOneOf(rhs->type(), List, Vector)) {
            MalType* newSeq;
            switch (rhs->type()) {
                case List: {
                    newSeq = new MalList;
                    break;
                }
                default: { // default is for Vector type
                    newSeq = new MalVector;
                }
            }
            auto seq = newSeq->as_sequence();
            seq->append(lhs); // add first item

            auto rhsItems = rhs->as_sequence()->items();
            for (auto item : rhsItems) { // add rhs's items to new sequence
                seq->append(item);
            }
            return newSeq;
        }
        return new MalPair(lhs, rhs);
    }

    MalType* isList(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'list?' requires 1 argument.";
            throw runExcep;
        }

        return args[0]->type() == List ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    MalType* isPair(MalType** args, size_t argc) {
        if (argc != 1) {
                auto runExcep = RuntimeException();
                runExcep.errMessage = "'pair?' requires 1 argument.";
                throw runExcep;
        }

        return args[0]->type() == Pair ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    MalType* isVector(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'vector?' requires 1 argument.";
            throw runExcep;
        }

        return args[0]->type() == Vector ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    MalType* isListOrVecEmpty(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'empty?' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        bool isEmpty = true;
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
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'count' requires 1 argument.";
            throw runExcep;
        }

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
        } else if (item->type() == Nil) {
            count = 0;
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
                // cout << l_con << " == " << r_con << endl;
                if (l_con != r_con)
                    return false;
            } else {
                if (A[i]->inspect() != B[i]->inspect())
                    return false;
            }
        }
        return true;
    }

    // TODO: make variadic when I'm bored
    MalType* isEqual(MalType** args, size_t argc) { 
        if (argc != 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'=' requires 1 argument.";
            throw runExcep;
        }

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
                } else if (lhs->items().size() == 0 && rhs->items().size() == 0) {
                    equal = true;
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
                } else if (lhs->items().size() == 0 && rhs->items().size() == 0) {
                    equal = true;
                } else {
                    auto litems = lhs->items();
                    auto ritems = rhs->items();
                    equal = compareSequenceItems(litems, ritems);
                }
            }
        }
        return equal ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    // implement it for a single list or vector argument:
    //  (< [1 2 3]) => true, just like how
    // (< 1 2 3) => true
    // then consider:
    // (< 1 2 3 [5 6 7] 8 (list 9 10)) => true
    MalType* lessThan(MalType** args, size_t argc) { 
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'<' requires at least 2 arguments.";
            throw runExcep;
        }

        // check the type of the first argument
        // to set the precedence, else throw on
        // type deviation
        auto f = args[0];
        bool lthan = false;
        Type calcType = f->type();

        if (calcType == Int) {
            long lhs = f->as_int()->to_long();
            for (int i = 1; argc > i; ++i) {
                auto rhs = args[i];
                if (typeCheck(rhs->type(), Int)) {
                    lthan = lhs < rhs->as_int()->to_long();
                    // if we examine any pair that returns false, 
                    // end the calculation there
                    if (!lthan) { 
                        break;
                    }
                    lhs = rhs->as_int()->to_long();
                } else {
                    auto typeExcep = TypeException();
                    typeExcep.errMessage = "'<' not defined for operands of varying types.";
                    throw typeExcep;
                }
            }
            return lthan ? CONSTANTS["true"] : CONSTANTS["false"];
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'<' not defined for non-Int operands.";
            throw typeExcep;
        }
    }

    MalType* lessOrEqual(MalType** args, size_t argc) { 
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'<=' requires at least 2 arguments.";
            throw runExcep;
        }

        // check the type of the first argument
        // to set the precedence, else throw on
        // type deviation
        auto f = args[0];
        bool lthan_eq = false;
        Type calcType = f->type();

        if (calcType == Int) {
            long lhs = f->as_int()->to_long();
            for (int i = 1; argc > i; ++i) {
                auto rhs = args[i];
                if (typeCheck(rhs->type(), Int)) {
                    lthan_eq = lhs <= rhs->as_int()->to_long();
                    // if we examine any pair that returns false, 
                    // end the calculation there
                    if (!lthan_eq) { 
                        break;
                    }
                    lhs = rhs->as_int()->to_long();
                } else {
                    auto typeExcep = TypeException();
                    typeExcep.errMessage = "'<=' not defined for operands of varying types.";
                    throw typeExcep;
                }
            }
            return lthan_eq ? CONSTANTS["true"] : CONSTANTS["false"];
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'<=' not defined for non-Int operands.";
            throw typeExcep;
        }
    }

    MalType* greaterThan(MalType** args, size_t argc) { 
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'<' requires at least 2 arguments.";
            throw runExcep;
        }

        // check the type of the first argument
        // to set the precedence, else throw on
        // type deviation
        auto f = args[0];
        bool gthan = false;
        Type calcType = f->type();

        if (calcType == Int) {
            long lhs = f->as_int()->to_long();
            for (int i = 1; argc > i; ++i) {
                auto rhs = args[i];
                if (typeCheck(rhs->type(), Int)) {
                    gthan = lhs > rhs->as_int()->to_long();
                    // if we examine any pair that returns false, 
                    // end the calculation there
                    if (!gthan) { 
                        break;
                    }
                    lhs = rhs->as_int()->to_long();
                } else {
                    auto typeExcep = TypeException();
                    typeExcep.errMessage = "'<' not defined for operands of varying types.";
                    throw typeExcep;
                }
            }
            return gthan ? CONSTANTS["true"] : CONSTANTS["false"];
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'<' not defined for non-Int operands.";
            throw typeExcep;
        }
    }

    MalType* greaterOrEqual(MalType** args, size_t argc) { 
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'>=' requires at least 2 arguments.";
            throw runExcep;
        }

        // check the type of the first argument
        // to set the precedence, else throw on
        // type deviation
        auto f = args[0];
        bool gthan_eq = false;
        Type calcType = f->type();

        if (calcType == Int) {
            long lhs = f->as_int()->to_long();
            for (int i = 1; argc > i; ++i) {
                auto rhs = args[i];
                if (typeCheck(rhs->type(), Int)) {
                    gthan_eq = lhs >= rhs->as_int()->to_long();
                    // if we examine any pair that returns false, 
                    // end the calculation there
                    if (!gthan_eq) { 
                        break;
                    }
                    lhs = rhs->as_int()->to_long();
                } else {
                    auto typeExcep = TypeException();
                    typeExcep.errMessage = "'>=' not defined for operands of varying types.";
                    throw typeExcep;
                }
            }
            return gthan_eq ? CONSTANTS["true"] : CONSTANTS["false"];
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'>=' not defined for non-Int operands.";
            throw typeExcep;
        }
    }

    MalType* pr__str(MalType** args, size_t argc) { 
        string out = "";
        for (int i = 0; argc > i; ++i) {
            out += pr_str(args[i]);
            if (i < argc && i + 1 != argc) {
                out += " ";
            }
        }
        return new MalString(out);
    }

    MalType* str(MalType** args, size_t argc) { 
        string out = "";
        for (int i = 0; argc > i; ++i) {
            out += pr_str(args[i], NULL, false);
            if (i < argc && i + 1 != argc) {
                out += "";
            }
        }
        return new MalString(out);
    }

    MalType* prn(MalType** args, size_t argc) {
        for (int i = 0; argc > i; ++i) {
            cout << pr_str(args[i]);
            if (i < argc && i + 1 != argc) {
                cout << " ";
            }
        }
        cout << "\n";
        return CONSTANTS["nil"];
    }

    MalType* println(MalType** args, size_t argc) { 
        for (int i = 0; argc > i; ++i) {
            cout << pr_str(args[i], NULL, false);
            if (i < argc && i + 1 != argc) {
                cout << " ";
            }
        }
        cout << "\n";
        return CONSTANTS["nil"];
    }

    // TODO: implement for strings
    MalType* sequenceFirst(MalType** args, size_t argc) { 
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'first' requires 1 argument.";
            throw runExcep;
        }

        auto arg = args[0];
        if (typeChecksOneOf(arg->type(), List, Vector)) {
            auto seq = arg->as_sequence();
            if (seq->items().size() >= 1)
                return seq->items()[0];
            else
                return CONSTANTS["nil"];
        } else if (typeCheck(arg->type(), Pair)) {
            auto pair = arg->as_pair();
            // we already know that pair will not be constructed
            // unless 2 MalTypes are provided as lhs and rhs of its
            // construction
            return pair->items()[0];
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'first' not defined for non-sequential operands.";
            throw typeExcep;
        }
    }

    // TODO: implement for strings
    MalType* sequenceRest(MalType** args, size_t argc) { 
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'rest' requires 1 argument.";
            throw runExcep;
        }

        auto arg = args[0];
        if (typeChecksOneOf(arg->type(), List, Vector)) {
            if (arg->type() == List) {
                auto list = arg->as_list()->items();
                if (list.size() > 1) {
                    auto rest = vector < MalType* >(list.begin() + 1, list.end());
                    return new MalList(rest);
                }
                else
                    return new MalList;
                    // return CONSTANTS["nil"];
            } else {
                auto vec = arg->as_vector()->items();
                if (vec.size() > 1) {
                    auto rest = vector < MalType* >(vec.begin() + 1, vec.end());
                    return new MalVector(rest);
                }
                else
                    return new MalVector;
                    // return CONSTANTS["nil"];
            }
        } else if (typeCheck(arg->type(), Pair)) {
            auto pair = arg->as_pair();
            // we already know that pair will not be constructed
            // unless 2 MalTypes are provided as lhs and rhs of its
            // construction
            return pair->items()[1];
        } else {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'rest' not defined for non-sequential operands.";
            throw typeExcep;
        }
    }

    MalType* newline(MalType** args, size_t argc) { 
        if (argc != 0) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'newline' requires no arguments.";
            throw runExcep;
        }

        return CONSTANTS["newline"];
    }

    BuiltIns getCoreBuiltins() {
        BuiltIns core;
        core["+"] = add;
        core["-"] = sub;
        core["*"] = mult;
        core["/"] = div;
        core["%"] = mod;
        core["or"] = or_;
        core["and"] = and_;
        core["**"] = exponent;
        core["pr-str"] = pr__str;
        core["str"] = str;
        core["prn"] = prn;
        core["println"] = println;
        core["list"] = list;
        core["cons"] = cons;
        core["list?"] = isList;
        core["pair?"] = isPair;
        core["vector?"] = isVector;
        core["empty?"] = isListOrVecEmpty;
        core["count"] = sequenceCount;
        core["="] = isEqual;
        core["<"] = lessThan;
        core["<="] = lessOrEqual;
        core[">"] = greaterThan;
        core[">="] = greaterOrEqual;
        core["first"] = sequenceFirst;
        core["rest"] = sequenceRest;
        core["newline"] = newline;
        return core;
    }
}