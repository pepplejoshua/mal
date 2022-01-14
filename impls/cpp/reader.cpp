#include <iostream>
#include <string_view>
#include <string>
#include <exception>
#include <vector>
#include <optional>
#include "mal_types.hpp"
#include "reader.hpp"

using std::string;
using std::string_view;
using std::exception;
using std::optional;
using std::endl;
using std::vector;
using std::cout;

vector < string_view > tokenize(string &input) {
    Tokenizer tokenizer(input);
    vector < string_view > tokens;
    while(auto token = tokenizer.nextToken()) {
        tokens.push_back(*token);
    }
    return tokens;
}

optional < MalType* > read_str(string &input) {
    vector < string_view > tokens = tokenize(input);
    // pass vector of all tokens to reader
    Reader reader(tokens);
    return read_form(reader);
}

optional < MalType* > read_form(Reader &reader) {
    auto token = reader.peek();
    
    // No more token?
    if (!token)
        return {};

    if (token.value() == "(" || token.value() == "[") {
        return read_list(reader);
    } else {
        return read_atom(reader);
    }
}

optional < MalType* > read_list(Reader &reader) {
    // skip over (
    reader.next();

    MalList* list = new MalList();
    bool terminated = false;
    while(auto token = reader.peek()) {
        // end of a list
        if (token.value() == ")" || token.value() == "]") {
            // skip )
            reader.next();
            terminated = true;
            break;
        }

        auto item = read_form(reader);
        // not a null optional value
        if (item)
            list->append(item.value());
    }
    if (!terminated) {
        cout << "(EOF|end of input|unbalanced) "<< endl;
        return {};
    }
    return list;
}

optional < MalType* > read_atom(Reader &reader) {
    return new MalSymbol(*reader.next());
}