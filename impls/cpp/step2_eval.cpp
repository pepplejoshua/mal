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

Env global;
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

MalType* READ(string input) {
    return *read_str(input, global);
}

MalType* EVAL(MalType*);
MalType* eval_ast(MalType* ast) {
    switch (ast->type()) {
        case Symbol: {
            auto symstr = ast->as_symbol()->str();
            auto search = global.find(symstr);
            if (search == global.end()) {
                auto runExcep = RuntimeException();
                runExcep.errMessage = "reference to unbound variable '" + symstr + "'.";
                throw runExcep;
            }
            return search->second;
        }
        case List: {
            auto results = new MalList();
            for (MalType* i : ast->as_list()->items()) {
                results->append(EVAL(i));
            }
            return results;
        }
        case Vector: {
            auto results = new MalVector();
            for (MalType* i : ast->as_vector()->items()) {
                results->append(EVAL(i));
            }
            return results;
        }
        case HashMap: {
            auto hmap = new MalHashMap();
            for (auto pair : ast->as_hashmap()->items()) {
                hmap->set(pair.first, EVAL(pair.second));
            }
            return hmap;
        }
        default:
            return ast;
    }
    return ast;
}

MalType* EVAL(MalType* ast) {
    // not a list, call eval_ast and return its result
    if (ast->type() != List) {
        return eval_ast(ast);
    } else if (ast->type() == List && ast->as_list()->items().empty()) {
        // empty list
        return ast;
    } else {
        // evaluate with eval_ast, and get new list
        // then call list[0] as a function with 
        // rest of list as it's argument
        auto list = eval_ast(ast)->as_list()->items();
        auto fn = list[0]->as_func();
        auto args = list.data() + 1; // start after fn item
        auto argc = list.size() - 1; // count args - fn item
        return fn->callable()(args, argc);
    }
}
 
string PRINT(MalType* input) {
    return pr_str(input);
}

string Rep(string input) {
    return PRINT(EVAL(READ(input)));
}

void loop() {
    string historyPath = "./mem.txt";
    linenoise::LoadHistory(historyPath.c_str());

    string input = "";
    global["nil"] = NIL;
    global["true"] = TRUE;
    global["false"] = FALSE;
    global["+"] = new MalFunc(add, "+");
    global["-"] = new MalFunc(sub, "-");
    global["*"] = new MalFunc(mult, "*");
    global["/"] = new MalFunc(div, "/");

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
