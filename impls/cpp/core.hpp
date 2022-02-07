#pragma once

#include <map>
#include <string>
#include <cmath>
#include <fstream>
#include "mal_types.hpp"
#include "printer.hpp"
#include "reader.hpp"
#include "env.hpp"

using namespace std;

// used to cache nil, true and false to be reused by reader
Env CONSTANTS;

// allows me to call EVAL defined in main
// used as the global environment
Environ* TOP_LEVEL = new Environ(NULL);
// EVAL used by eval
MalType * eval_ast(MalType * ast, Environ* curEnv);
MalType * EVAL(MalType *, Environ* curEnv);

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

    bool typeChecksOneFrom(Type A, vector < Type > Types) {
        for (auto t : Types) {
            if (A == t)
                return true;
        }
        return false;
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
            auto newSeq = new MalList;
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

    MalType* isSequence(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'seq?' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        vector < Type > types { List, Vector, Pair };
        if (!typeChecksOneFrom(item->type(), types)) {
            return CONSTANTS["false"];
        }
        return CONSTANTS["true"];
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
        if (A.size() != B.size()) {
            return false;
        }

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
                case Pair:
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
            switch(arg->type()) {
                case List:
                default: { // default is Vector
                    auto seq = arg->as_sequence()->items();
                    if (seq.size() > 1) {
                        auto rest = vector < MalType * >(seq.begin() + 1, seq.end());
                        return new MalList(rest);
                    }
                    return new MalList;
                }
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

    MalType* sequenceNth(MalType** args, size_t argc) { 
        if (argc != 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'nth' requires 2 arguments.";
            throw runExcep;
        }

        auto a = args[0];
        auto b = args[1];

        vector < Type > atypes { List, Vector, Pair };
        if (!typeChecksOneFrom(a->type(), atypes)) {
            auto e = TypeException();
            e.errMessage = "'nth' requires a Sequence as its first argument.\n";
            e.errMessage += "'" + a->inspect() + "' is not a Sequence.";
            throw e;
        }
        
        if (!typeCheck(b->type(), Int)) {
            auto e = TypeException();
            e.errMessage = "'nth' requires an Int as its second argument.\n";
            e.errMessage += "'" + b->inspect() + "' is not an Int.";
            throw e;
        }
        auto seq = a->as_sequence()->items();
        auto index = b->as_int()->to_long();
        long size = seq.size();

        if (index >= size) {
            auto e = RuntimeException();
            e.errMessage = to_string(index) + " is out of bounds. ";
            e.errMessage += a->inspect() + " has " + to_string(seq.size()) + " items.\n";
            e.errMessage += "indexing starts at 0 and ends at " + to_string(seq.size() - 1) + ".";
            throw e;
        }

        // allowing reverse accessing index with negative indexes
        // supposing a = [1 2 3 4 5] and index = -2
        // we should return 4
        // a.size() + index = 5 - 2 = 3 which is 4
        // make sure a.size() + index >= 0
        if (index < 0) {
            long r_index = size + index;
            if (r_index < 0) {
                auto e = RuntimeException();
                e.errMessage = to_string(index) + " is out of bounds, as it maps to " + to_string(r_index) + ". ";
                e.errMessage += a->inspect() + " has " + to_string(seq.size()) + " items.\n";
                e.errMessage += "indexing starts at 0 and ends at " + to_string(seq.size() - 1) + ".";
                throw e;
            }
            return seq[r_index];
        }
        return seq[index];
    }

    MalType* newline(MalType** args, size_t argc) { 
        if (argc != 0) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'newline' requires no arguments.";
            throw runExcep;
        }

        return CONSTANTS["newline"];
    }

    // this function exposes read_str from reader onto its argument
    // and turns it into a MalType, or returns nil
    MalType* readstring(MalType** args, size_t argc) { 
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'read-string' requires 1 argument.";
            throw runExcep;
        }
        auto item = args[0];
        // make sure argument is a String
        if (!typeCheck(item->type(), String)) {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'read-string' only takes a String argument.";
            throw typeExcep;
        }
        auto src = item->as_string()->inspect(false);
        auto res = read_str(src, CONSTANTS).value_or(CONSTANTS["nil"]);
        return res;
    }

    MalType* slurp(MalType** args, size_t argc) { 
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'slurp' requires 1 argument.";
            throw runExcep;
        }
        auto item = args[0];
        // make sure argument is a String
        if (!typeCheck(item->type(), String)) {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'slurp' only takes a String argument.";
            throw typeExcep;
        }
        auto path = item->as_string()->inspect(false);
        // read file using a closure
        auto fileReader = [&, path]() {
            ifstream file(path);
            if (!file.is_open()) {
                throw system_error(errno, system_category(), "unable to open " + path);
            }

            string content = "";
            string line;
            while(getline(file, line)) {
                content += line + "\\n"; // add back the stripped newline character
            }

            // cout << content << endl;
            return content;
        };
        auto src = fileReader();
        return new MalString(src);
    }

    MalType* eval(MalType** args, size_t argc) { 
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'eval' requires 1 argument.";
            throw runExcep;
        }

        auto ast = args[0];
        return EVAL(ast, TOP_LEVEL);
        // return CONSTANTS["nil"];
    }

    MalType* make_atom(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'atom' requires 1 argument.";
            throw runExcep;
        }

        auto obj = args[0];
        auto atom = new MalAtom(obj);
        atom->setName("(atom " + obj->inspect() + ")");
        return atom;
    }

    MalType* isAtom(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'atom?' requires 1 argument.";
            throw runExcep;
        }
        auto item = args[0];
        return item->type() == Atom ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    MalType* deref(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'atom?' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        if (!typeCheck(item->type(), Atom)) {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'deref' only takes an Atom argument.";
            throw typeExcep;
        }
        auto atom = item->as_atom();
        return atom->deref();
    }

    MalType* reset_atom(MalType** args, size_t argc) {
        if (argc != 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'reset!' requires 2 arguments.";
            throw runExcep;
        }

        auto item = args[0];
        if (!typeCheck(item->type(), Atom)) {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'deref' only takes an Atom argument.";
            throw typeExcep;
        }
        auto atom = item->as_atom();
        auto replacement = args[1];
        atom->reset(replacement);
        return replacement;
    }

    MalType* swap_atom(MalType** args, size_t argc) {
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'swap!' requires at least 2 arguments.";
            throw runExcep;
        }

        // (swap! atom fn & params)
        // so args would be: atom fn and 0 or more arguments to pass to fn
        // have to make sure we have at least the atom and fn
        auto item = args[0];
        auto second = args[1];
        if (!typeCheck(item->type(), Atom)) {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'swap!' requires an Atom as it's first argument.";
            throw typeExcep;
        }   
        auto atom = item->as_atom();

        MalType* callable = NULL;
        if (!typeChecksOneOf(second->type(), TCOptFunc, Func)) {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'swap!' requires a TCOpt|Func as it's second argument.";
            throw typeExcep;
        } else {
            switch (second->type()) {
                case TCOptFunc: {
                    callable = second->as_tcoptfunc();
                    break;
                }
                default: { // for Func
                    callable = second->as_func();
                }
            }
        }

        auto list = new MalList;
        list->append(callable);
        list->append(atom->deref());

        for (int i = 2; argc > i; ++i) {
            list->append(args[i]);
        }
        MalType* items[1] { list };
        auto newVal = eval(items, 1);
        atom->reset(newVal);
        return newVal;
    }

    MalType* sequenceFind(MalType** args, size_t argc) {
        if (argc != 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'find!' requires 2 arguments.";
            throw runExcep;
        }

        auto item = args[0];
        auto key = args[1];
        vector < Type > stypes { Pair, List, Vector };
        // we expect a sequence to use find one
        if (!typeChecksOneFrom(item->type(), stypes)) {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'find' requires a Sequence as it's first argument.";
            throw typeExcep;
        }

        switch (item->type()) {
            case Pair:
            case List:
            case Vector: 
            default: {
                auto seq = item->as_sequence()->items();
                int index = -1;
                bool found = false;
                for (int i = 0; seq.size() > i; ++i) {
                    auto c = seq[i];
                    // if the key and current item are either a list or vector,
                    // we use a helper to tell if they're the same content-wise
                    if (typeChecksOneOf(c->type(), List, Vector) && 
                        typeChecksOneOf(key->type(), List, Vector)) {
                        auto seqA = c->as_sequence()->items();
                        auto seqB = key->as_sequence()->items();
                        if (compareSequenceItems(seqA, seqB)) {
                            found = true;
                            index = i;
                            break;
                        }
                    } else {
                        if (c->inspect() == key->inspect()) {
                            found = true;
                            index = i;
                            break;
                        }
                    }
                }

                if (found) {
                    return new MalInt(index);
                }
                return CONSTANTS["nil"];
            }
        }
    }

    MalType* assoc(MalType** args, size_t argc) {
        // takes a hashmap, a key (could be existing or not) and a value
        // and inserts value at key in provide hashmap
        if (argc < 3) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'assoc' requires at least 3 arguments.";
            throw runExcep;
        }
        // make sure argc - 1 is an even number
        if ((argc - 1) % 2 != 0) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'assoc' requires an even number of arguments to pair keys and values.";
            throw runExcep;
        }

        auto store = args[0];
        if (!typeCheck(store->type(), HashMap)) {
            auto typeExcep = TypeException();
            typeExcep.errMessage = "'assoc' requires a HashMap as it's first argument.";
            throw typeExcep;
        }

        auto hmap = store->as_hashmap();
        auto res = new MalHashMap(*hmap);

        for (int i = 1; argc > i; i = i + 2) {
            auto k = args[i];
            auto v = args[i+1];
            res->set(k->inspect(), k, v);
        }
        return res;
    }

    MalType* concat(MalType** args, size_t argc) {
        auto res = new MalList;
        for (int i = 0; argc > i; ++i) {
            auto arg = args[i];
            // make sure the argument is a List
            if (!typeChecksOneOf(arg->type(), List, Vector)) {
                auto typeExcep = TypeException();
                typeExcep.errMessage = "'concat' requires List|Vector arguments.";
                throw typeExcep;
            }
            auto seq = arg->as_sequence()->items();
            for (int j = 0; seq.size() > j; ++j)
                res->append(seq[j]);
        }
        return res;
    }

    MalType* quasiquote(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'quasiquote' requires 1 argument.";
            throw runExcep;
        }

        auto ast = args[0];
        auto result = new MalList;

        switch (ast->type()) {
            case List: {
                auto list = ast->as_list();
                auto items = list->items();

                if (items.size() < 1) {
                    return list;
                }

                auto first = items[0];
                
                // first test
                if (first->inspect() == "unquote") {
                    return items[1];
                }

                // second test
                int count = items.size() - 1;
                // loop inreverse and then:
                for (int i = count; i >= 0; --i) {
                    auto elem = list->items()[i];
                    // make sure elem is a list, and starts with splice-quote
                    if (typeCheck(elem->type(), List)) {
                        // check for splice-quote
                        auto elems = elem->as_list()->items();
                        
                        if (elems.size() > 0) {
                            auto top = elems[0];

                            if (top->inspect() == "splice-unquote") {
                                // replace current result with a new list that
                                // contains:
                                // concat symbol
                                // elem's second elementsss
                                // result
                                auto newRes = new MalList;
                                newRes->append(new MalSymbol("concat"));
                                newRes->append(elems.at(1));
                                newRes->append(result);
                                result = newRes;
                                continue;
                            }
                        }
                    }
                    // catch all
                    auto newRes = new MalList;
                    newRes->append(new MalSymbol("cons"));
                    // call quasiquote on elem
                    MalType *args[1] { elem };
                    auto q_elem = quasiquote(args, 1);
                    newRes->append(q_elem);
                    newRes->append(result);
                    result = newRes;
                }
                return result;
            }
            case Vector: {
                auto vect = ast->as_vector()->items();
                int count = vect.size() -1;
                for (int i = count; i >= 0; --i) {
                    auto elem = vect[i];
                    // make sure elem is a list, and starts with splice-quote
                    if (typeCheck(elem->type(), List)) {
                        // check for splice-quote
                        auto elems = elem->as_list()->items();
                        
                        if (elems.size() > 0) {
                            auto top = elems[0];

                            if (top->inspect() == "splice-unquote") {
                                // replace current result with a new list that
                                // contains:
                                // concat symbol
                                // elem's second elementsss
                                // result
                                auto newRes = new MalList;
                                newRes->append(new MalSymbol("concat"));
                                newRes->append(elems.at(1));
                                newRes->append(result);
                                result = newRes;
                                continue;
                            }
                        }
                    }

                    auto newRes = new MalList;
                    newRes->append(new MalSymbol("cons"));
                    MalType *args[1] { elem };
                    auto q_elem = quasiquote(args, 1);
                    newRes->append(q_elem);
                    newRes->append(result);
                    result = newRes;
                }
                auto temp = new MalList;
                temp->append(new MalSymbol("vec"));
                temp->append(result);
                result = temp;
                return result;
            }
            case Symbol:
            case HashMap: {
                auto res = new MalList;
                res->append(CONSTANTS["quote"]);
                res->append(ast);
                return res;
            }
            default:
                return ast;
        }
    }

    MalType* vec(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'vec' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];

        switch(item->type()) {
            case List:
            case Pair: {
                auto res = new MalVector;
                auto items = item->as_sequence()->items();
                for (auto i : items) {
                    res->append(i);
                }
                return res;
            }
            case Vector:
                return item;
            default: {
                auto typeExcep = TypeException();
                typeExcep.errMessage = "'vec' requires a Sequence as it's first argument.";
                throw typeExcep;
            }

        }
    }
    
    MalType* type(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'type?' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        return item->stringedType();
    }

    MalType* isMacroCall(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'is_macro_call' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        
        if (!typeCheck(item->type(), List)) {
            return CONSTANTS["false"];
        }
        auto ast = item->as_list()->items();    
        if (ast.size() > 0) {
            auto first = ast[0];
            if (!typeCheck(first->type(), Symbol))
                return CONSTANTS["false"];

            auto found = TOP_LEVEL->find(first); 
            if (found != NULL &&
                typeCheck(found->type(), TCOptFunc)) {
                auto fn = found->as_tcoptfunc();
                if (fn->isMacro())
                    return CONSTANTS["true"];
            }   
        }
        return CONSTANTS["false"];
    }

    MalType* macroExpand(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'macroexpand' requires 1 argument.";
            throw runExcep;
        }

        auto ast = args[0];
        MalType* call_args[1] { ast };
        auto val = isMacroCall(call_args, 1);
        while (val == CONSTANTS["true"]) {
            // so we do have a macro call.
            auto items = ast->as_list()->items();
            // we need to grab the macro itself
            auto macro = TOP_LEVEL->get(items[0]);
            auto list = new MalList;
            // create a call list with the macro in the callable
            // position and then add the other arguments to the list
            // to be EVAL'd
            list->append(macro);
            for (int i = 1; items.size() > i; ++i) {
                list->append(items[i]);
            }
            ast = EVAL(list, TOP_LEVEL);
            call_args[0] = ast;
            val = isMacroCall(call_args, 1);
        }
        return ast;
    }

    
    MalType* throwMalException(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'throw' requires 1 argument.";
            throw runExcep;
        }

        auto throwable = args[0];
        throw throwable;        
    }

    MalType* mapper(MalType** args, size_t argc) {
        if (argc != 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'map' requires 2 arguments.";
            throw runExcep;
        }

        auto cal = args[0];
        auto lst = args[1];

        if (!typeChecksOneOf(cal->type(), Func, TCOptFunc)) {
            auto t = TypeException();
            t.errMessage = "'" + cal->inspect() + "' is not a Callable. map takes a Callable and a Sequence.";
            throw t;
        }

        vector < Type > stypes { List, Vector, Pair };
        if (!typeChecksOneFrom(lst->type(), stypes)) {
            auto t = TypeException();
            t.errMessage = "'" + lst->inspect() + "' is not a Sequence (List, Vector, Pair).\nmap takes a Callable and a Sequence.";
            throw t;
        }

        auto seq = lst->as_sequence()->items();
        auto res = new MalList;
        switch (cal->type()) {
            case Func: {
                auto fn = cal->as_func()->callable();
                // we need to loop through seq,
                // call fn on it and then append it to res
                for (auto i : seq) {
                    MalType* arg[1] { i };
                    res->append(fn(arg, 1));
                }
                return res;
            }
            default: { // User Defined Fn
                auto tco = cal->as_tcoptfunc();

                for (auto i : seq) {
                    auto callList = new MalList;
                    callList->append(tco);
                    callList->append(i);
                    res->append(EVAL(callList, TOP_LEVEL));
                }
            }
        }
        return res;
    }

    MalType* applicator(MalType** args, size_t argc) {
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'apply' requires at least 2 arguments.";
            throw runExcep;
        }

        // make sure the last argument is a list
        auto fn = args[0];
        if (!typeChecksOneOf(fn->type(), Func, TCOptFunc)) {
            auto t = TypeException();
            t.errMessage = "'" + fn->inspect() + "' is not a Callable. map takes a Callable as its first argument.";
            throw t;
        }
        
        auto last = args[argc - 1];
        vector < Type > stypes { List, Vector, Pair };
        if (!typeChecksOneFrom(last->type(), stypes)) {
            auto t = TypeException();
            t.errMessage = "'" + last->inspect() + "' is not a Sequence (List, Vector, Pair).\napply takes a Sequence as its last argument.";
            throw t;
        }

        auto callList = new MalList;
        callList->append(fn);
        for (int i = 1; (argc - 1) > i; ++i) {
            callList->append(args[i]);
        }
        
        auto seq = last->as_sequence()->items();
        for (auto i : seq) {
            callList->append(i);
        }

        return EVAL(callList, TOP_LEVEL);
    }

    MalType* isNil(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'nil?' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        return item->type() == Nil ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    MalType* isTrue(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'true?' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];

        if (item->type() == Boolean) {
            auto actual = item->as_boolean();
            return actual == CONSTANTS["true"] ? CONSTANTS["true"] : CONSTANTS["false"];
        }

        return CONSTANTS["false"];
    }

    MalType* isFalse(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'false?' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];

        if (item->type() == Boolean) {
            auto actual = item->as_boolean();
            return actual == CONSTANTS["false"] ? CONSTANTS["true"] : CONSTANTS["false"];
        }

        return CONSTANTS["false"];
    }

    MalType* isSymbol(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'symbol?' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        return item->type() == Symbol ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    MalType* makeSymbol(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'symbol' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        if (!typeCheck(item->type(), String)) {
            auto t = TypeException();
            t.errMessage = "'" + item->inspect() + "' is not a String. You need a String to make a Symbol.";
            throw t;
        }

        return new MalSymbol(item->inspect(false));
    }

    MalType* makeKeyword(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'keyword' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        if (typeCheck(item->type(), Keyword))
            return item;
        
        if (!typeCheck(item->type(), String)) {
            auto t = TypeException();
            t.errMessage = "'" + item->inspect() + "' is not a String.";
            throw t;
        }

        return new MalKeyword(item->inspect(false));
    }

    MalType* isKeyword(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'keyword?' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        return item->type() == Keyword ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    MalType* makeVector(MalType** args, size_t argc) {
        if (argc < 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'vector' requires at least 1 argument.";
            throw runExcep;
        }

        auto res = new MalVector;
        for (int i = 0; argc > i; ++i) {
            res->append(args[i]);
        }

        return res;
    }

    MalType* isHashMap(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'nil?' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        return item->type() == HashMap ? CONSTANTS["true"] : CONSTANTS["false"];
    }

    MalType* hashMapGet(MalType** args, size_t argc) {
        if (argc != 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'get' requires 2 arguments.";
            throw runExcep;
        }
        
        auto item = args[0];
        if (!typeCheck(item->type(), HashMap)) {
            auto t = TypeException();
            t.errMessage = "'" + item->inspect() + "' is not a HashMap.";
            throw t;
        }
        auto key = args[1];
        auto hmap = item->as_hashmap();
        auto match = hmap->get(key);
        if (match == NULL) {
            return CONSTANTS["nil"];
        }
        return match->as_pair()->items()[0];
    }

    MalType* hashMapContains(MalType** args, size_t argc) {
        if (argc != 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'get' requires 2 arguments.";
            throw runExcep;
        }
        
        auto item = args[0];
        if (!typeCheck(item->type(), HashMap)) {
            auto t = TypeException();
            t.errMessage = "'" + item->inspect() + "' is not a HashMap.";
            throw t;
        }
        auto key = args[1];
        auto hmap = item->as_hashmap();
        auto match = hmap->get(key);
        if (match == NULL) {
            return CONSTANTS["false"];
        }
        return CONSTANTS["true"];
    }

    MalType* makeHashMap(MalType** args, size_t argc) {
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'hash-map' requires at least 2 arguments.";
            throw runExcep;
        }

        if (argc % 2 != 0) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'hash-map' requires an even number of arguments to pair keys and values.";
            throw runExcep;
        }

        auto res = new MalHashMap;
        for (int i = 0; argc > i; i = i + 2) {
            res->set(args[i]->inspect(), args[i], args[i+1]);
        }

        return res;
    }

    MalType* dissoc(MalType** args, size_t argc) {
        if (argc < 2) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'dissoc' requires at least 2 arguments.";
            throw runExcep;
        }

        auto item = args[0];
        if (!typeCheck(item->type(), HashMap)) {
            auto t = TypeException();
            t.errMessage = "'" + item->inspect() + "' is not a HashMap.";
            throw t;
        }

        auto hmap = item->as_hashmap();
        auto items = hmap->items();
        for (int i = 1; argc > i; ++i) {
            auto item = args[i];
            items.erase(item->inspect());
        }

        auto res = new MalHashMap(items);
        return res;
    }

    MalType* hashMapKeysList(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'keys' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        if (!typeCheck(item->type(), HashMap)) {
            auto t = TypeException();
            t.errMessage = "'" + item->inspect() + "' is not a HashMap.";
            throw t;
        }

        auto hmap = item->as_hashmap();
        auto items = hmap->items();

        auto res = new MalList;
        for (auto p : items) {
            auto pair = p.second->as_pair();
            auto key = pair->items()[1];
            res->append(key);
        }
        return res;
    }

    MalType* hashMapValuesList(MalType** args, size_t argc) {
        if (argc != 1) {
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'values' requires 1 argument.";
            throw runExcep;
        }

        auto item = args[0];
        if (!typeCheck(item->type(), HashMap)) {
            auto t = TypeException();
            t.errMessage = "'" + item->inspect() + "' is not a HashMap.";
            throw t;
        }

        auto hmap = item->as_hashmap();
        auto items = hmap->items();

        auto res = new MalList;
        for (auto p : items) {
            auto pair = p.second->as_pair();
            auto val = pair->items()[0];
            res->append(val);
        }
        return res;
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
        core["seq?"] = isSequence;
        core["empty?"] = isListOrVecEmpty;
        core["count"] = sequenceCount;
        core["="] = isEqual;
        core["<"] = lessThan;
        core["<="] = lessOrEqual;
        core[">"] = greaterThan;
        core[">="] = greaterOrEqual;
        core["first"] = sequenceFirst;
        core["rest"] = sequenceRest;
        core["nth"] = sequenceNth;
        core["newline"] = newline;
        core["read-string"] = readstring;
        core["parse"] = readstring;
        core["slurp"] = slurp;
        core["eval"] = eval;
        core["atom"] = make_atom;
        core["atom?"] = isAtom;
        core["deref"] = deref;
        core["reset!"] = reset_atom;
        core["swap!"] = swap_atom;
        core["find"] = sequenceFind;
        core["assoc"] = assoc;
        core["quasiquote"] = quasiquote;
        core["concat"] = concat;
        core["vec"] = vec;
        core["type"] = type;
        core["throw"] = throwMalException;
        core["apply"] = applicator;
        core["map"] = mapper;
        core["sequential?"] = isSequence;
        core["nil?"] = isNil;
        core["true?"] = isTrue;
        core["false?"] = isFalse;
        core["symbol?"] = isSymbol;
        core["symbol"] = makeSymbol;
        core["keyword"] = makeKeyword;
        core["keyword?"] = isKeyword;
        core["vector"] = makeVector;
        core["map?"] = isHashMap;
        core["get"] = hashMapGet;
        core["contains"] = hashMapContains;
        core["hash-map"] = makeHashMap;
        core["dissoc"] = dissoc;
        core["keys"] = hashMapKeysList;
        core["values"] = hashMapValuesList;
        return core;
    }
}