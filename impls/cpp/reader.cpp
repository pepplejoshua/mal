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

auto NIL = new MalNil();
auto TRUE = new MalBoolean(true);
auto FALSE = new MalBoolean(false);

auto comment = CommentException();

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
        case '\'': // quote
        case '`': // quasiquote
        case '~': // unquote or splice-unquote
            return read_quoted_val(reader);
        case '@': // dereferenced value
            return read_dereferenced_val(reader);
        case '^': // attached meta data dictionary to a MalTypea
            return read_metadata_w_object(reader);
        case ';': // comments
            throw comment;
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
            // can decide to return nil if list is empty
            return list;
        }

        auto item = read_form(reader);
        // not a null optional value
        if (item)
            list->append(item.value());
    }
    // throw reader exception
    auto r_except = ReaderException();
    r_except.errMessage = "unbalanced";
    throw r_except;
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
    // throw reader exception
    auto r_except = ReaderException();
    r_except.errMessage = "unbalanced";
    throw r_except;
}

optional < MalType* > read_hashmap(Reader &reader) {
    // skip over {
    auto lbracket = reader.next().value();

    // not a hashmap?
    // used to guarantee read_metadata_w_object works
    // correctly
    if (lbracket != "{") {
        // throw reader exception
        auto r_except = ReaderException();
        r_except.errMessage = "a hashmap comes after ^ (for metadata)";
        throw r_except;
    }

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
            auto r_except = ReaderException();
            r_except.errMessage = "unbalanced hashmap";
            throw r_except;
        }
        auto val = read_form(reader);
        hmap->set(*key, *val);
    }
    auto r_except = ReaderException();
    r_except.errMessage = "unbalanced";
    throw r_except;
}

bool isNilToken(string_view token) {
    return token == "nil";
}

bool isBooleanToken(string_view token) {
    if (token == "true" || token == "false")
        return true;
    return false;
}

optional < MalType* > read_atom(Reader &reader) {
    auto token = *reader.peek();

    char firstChar = token[0];

    switch (firstChar) {
        case '"': {
            return read_string(reader);
        }
        case ':': {
            return read_keyword(reader);
        }
        default: {
            if (isNilToken(token)) {
                reader.next();
                return NIL;
            }

            if (isBooleanToken(token)) {
                reader.next();
                return token == "true" ? TRUE : FALSE;
            }
            return new MalSymbol(*reader.next());
        }
    }
}

optional < MalType* > read_keyword(Reader &reader) {
    // skip over :
    reader.next().value();
    return new MalKeyword(reader.next().value());
}

optional < MalType* > read_string(Reader &reader) {
    auto token = reader.next().value();

    if (token.length() < 2) {
        auto r_except = ReaderException();
        r_except.errMessage = "unbalanced";
        throw r_except;
    } 

    // unterminated non-empty string
    if (token.length() > 2 && token[token.length()-1] != '"') {
        auto r_except = ReaderException();
        r_except.errMessage = "unbalanced";
        throw r_except;
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
                    auto r_except = ReaderException();
                    r_except.errMessage = "unbalanced";
                    throw r_except;
                }
                char next = stringContent[i];
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

optional < MalType* > read_quoted_val(Reader &reader) {
    string_view token = reader.peek().value();

    char firstChar = token[0];
    switch (firstChar) {
        // quote
        case '\'': {
            // skip '
            reader.next();
            auto quoteList = new MalList();
            quoteList->append(new MalSymbol("quote"));
            quoteList->append(read_form(reader).value());
            return quoteList;
        }
         // quasiquote
        case '`': {
            // skip `
            reader.next();
            auto q_quoteList = new MalList();
            q_quoteList->append(new MalSymbol("quasiquote"));
            q_quoteList->append(read_form(reader).value());
            return q_quoteList;
        }
        // unquote or splice-unquote
        case '~': {
            // skip either ~ or 
            // ~@ (which is tokenized into a single token)
            reader.next();
            // splice-unquote
            if (token.length() > 1 && token[1] == '@') {
                auto s_unquoteList = new MalList();
                s_unquoteList->append(new MalSymbol("splice-unquote"));
                s_unquoteList->append(read_form(reader).value());
                return s_unquoteList;
            } else { // unquote
                auto unquoteList = new MalList();
                unquoteList->append(new MalSymbol("unquote"));
                unquoteList->append(read_form(reader).value());
                return unquoteList;
            }
            break;
        }
        default:
            auto r_except = ReaderException();
            r_except.errMessage = "bad quote";
            throw r_except;
            abort();
    }
}

optional < MalType* > read_dereferenced_val(Reader &reader) {
    reader.next();
    auto deref_list = new MalList();
    deref_list->append(new MalSymbol("deref"));
    deref_list->append(read_form(reader).value());
    return deref_list;
}

optional < MalType* > read_metadata_w_object(Reader &reader) {
    reader.next();
    auto meta_list = new MalList();
    meta_list->append(new MalSymbol("with-meta"));
    auto metadata_hmap = read_hashmap(reader).value();
    auto obj = read_form(reader).value();
    meta_list->append(obj); // read metadata
    meta_list->append(metadata_hmap); // read object
    return meta_list;
}