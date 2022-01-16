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
using std::cerr;

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

    char firstChar = token.value()[0];

    switch (firstChar) {
        case '(':
            return read_list(reader);
        case '[':
            return read_vector(reader);
        case '{':
            return read_hashmap(reader);
        default:
            return read_atom(reader);
    }
}

optional < MalType* > read_list(Reader &reader) {
    // skip over (
    reader.next();

    MalList* list = new MalList();
    while(auto token = reader.peek()) {
        // end of a list
        if (token.value() == ")") {
            // skip )
            reader.next();
            return list;
        }

        auto item = read_form(reader);
        // not a null optional value
        if (item)
            list->append(item.value());
    }
    cerr << "unbalanced" << endl;
    return list;
}

optional < MalType* > read_vector(Reader &reader) {
    // skip over (
    reader.next();

    MalVector* vec = new MalVector();
    while(auto token = reader.peek()) {
        // end of a list
        if (token.value() == "]") {
            // skip )
            reader.next();
            return vec;
        }

        auto item = read_form(reader);
        // not a null optional value
        if (item)
            vec->append(item.value());
    }
    cerr << "unbalanced" << endl;
    return vec;
}

optional < MalType* > read_hashmap(Reader &reader) {
    // skip over {
    reader.next();

    MalHashMap* hmap = new MalHashMap();
    while(auto token = reader.peek()) {
        if (token.value() == "}") {
            reader.next();
            return hmap;
        }

        auto key = read_form(reader);

        // make sure this isn't an incomplete hashmap
        token = reader.peek();
        if (token.value() == "}") {
            cerr << "unbalanced" << endl;
            reader.next();
            return hmap;
        }
        auto val = read_form(reader);
        hmap->set(*key, *val);
    }
    cerr << "unbalanced" << endl;
    return hmap;
}


optional < MalType* > read_atom(Reader &reader) {
    auto token = *reader.peek();

    // a string
    if (token[0] == '"') {
        return read_string(reader);
    }
    return new MalSymbol(*reader.next());
}

optional < MalString* > read_string(Reader &reader) {
    auto token = reader.next().value();

    if (token.length() < 2) {
        cerr << "unbalanced" << endl;
        return new MalString(token);
    } 

    // unterminated non-empty string
    if (token.length() > 2 && token[token.length()-1] != '"') {
        cerr << "unbalanced" << endl;
        return new MalString(token);
    } 
    
    if (token.length() == 0) { // empty string
        return new MalString(token);
    }

    // extract string content without the parenthesis
    auto stringContent = token.substr(1, token.length() -2);
    string finalStr = "";
    for (size_t i = 0; i < stringContent.size(); ++i) {
        char c = stringContent[i];
        switch(c) {
            case '\\': {
                ++i;
                if (i >= stringContent.size()) {
                    cerr << "unbalanced" << endl;
                    return new MalString(token);
                }
                char next = stringContent[i];
                // cout << "prev -> " << c << endl;
                // cout << "next -> " << next << endl;
                switch (next) {
                    case 'n':
                        finalStr += "\\n";
                        break;
                    case '\\':
                        finalStr += "\\\\";
                        break;
                    case '"':
                        finalStr += "\\\"";
                        break;
                    default:
                        finalStr += next;
                }
                break;
            }
            default:    
                finalStr += c;
        }
    }
    return new MalString("\"" + finalStr + "\"");
}

optional < MalString* > read_quoted_val(Reader &reader) {
    
}