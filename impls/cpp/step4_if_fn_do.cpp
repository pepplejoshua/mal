#include <string>
#include <string_view>
#include <iostream>
#include "linenoise.hpp"
#include "reader.hpp"
#include "printer.hpp"
#include "mal_types.hpp"
#include "env.hpp"

using std::string;
using std::getline;
using std::cout;
using std::cin;
using std::endl;

// used to cache nil, true and false to be reused by reader
Env CONSTANTS;
// used as the global environment
Environ* TOP_LEVEL = new Environ(NULL);
auto NIL = new MalNil();
auto TRUE = new MalBoolean(true);
auto FALSE = new MalBoolean(false);

void assertTypeCheck(Type A, Type Expected) {
    assert(A == Expected);
}

bool typeCheck(Type A, Type Expected) {
    return A == Expected;
}

// pre-defined functions
MalType* add(MalType** args, size_t argc);
MalType* sub(MalType** args, size_t argc);
MalType* mult(MalType** args, size_t argc);
MalType* div(MalType** args, size_t argc);
MalType* or_(MalType** args, size_t argc);
MalType* and_(MalType** args, size_t argc);

MalType* READ(string input) {
    return *read_str(input, CONSTANTS);
}

MalType* EVAL(MalType*, Environ* curEnv);
MalType* eval_ast(MalType* ast, Environ* curEnv) {
    switch (ast->type()) {
        case Symbol: {
            auto symbol = ast->as_symbol();
            return curEnv->get(symbol);
        }
        case Keyword: {
            auto symbol = ast->as_keyword();
            return curEnv->get(symbol);
        }
        case List: {
            auto results = new MalList();
            for (MalType* i : ast->as_list()->items()) {
                results->append(EVAL(i, curEnv));
            }
            return results;
        }
        case Vector: {
            auto results = new MalVector();
            for (MalType* i : ast->as_vector()->items()) {
                results->append(EVAL(i, curEnv));
            }
            return results;
        }
        case HashMap: {
            auto hmap = new MalHashMap();
            for (auto pair : ast->as_hashmap()->items()) {
                hmap->set(pair.first, EVAL(pair.second, curEnv));
            }
            return hmap;
        }
        default:
            return ast;
    }
    return ast;
}

MalType* eval_let(vector < MalType* >, MalType*, Environ*);
MalType* EVAL(MalType* ast, Environ* curEnv) {
    // not a list, call eval_ast and return its result
    if (ast->type() != List) {
        return eval_ast(ast, curEnv);
    } else if (ast->type() == List && ast->as_list()->items().empty()) {
        // empty list
        return ast;
    } else { // a non-empty list
        // take the first item and check if it is a symbol
        auto rawlist = ast->as_list()->items();
        auto firstItem = rawlist[0];
        if(firstItem->type() == Symbol) {
            // if it is a Symbol, is it def! or let*?
            auto symstr = firstItem->as_symbol()->str();
            // if it is a def!
            if (symstr == "def!") {
                // make sure we have 3 parameters
                if (rawlist.size() != 3) {
                    auto runExcep = RuntimeException();
                    runExcep.errMessage = "def! form requires 2 arguments (symbol and its value).";
                    throw runExcep;
                }
                auto key = rawlist[1];
                auto val = rawlist[2];
                auto e_val = EVAL(val, curEnv);
                curEnv->set(key, e_val);
                return e_val;
            } else if (symstr == "let*") {
                // make sure it has 3 parameters
                if (rawlist.size() != 3) {
                    auto runExcep = RuntimeException();
                    runExcep.errMessage = "let* form requires 2 arguments (bindings and a body).";
                    throw runExcep;
                }
                // get bindings and let* body
                auto bindings = rawlist[1];
                auto body = rawlist[2];
                // create let* env
                auto letEnv = new Environ(curEnv);
                // make sure bindings are in a list
                if (bindings->type() == List) {
                    auto list = bindings->as_list()->items();
                    return eval_let(list, body, letEnv);
                } else if (bindings->type() == Vector) {
                    auto vec = bindings->as_vector()->items();
                    return eval_let(vec, body, letEnv);
                } else {
                    auto runExcep = RuntimeException();
                    runExcep.errMessage = "let* form requires 2nd arguments to be a sequence of bindings.";
                    throw runExcep;
                }
            } else if (symstr == "do") { // do special form
                MalType* _AST = NIL;
                for (int i = 1; rawlist.size() > i; ++i) {
                    _AST = rawlist[i];
                    if(i == rawlist.size() - 1) { // final expr in list
                        break;
                    }
                    EVAL(rawlist[i], curEnv);
                }
                return EVAL(_AST, curEnv);
            }
            // if it is neither, it will full through to the bottom
            // and be a function call
        }
        // evaluate with eval_ast, and get new list
        // then call list[0] as a function with 
        // rest of list as it's argument
        auto list = eval_ast(ast, curEnv)->as_list()->items();
        auto fn = list[0]->as_func();
        auto args = list.data() + 1; // start after fn item
        auto argc = list.size() - 1; // count args - fn item
        return fn->callable()(args, argc);
    }
}

MalType* eval_let(vector < MalType* > bindables, MalType* body, Environ* letEnv) {
    // make sure list has even number of elements
    if (bindables.size() % 2 != 0) {
        auto runExcep = RuntimeException();
        runExcep.errMessage = "unbalanced binding list (not an even number of elements).";
        throw runExcep;    
    }

    // bind each key in list to its value in let* env
    for (int i = 0; bindables.size() > i; i += 2) {
        auto key = bindables[i];
        auto val = bindables[i+1];
        letEnv->set(key, EVAL(val, letEnv));
    }

    return EVAL(body, letEnv);
}
 
string PRINT(MalType* input) {
    return pr_str(input);
}

string Rep(string input) {
    return PRINT(EVAL(READ(input), TOP_LEVEL));
}

void loop() {
    string historyPath = "./mem.txt";
    linenoise::LoadHistory(historyPath.c_str());

    string input = "";
    CONSTANTS["nil"] = NIL;
    CONSTANTS["true"] = TRUE;
    CONSTANTS["false"] = FALSE;

    TOP_LEVEL->set(new MalSymbol("+"), new MalFunc(add, "+"));
    TOP_LEVEL->set(new MalSymbol("-"), new MalFunc(sub, "-"));
    TOP_LEVEL->set(new MalSymbol("*"), new MalFunc(mult, "*"));
    TOP_LEVEL->set(new MalSymbol("/"), new MalFunc(div, "/"));
    TOP_LEVEL->set(new MalSymbol("or"), new MalFunc(or_, "or"));
    TOP_LEVEL->set(new MalSymbol("and"), new MalFunc(and_, "and"));

    while(true) {   
        bool quit = linenoise::Readline("user> ", input);
        if (quit || input == ".q")
            break;
        if (input == "")
            continue;

        try {
            cout << Rep(input) << endl;
            linenoise::AddHistory(input.c_str());
        } catch (ReaderException &e) {
            cerr << e.what() << endl;
            linenoise::AddHistory(input.c_str());
            continue;
        } catch (CommentException &c) {
            linenoise::AddHistory(input.c_str());
            continue;
        } catch (RuntimeException &r) {
            cerr << r.what() << endl;
            linenoise::AddHistory(input.c_str());
            continue;
        }
        linenoise::AddHistory(input.c_str());
    }

    linenoise::SaveHistory(historyPath.c_str());
}



int main() {
    loop();
}
 
MalType* add(MalType** args, size_t argc) {
    assert(argc == 2);
    auto l = args[0];
    auto r = args[1];

    assertTypeCheck(l->type(), r->type());
    if (typeCheck(l->type(), Int)) {
        long sum = l->as_int()->to_long() + r->as_int()->to_long();
        return new MalInt(sum);
    } else if (typeCheck(l->type(), String)) {
        string concat = "";
        auto lhs = l->as_string()->content();
        auto rhs = r->as_string()->content();
        concat +=  "\"" + lhs + rhs + "\""; 
        return new MalString(concat);
    } else {
        auto typeExcep = TypeException();
        typeExcep.errMessage = "'+' not defined for operands.";
        throw typeExcep;
    }
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