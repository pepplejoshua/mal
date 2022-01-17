#include <string>
#include <iostream>
#include "../linenoise.hpp"

using std::string;
using std::getline;
using std::cout;
using std::cin;
using std::endl;

string READ(string input) {
    return input;
}

string EVAL(string input) {
    return input;
}


string PRINT(string input) {
    return input;
}

string Rep(string input) {
    return PRINT(EVAL(READ(input)));
}

void loop() {
    string historyPath = "./mem.txt";
    linenoise::LoadHistory(historyPath.c_str());

    string input = "";

    while(true) {   
        bool quit = linenoise::Readline("user> ", input);
        if (quit || input == ".q")
            break;
        if (input == "")
            continue;

        cout << Rep(input) << endl;
        linenoise::AddHistory(input.c_str());
    }

    linenoise::SaveHistory(historyPath.c_str());
}

int main() {
    loop();
}