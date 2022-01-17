#include <string>
#include <string_view>
#include <iostream>
#include "../linenoise.hpp"
#include "reader.hpp"
#include "printer.hpp"
#include "mal_types.hpp"
#include "env.hpp"
#include "core.hpp"

using std::string;
using std::getline;
using std::cout;
using std::cin;
using std::endl;

// // used to cache nil, true and false to be reused by reader
// Env CONSTANTS;
// used as the global environment
Environ* TOP_LEVEL = new Environ(NULL);
auto NIL = new MalNil();
auto TRUE = new MalBoolean(true);
auto FALSE = new MalBoolean(false);
auto VARIADIC = new MalSymbol("&");

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
                    runExcep.errMessage = "let* form requires 2nd argument to be a sequence of bindings.";
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
            } else if (symstr == "if") { // if statement
                // (if condition trueBody falseBody?)
                // condition that isn't nil or false is truthy
                // falseBody? can be forgone for a nil default return:
                // (if condition trueBody)
                if (rawlist.size() < 3 || rawlist.size() > 4) {
                    auto runExcep = RuntimeException();
                    runExcep.errMessage = "if form: (if condition trueBody optionalFalseBody?)";
                    throw runExcep;
                }

                auto cond = rawlist[1];
                auto trueBody = rawlist[2];
                MalType* falseBody = NIL;
                if (rawlist.size() == 4) { falseBody = rawlist[3]; }
                auto e_cond = EVAL(cond, curEnv);
                // nil is nontruthy
                if (e_cond->type() == Nil) {
                    return EVAL(falseBody, curEnv);
                } else if (e_cond->type() == Boolean) {
                    auto bool_cond = e_cond->as_boolean();
                    // false condition
                    if (bool_cond->inspect() == "false") {
                        return EVAL(falseBody, curEnv);
                    }
                    return EVAL(trueBody, curEnv);
                } else
                    return EVAL(trueBody, curEnv);
            } else if (symstr == "fn*") { // function definition
                // make sure it has 3 parameters
                if (rawlist.size() != 3) {
                    auto runExcep = RuntimeException();
                    runExcep.errMessage = "fn* form requires 2 arguments (bindings and a body).";
                    throw runExcep;
                }
                // get bindings and let* body
                auto bindings = rawlist[1];
                auto body = rawlist[2];
                vector < MalType* > fn_params;
                // make sure bindings are in a list or vector
                if (bindings->type() == List) {
                    fn_params = bindings->as_list()->items();
                } else if (bindings->type() == Vector) {
                    fn_params = bindings->as_vector()->items();
                } else {
                    auto runExcep = RuntimeException();
                    runExcep.errMessage = "fn* form requires 2nd argument to be a sequence of bindable Symbols.";
                    throw runExcep;
                }

                bool variadic = false;
                size_t variadic_index = -1;
                vector < MalType* > var_params;
                // check that passed params are either Keywords/Symbols
                for (int i = 0; fn_params.size() > i; ++i) {
                    auto item = fn_params[i];
                    if (item->type() != Symbol) {
                        auto runExcep = RuntimeException();
                        runExcep.errMessage = "fn* parameters have to be bindable Symbols.";
                        throw runExcep;
                    }
                    if (item->inspect() == "&") {
                        if (i + 2 != fn_params.size()) {
                            auto runExcep = RuntimeException();
                            runExcep.errMessage = "variadic function requires 1 variadic parameter.";
                            throw runExcep;
                        }
                        variadic = true;
                        variadic_index = i;
                        continue;
                    }
                    var_params.push_back(item);
                }

                // why does this work????
                auto closure = [&, variadic,  variadic_index, curEnv, var_params, body]
                                (MalType** args, size_t argc) {
                    auto fn_args = vector < MalType* >(args, args + argc);
                    Environ* fnEnv = NULL;

                    // handle arguments properly
                    if (variadic) {
                        // this allows u to pass nothing for the variadic
                        // parameter
                        // handling variadic functions
                        // (def! a (fn* [a b & rest]) (list? rest))
                        // (a 1 2) => then rest is ()
                        // (a 1 2 3 4 5) => then rest is (3 4 5)
                        if (fn_args.size() < (var_params.size() - 1)) {
                            auto runExcep = RuntimeException();
                            runExcep.errMessage = "variadic function requires at least " + to_string(var_params.size() - 1) + " arguments.";
                            throw runExcep;
                        }
                        auto v_arg = new MalList();
                        vector < MalType* > var_args;
                        for (int i = 0; var_params.size() - 1 > i; ++i) {
                            var_args.push_back(fn_args[i]);
                        }

                        for (int i = var_params.size() - 1; fn_args.size() > i; ++i) {
                            v_arg->append(fn_args[i]);
                        }

                        var_args.push_back(v_arg);
                        fnEnv = new Environ(curEnv, var_params, var_args);
                    } else {
                        fnEnv = new Environ(curEnv, var_params, fn_args);
                    }
                    return EVAL(body, fnEnv);
                };
                return new MalFunc(closure, "lambda");
            }
            // if it is neither, it will full through to the bottom
            // and be a function call
        }
        // evaluate with eval_ast, and get new list
        // then call list[0] as a function with 
        // rest of list as it's argument
        auto list = eval_ast(ast, curEnv)->as_list()->items();
        auto callable = list[0];
        // check if fn is built in or a user fn
        auto fn = callable->as_func();
        auto args = list.data() + 1; // start after fn item
        auto argc = list.size() - 1; // count args - fn item
        return fn->callable()(args, argc);
        auto runExcep = RuntimeException();
        runExcep.errMessage = "'" + callable->inspect() + "' is not a callable.";
        throw runExcep;
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
    CONSTANTS["&"] = VARIADIC;
    
    for (auto fn : Core::getCoreBuiltins()) {
        TOP_LEVEL->set(new MalSymbol(fn.first), new MalFunc(fn.second, fn.first));
    }
    
    // create not, and execute it to bind into Env
    // C++ Raw strings require parentheses as delimiters
    // which is ironic, so delimter for this is:
    // code(content)code, with content being actual string
    auto notFn = R"code((def! not
                        (fn* [a]
                            (if a
                                false
                                true))))code";
    Rep(notFn);
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