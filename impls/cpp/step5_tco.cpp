#include <string>
#include <string_view>
#include <iostream>
#include <memory>
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
auto SPREAD = new MalSpreader();
auto NEWLINE = new MalString("\n");

MalType * READ(string input) {
    auto r = *read_str(input, CONSTANTS);
    return r;
}

MalType * EVAL(MalType *, Environ* curEnv);
MalType * eval_ast(MalType * ast, Environ* curEnv) {
    switch (ast->type()) {
        case Symbol: {
            // check if we have a negated symbol
            // if we do, call MAL's - on it
            auto sym = ast->as_symbol();
            auto symstr = sym->str();
            if (symstr[0] == '-' && symstr.size() > 1) {
                auto actual = new MalSymbol(symstr.substr(1, symstr.size()));
                MalType* arg[1] { curEnv->get(actual) };
                return Core::sub(arg, 1);
            } else {
                return curEnv->get(sym);
            }
        }
        case Keyword: {
            // similar to Symbol
            auto kw = ast->as_keyword();
            auto kwstr = kw->inspect();
            if (kwstr[0] == '-' && kwstr.size() > 1) {
                auto actual = new MalSymbol(kwstr.substr(1, kwstr.size()));
                MalType* arg[1] { curEnv->get(actual) };
                return Core::sub(arg, 1);
            } else {
                return curEnv->get(kw);
            }
        }
        case List: {
            auto results = new MalList;
            for (auto i : ast->as_list()->items()) {
                results->append(EVAL(i, curEnv));
            }
            return results;
        }
        case Vector: {
            auto results = new MalVector;
            for (auto i : ast->as_vector()->items()) {
                results->append(EVAL(i, curEnv));
            }
            return results;
        }
        case HashMap: {
            auto hmap = new MalHashMap;
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

MalType * EVAL(MalType * ast, Environ* curEnv) {
    // we implement tail call optim
    while (true) { 
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
                    // since def! form looks like: (def! a b)
                    // where a is either 
                    // 1. a sequential(List, Vector), which would mean b would also have to be
                    // a spreadable sequential as well (similar to function definitions) and we will bind them in order
                    // e.g:
                    // (def! [a b] [1 2]) -> a is 1 and b is 2, 
                    //       or in a more specialized case, we bind with variadic syntax 
                    // e.g:
                    // (def! [a b c & d] [1 2 3 4 5 6]) -> a is 1, b is 2, c is 3 and d is [4 5 6]
                    // or u can provide arguments that match the number of non-variadic parameters
                    // (def! [a b & c] [1 2])
                    // 2. a scalar, so we bind whatever b is to it
                    // e.g:
                    // (def! a [1 2 3]) or (def! a true) -> a is [1 2 3] or a is true
                    
                    // if we are not handling multiple bindings, 
                    if (!Core::typeChecksOneOf(key->type(), Vector, List)) {
                        // set function's name to key's inspect
                        if (e_val->type() == Func) {
                            auto fn = e_val->as_func();
                            // currently unnamed
                            if (fn->name() == "<~lambda~>") {
                                fn->setName(key->inspect());
                            }
                        } else if (e_val->type() == TCOptFunc) {
                            auto fn = e_val->as_tcoptfunc()->getMalFunc();
                            // currently unnamed
                            if (fn->name() == "<~lambda~>") {
                                fn->setName(key->inspect());
                            }
                        }
                        curEnv->set(key, e_val);
                    } else { // we are handling multiple bindings.
                        // check if e_val (result of evaluating binding value, val) is a sequence 
                        // that we can spread across parameters
                        if (!Core::typeChecksOneOf(e_val->type(), List, Vector)) {
                            auto e = TypeException();
                            e.errMessage = "'" + val->inspect() + "' is not a sequence that can be used in multiple bindings.";
                            throw e;
                        } else {
                            // check if we are looking at multiple bindings with a variadic end
                            bool variadic = false;
                            auto variadic_index = -1;
                            
                            // check that keys are either Symbols or Keywords
                            // and also look out for variadic binding, ensure it is done properly
                            auto keys = key->as_sequence()->items();
                            vector < MalType * > bind_keys;
                            // used to catch duplicated parameter names
                            // e.g (def! [a b b] ... ) is erroneous
                            vector < string > key_insp;
                            for (int i = 0; keys.size() > i; ++i) {
                                auto item = keys[i];
                                auto found = find(key_insp.begin(), key_insp.end(), item->inspect());
                                // duplicate check
                                if (found != key_insp.end()) {
                                    auto runExcep = RuntimeException();
                                    runExcep.errMessage = "def! parameters have to be unique (";
                                    runExcep.errMessage += item->inspect() + " has multiple references).";
                                    throw runExcep;
                                }
                                // type check
                                if (!Core::typeChecksOneOf(item->type(), Symbol, Keyword)) {
                                    auto runExcep = RuntimeException();
                                    runExcep.errMessage = "def! parameters have to be bindable Symbols/Keywords. ";
                                    runExcep.errMessage += "'" + item->inspect() + "' is not.";
                                    throw runExcep;
                                }
                                // variadic check
                                if (item->inspect() == "&") {
                                    if (i + 2 != keys.size()) {
                                        auto runExcep = RuntimeException();
                                        runExcep.errMessage = "variadic binding requires 1 variadic parameter at end of parameters list.";
                                        throw runExcep;
                                    } 

                                    // if we have no non variadic key, this is a somewhat useless operation
                                    // e.g 
                                    // (def! [& a] [1 2 3]) -> a is (1 2 3), which is not much too different from:
                                    // (def! a [1 2 3]) -> a is [1 2 3], which is perhaps more straightforward 
                                    // and takes less processing so suggest a scalar binding 
                                    if (i == 0) { 
                                        auto variad_k = keys[i+1];
                                        auto e = RuntimeException();
                                        e.errMessage = "def! allows multiple bindings with a variadic key\n";
                                        e.errMessage += "only if there is at least some non-variadic key occuring before it.\n";
                                        e.errMessage += "using (def! " + variad_k->inspect() + " " + val->inspect() + ") ";
                                        e.errMessage += "has a similar effect, only differing in the sequential type\n";
                                        e.errMessage += "that " + variad_k->inspect() + " is set to.";
                                        throw e;
                                    }
                                    // cout << "variad! -> " << item->inspect() << " at " + to_string(i) <<  endl;
                                    variadic = true;
                                    variadic_index = i;
                                    continue;
                                }
                                // cout << "non_variad -> " << item->inspect() << endl;
                                bind_keys.push_back(item);
                                key_insp.push_back(item->inspect());
                            }

                            auto bind_args = e_val->as_sequence()->items();
                            if (variadic) {
                                int nonVariadLength = bind_keys.size() - 1;
                                // make sure we have enough items to fill the non-variadic part of bindings
                                // so value args should have at least keys.size() - 1 values
                                if (bind_args.size() < nonVariadLength) {
                                    auto runExcep = RuntimeException();
                                    runExcep.errMessage = "variadic binding requires at least " + to_string(nonVariadLength) + " arguments in value sequence.";
                                    throw runExcep;
                                }
                                // define non-variadic keys
                                for (int i = 0; nonVariadLength > i; ++i) {
                                    curEnv->set(bind_keys[i], bind_args[i]);
                                }
                                // copy the rest of arguments from value sequence into a MalList
                                auto last_variad = new MalList;
                                for (int i = nonVariadLength; bind_args.size() > i; ++i) {
                                    last_variad->append(bind_args[i]);
                                }

                                // then set the variad key to this variad arguements list
                                auto variad_k = keys[variadic_index+1];
                                curEnv->set(variad_k, last_variad);
                            } else {
                                // make sure we have enough arguments
                                if (bind_keys.size() != bind_args.size()) {
                                    auto e = RuntimeException();
                                    e.errMessage = "mismatched key to value size. expected ";
                                    e.errMessage += to_string(bind_keys.size()) + " items in value sequence, got ";
                                    e.errMessage += to_string(bind_args.size()) + " items.";
                                    throw e;
                                }    
                                for (int i = 0; bind_keys.size() > i; ++i) {
                                    curEnv->set(bind_keys[i], bind_args[i]);
                                }
                            }
                        }
                    }
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
                    if (Core::typeChecksOneOf(bindings->type(), List, Vector)) {
                        // instead of doing an eval on body, we need to loop over
                        // after setting things correctly:
                        auto items = bindings->as_sequence()->items();
                        // make sure list has even number of elements
                        if (items.size() % 2 != 0) {
                            auto runExcep = RuntimeException();
                            runExcep.errMessage = "unbalanced binding list (not an even number of elements).";
                            throw runExcep;    
                        }

                        // bind each key in list to its value in let* env
                        for (int i = 0; items.size() > i; i += 2) {
                            auto key = items[i];
                            auto val = items[i+1];
                            letEnv->set(key, EVAL(val, letEnv));
                        }

                        // we do tail call optimization 
                        // set ast = body
                        // set env = letEnv and then restart the loop
                        curEnv = letEnv;
                        ast = body;
                        continue;
                    } else {
                        auto runExcep = RuntimeException();
                        runExcep.errMessage = "let* form requires 2nd argument to be a sequence of bindings.";
                        throw runExcep;
                    }
                } else if (symstr == "do") { // do special form
                    MalType * _AST;
                    for (int i = 1; rawlist.size() > i; ++i) {
                        _AST = rawlist[i];
                        if(i == rawlist.size() - 1) { // final expr in list
                            break;
                        }
                        EVAL(_AST, curEnv);
                    }
                    // we implement tail call recursion by:
                    // setting ast = _AST (last expression in do statement)
                    // and restarting the loop
                    ast = _AST;
                    continue;
                    // return EVAL(_AST, curEnv);
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
                        // we implement tail call optimization by:
                        // setting ast to falseBody in this case
                        // and restart the loop
                        ast = falseBody;
                        continue;
                        // return EVAL(falseBody, curEnv);
                    } else if (e_cond->type() == Boolean) {
                        auto bool_cond = e_cond->as_boolean();
                        // false condition
                        // so we do tail call optimization by:
                        // setting ast to falseBody in this case as well
                        // and restart the loop
                        if (bool_cond->inspect() == "false") {
                            ast = falseBody;
                            continue;
                            // return EVAL(falseBody, curEnv);
                        }
                        // return EVAL(trueBody, curEnv);
                    }
                    // in the case where the boolean value is true,
                    // or just any other cases but nil and false
                    // are considered truthy
                    // so we implement tail call optimization by:
                    // setting ast to trueBody and then restart loop
                    ast = trueBody;
                    continue;
                } else if (symstr == "cond") {
                    // conds are a replacement for repetitive if expressions                    
                    // instead of:
                    // (if cond a (if condB b (if condC c)))
                    // we write:
                    // (cond [cond a] [condB b] [condC c]). a catch all case can use true as its cond as the last case

                    // make sure we have at least 1 cond case following "cond" keyword
                    if (rawlist.size() < 2) {
                        auto runExcep = RuntimeException();
                        runExcep.errMessage = "cond form requires at least one case.";
                        throw runExcep;
                    }

                    // loop through each case, which should be a list or vector
                    // making sure to skip the cond symbol
                    bool matched = false;
                    for (int i = 1; rawlist.size() > i; ++i) {
                        auto item = rawlist[i];
                        if (!Core::typeChecksOneOf(item->type(), List, Vector)) {
                            auto e = TypeException();
                            e.errMessage = "'" + item->inspect() + "' is not a sequence. Each cond case should be a sequence.";
                            throw e;
                        }
                        // make sure cond has 2 items in it
                        auto seq = item->as_sequence()->items();
                        if (seq.size() != 2) {
                            auto e = TypeException();
                            e.errMessage = "Each cond case requires a condition and a body: [cond body].";
                            throw e;
                        }
                        // now we EVAL the condition part of seq and if it's a Boolean true, 
                        // set ast to its body and restart the loop
                        // else we restart the loop
                        // unless we see neither a nil or boolean, we should through an error
                        // (for now)

                        auto cond = EVAL(seq[0], curEnv);
                        if (cond->type() == Nil) {
                            continue;
                        } else if (cond->type() == Boolean && cond == FALSE) {
                            continue;
                        } else if (cond->type() == Boolean && cond == TRUE) {
                            // means we have a truthy value
                            // so set ast to its body and restart loop
                            ast = seq[1];
                            matched = true;
                            break;
                        } else {
                            auto e = TypeException();
                            e.errMessage = "Each cond case's condition should evaluate to Nil or Boolean.";
                            throw e;
                        }
                    }
                    if (matched)
                        continue;
                    else { // in the case of no match, return nil
                        ast = NIL;
                        continue;
                    }
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
                    vector < MalType * > fn_params;
                    // make sure bindings are in a list or vector
                    if (Core::typeChecksOneOf(bindings->type(), List, Vector)) {
                        fn_params = bindings->as_sequence()->items();
                    } else {
                        auto runExcep = RuntimeException();
                        runExcep.errMessage = "fn* form requires 2nd argument to be a sequence of bindable Symbols.";
                        throw runExcep;
                    }

                    bool variadic = false;
                    auto variadic_index = -1;
                    vector < MalType * > var_params;
                    // used to catch duplicated parameter
                    // e.g: (fn* [a b a] ... ) is erroneous
                    vector < string > params_insp; 
                    // check that passed params are either Keywords/Symbols
                    // and also look out for variadic binding, ensure it is done properly
                    for (int i = 0; fn_params.size() > i; ++i) {
                        auto item = fn_params[i];
                        auto found = find(params_insp.begin(), params_insp.end(), item->inspect());
                        // duplicate check
                        if (found != params_insp.end()) {
                            auto runExcep = RuntimeException();
                            runExcep.errMessage = "fn* parameters have to be unique (";
                            runExcep.errMessage += item->inspect() + " has multiple references).";
                            throw runExcep;
                        }
                        // type check
                        if (!Core::typeChecksOneOf(item->type(), Symbol, Keyword)) {
                            auto runExcep = RuntimeException();
                            runExcep.errMessage = "fn* parameters have to be bindable Symbols/Keywords. ";
                            runExcep.errMessage += "'" + item->inspect() + "' is not.";
                            throw runExcep;
                        }
                        // variadic check
                        if (item->inspect() == "&") {
                            if (i + 2 != fn_params.size()) {
                                auto runExcep = RuntimeException();
                                runExcep.errMessage = "variadic function requires 1 variadic parameter at end of parameters list.";
                                throw runExcep;
                            }
                            variadic = true;
                            variadic_index = i;
                            continue;
                        }
                        var_params.push_back(item);
                        params_insp.push_back(item->inspect());
                    }

                    // why does this work????
                    // my question in particular is:
                    // why does capturing the environment in this way work
                    // vs what i had before (which I now cannot recall)
                    // currently, all the code in here has been moved out 
                    // to the apply(call) stage
                    // TODO: unused currently. why?
                    //*********************************************************************
                    auto closure = [&, variadic,  variadic_index, curEnv, var_params, body]
                                    (MalType ** args, size_t argc) {
                        auto fn_args = vector < MalType * >(args, args + argc);
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
                            auto v_arg = new MalList;
                            vector < MalType * > var_args;
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
                    //*********************************************************************
                    // to implement tail call recursion, we need to:
                    // we need to capture these attributes to allow the default apply (or call stage) be
                    // able to tail call optimize its use later down the line:
                    // body -> code to be later run
                    // curEnv -> track the current environment
                    // params -> the func's parameters
                    // fn -> original object that got returned before tco
                    auto actualFn = new MalFunc(closure, "<~lambda~>");
                    // return new MalFunc(closure, "<~lambda~>");
                    return new MalTCOptFunc(body, var_params, curEnv, actualFn, variadic);
                }
                // if it is neither, it will full through to the bottom
                // and be a function call
            }
            // evaluate with eval_ast, and get new list
            // then call list[0] as a function with 
            // rest of list as it's argument
            auto list = eval_ast(ast, curEnv)->as_list()->items();
            auto callable = list[0];

            // process args to find any spread syntax
            auto args = list.data() + 1; // start after fn item
            auto argc = list.size() - 1; // count args - fn item
            vector < MalType * > arguments;
            for (int i = 0; argc > i; ++i) {
                auto item = args[i];
                // we have found an expand in args, so we need to:
                // make sure there is an argument after it, 
                // and it is a sequence. then we take this sequence
                // and add it to our arguments list, and track arguments count
                // as well
                // allows things like:
                // (callable 1 2 3 ... a), where a is [1 2 3] becomes:
                // (callable 1 2 3 1 2 3)
                if (item->type() == Spreader) {
                    if (i + 1 >= argc) {
                        auto e = RuntimeException();
                        e.errMessage = "'...' must be followed by another argument.";
                        throw e;
                    }
                    auto a = args[++i];
                    // cout << obj->inspect() << endl;
                    if (!Core::typeChecksOneOf(a->type(), List, Vector)) {
                        auto e = RuntimeException();
                        e.errMessage = "'...' must be followed by Sequential type (List|Vector).";
                        throw e;
                    }
                    auto items = vector < MalType * >();
                    switch(a->type()) {
                        case List: {
                            items = a->as_list()->items();
                            break;
                        }
                        default: {
                        // default case is for Vectors 
                            items = a->as_vector()->items();
                            break;
                        }
                    }
                    for (auto i : items) {
                        arguments.push_back(i);
                    }
                } else {
                    arguments.push_back(item);
                }
            }
            
            if (Core::typeCheck(callable->type(), Func)) {
                // check if fn is built in or a user fn
                auto a_args = arguments.data();
                auto fn = callable->as_func();
                return fn->callable()(a_args, arguments.size());
            } else if (Core::typeCheck(callable->type(), TCOptFunc)) {
                auto tcofn = callable->as_tcoptfunc();
                // TODO: this is currently unused
                auto internalFn = tcofn->getMalFunc();
                bool variadic = tcofn->isVariad();
                auto envAtTime = tcofn->getEnviron();
                auto params = tcofn->getParameters();
                Environ* newFnEnv = NULL;
                // Environ* newFnEnv = new Environ(envAtTime, params, arguments);
                if (variadic) {
                    if (arguments.size() < (params.size() - 1)) {
                        auto runExcep = RuntimeException();
                        runExcep.errMessage = "variadic function requires at least " + to_string(params.size() - 1) + " arguments.";
                        throw runExcep;
                    }
                    auto variadList = new MalList;
                    vector < MalType* > variad_args;
                    for (int i = 0; params.size() - 1 > i; ++i) {
                        variad_args.push_back(arguments[i]);
                    }

                    for (int i = params.size() - 1; arguments.size() > i; ++i) {
                        variadList->append(arguments[i]);
                    } 

                    variad_args.push_back(variadList);
                    newFnEnv = new Environ(envAtTime, params, variad_args);
                } else {
                    newFnEnv = new Environ(envAtTime, params, arguments);
                }
                ast = tcofn->getBody();
                curEnv = newFnEnv;
                continue;
            }
            auto nonCallable = ast->as_list()->items()[0];
            auto runExcep = RuntimeException();
            runExcep.errMessage = "'" + nonCallable->inspect() + "' is not a callable.";
            throw runExcep;
        }
    }
}
 
string PRINT(MalType * input) {
    return pr_str(input, NEWLINE);
}

string Rep(string input) {
    return PRINT(EVAL(READ(input), TOP_LEVEL));
}

void loop() {
    // find out how and if this works
    // linenoise::SetMultiLine(true);
    linenoise::SetHistoryMaxLen(20);
    string historyPath = "./mem.txt";
    linenoise::LoadHistory(historyPath.c_str());

    string input = "";
    CONSTANTS["nil"] = NIL;
    CONSTANTS["true"] = TRUE;
    CONSTANTS["false"] = FALSE;
    CONSTANTS["&"] = VARIADIC;
    CONSTANTS["..."] = SPREAD;
    CONSTANTS["newline"] = NEWLINE;
    
    for (auto fn : Core::getCoreBuiltins()) {
        auto name = new MalSymbol(fn.first);
        auto builtin = new MalFunc(fn.second, fn.first);
        TOP_LEVEL->set(name, builtin);
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
        } catch (TypeException &t) {
            cerr << t.what() << endl;
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