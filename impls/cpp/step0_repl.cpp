#include <string>
#include <iostream>

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
    while(true) {
        cout << "user> ";
        string input = "";
        // handle ctrl+d closing of stream
        if (!getline(cin, input))
            break;
        if (input == ".q")
            break;
        if (input == "")
            continue;
        cout << Rep(input) << endl;
    }
}

int main() {
    loop();
}
 