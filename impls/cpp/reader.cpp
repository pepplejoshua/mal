#include <charconv>
#include "reader.hpp"

using std::string;
using std::string_view;
using std::exception;
using std::optional;
using std::endl;
using std::vector;
using std::cout;
using std::cerr;

Env glob;
vector < string_view > tokenize(string &input) {
    Tokenizer tokenizer(input);
    vector < string_view > tokens;
    while(auto token = tokenizer.nextToken()) {
        tokens.push_back(*token);
    }
    return tokens;
}

optional < MalType * > read_str(string &input, Env global) {
    vector < string_view > tokens = tokenize(input);
    glob = global;
    // pass vector of all tokens to reader
    Reader reader(tokens);
    return read_form(reader);
}

optional < MalType * > read_form(Reader &reader) {
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
        default:
            return read_atom(reader);
    }
}

optional < MalType * > read_list(Reader &reader) {
    // skip over (
    reader.next();

    auto list = new MalList();
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
        if (item) {
            list->append(item.value());
        }
    }
    // throw reader exception
    auto r_except = ReaderException();
    r_except.errMessage = "unbalanced";
    throw r_except;
}

optional < MalType * > read_vector(Reader &reader) {
    // skip over (
    reader.next();

    auto vec = new MalVector;
    while(auto token = reader.peek()) {
        // end of a list
        if (token.value() == "]") {
            // skip )
            reader.next();
            return vec;
        }

        auto item = read_form(reader);
        // not a null optional value
        if (item) {
            vec->append(item.value());
        }
    }
    // throw reader exception
    auto r_except = ReaderException();
    r_except.errMessage = "unbalanced";
    throw r_except;
}

optional < MalType * > read_hashmap(Reader &reader) {
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

    auto hmap = new MalHashMap;
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
        hmap->set(key.value()->inspect(), *val);
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

bool isNumberToken(string_view token) {
    // isdigit returns 0 when i is not a digit
    // or is a negative number
    if (isdigit(token[0]) || (token[0] == '-' && token.size() > 1)) {
        for (int i = 1; token.size() > i; ++i) {
            auto c = token[i];
            if (!isdigit(c))
                return false;
        }
        return true;
    }
    return false;
}

optional < MalType * > read_atom(Reader &reader) {
    auto token = *reader.peek();

    char firstChar = token[0];

    switch (firstChar) {
        case '"': {
            return read_string(reader);
        }
        case ':': {
            return read_keyword(reader);
        }
        case '&': {
            if (token.size() == 1) {
                reader.next();
                return glob.at("&");
            }
        }
        case '.': {
            if (token.size() == 3 && token == "...") {
                reader.next();
                return glob.at("...");
            }
        }
        default: {
            if (isNilToken(token)) {
                reader.next();
                return glob.at("nil");
            } else if (isBooleanToken(token)) {
                reader.next();
                return token == "true" ? glob.at("true") : glob.at("false");
            } else if (isNumberToken(token)) {
                reader.next();
                // cast token to long from a string and then make a MalInt with it
                long num = 0;
                // returns a from_chars_result struct which gets 
                // destructured into ptr for rest of non-number string
                // and errCode for any errors 
                auto tokenEnd = token.data() + token.size();
                auto [rest, errCode] { from_chars(token.data(), tokenEnd, num) };
                
                // no errors, and entirety of token is a number
                if (errCode == errc() && tokenEnd - rest == 0) {
                    return new MalInt(num);
                } else if (errCode == errc()) {
                    // no errors but not a fully formed number
                    // e.g: 1234abc, 200b
                    auto n_excep = ReaderException();
                    n_excep.errMessage = "number token contains invalid characters!";
                    throw n_excep;
                } else if (errCode == errc::invalid_argument) { 
                    // means isNumberToken() failed
                    auto n_excep = ReaderException();
                    n_excep.errMessage = "invalid number!";
                    throw n_excep;
                } else if (errCode == errc::result_out_of_range) {
                    auto n_excep = ReaderException();
                    n_excep.errMessage = "number is out of range of a 'long'!";
                    throw n_excep;
                } else {
                    auto n_excep = ReaderException();
                    n_excep.errMessage = "unknown number conversion error :(";
                    throw n_excep;
                }
            } else {
                // determine is we have a - sign before our symbol
                if (token.size() > 1 && firstChar == '-') {
                    bool isKeyword = token.find(":") != string::npos;
                    if (!isKeyword) { // its not a keyword so parse it to a symbol
                        return new MalSymbol(reader.next().value());
                    }
                    reader.next();
                    // if it is a negated symbol, return only the negation sign
                    return new MalSymbol("-");
                }
                return new MalSymbol(reader.next().value());
            }
        }
    }
}

optional < MalType * > read_keyword(Reader &reader) {
    // skip over :
    reader.next().value();
    return new MalKeyword(reader.next().value());
}

optional < MalType * > read_string(Reader &reader) {
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
    
    if (token.length() == 2) { // empty string
        // cout << token << endl;
        return new MalString("");
    }

    return new MalString(token.substr(1, token.size() - 2));
}

optional < MalType * > read_quoted_val(Reader &reader) {
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

optional < MalType * > read_dereferenced_val(Reader &reader) {
    reader.next();
    auto deref_list = new MalList();
    auto sym = new MalSymbol("deref");
    deref_list->append(sym);
    deref_list->append(read_form(reader).value());
    return deref_list;
}

optional < MalType * > read_metadata_w_object(Reader &reader) {
    reader.next();
    auto meta_list = new MalList();
    auto sym = new MalSymbol("with-meta");
    meta_list->append(sym);
    auto metadata_hmap = read_hashmap(reader).value();
    auto obj = read_form(reader).value();
    meta_list->append(obj); // read metadata
    meta_list->append(metadata_hmap); // read object
    return meta_list;
}