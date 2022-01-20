#include <string>
#include <string_view>
#include <iostream>
#include "../linenoise.hpp"
#include "reader.hpp"
#include "printer.hpp"
#include "mal_types.hpp"
#include "core.hpp"

using std::string;
using std::getline;
using std::cout;
using std::cin;
using std::endl;

auto NIL = new MalNil();
auto TRUE = new MalBoolean(true);
auto FALSE = new MalBoolean(false);
auto VARIADIC = new MalSymbol("&");
auto SPREAD = new MalSpreader();

MalType* READ(string input) {
    return *read_str(input, CONSTANTS);
}

MalType* EVAL(MalType* input) {
    return input; 
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
    CONSTANTS["nil"] = NIL;
    CONSTANTS["true"] = TRUE;
    CONSTANTS["false"] = FALSE;
    CONSTANTS["&"] = VARIADIC;
    CONSTANTS["..."] = SPREAD;

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
 