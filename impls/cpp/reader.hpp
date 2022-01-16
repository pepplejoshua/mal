#pragma once

#include <iostream>
#include <string_view>
#include <string>
#include <exception>
#include <vector>
#include <optional>
#include "mal_types.hpp"

using std::string;
using std::string_view;
using std::exception;
using std::optional;
using std::endl;
using std::vector;
using std::cout;

class ReaderException : exception {
public:
    virtual const char* what() const throw()
    {
        return errMessage.c_str();
    }

    string errMessage;
};

class Tokenizer {
public:
    // we use a reference because we expect input to outlive Tokenizer
    // so we won't hold a dangling reference at any point
    Tokenizer(string &input) : t_input {input} { }

    optional < string_view > nextToken() {
        auto s_view = string_view(t_input);
        // while we still have unseen characters in the input
        while(!isAtEndOfInput()) {
            bool bufferedChar = false;
            char ch = currentCharacter();

            switch (ch) {
                // skip whitespace and commas
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                case ',':
                    break;
                // process special pair ~@ and ~
                case '~': {
                    // if it has a @ next to it, return them both
                    if (peekCharacter() == '@') {
                        // skip over both ~ and @
                        advanceIndexBy(2);
                        return s_view.substr(t_input_index-2, 2);
                    } 
                    // step over ~
                    advanceIndexBy(1);
                    return s_view.substr(t_input_index-1, 1);
                }
                case '[':
                case ']':
                case '{':
                case '}':
                case '(':
                case ')':
                case '\'':
                case '`':
                case '@':
                case '^': {
                    // step over it
                    advanceIndexBy(1);
                    return s_view.substr(t_input_index-1, 1);
                }
                // process an escapable string
                case '"': {
                    // so we can do the full span of the string
                    size_t stringStart = t_input_index; 
                    advanceIndexBy(1);
                    while(!isAtEndOfInput()) {
                        char cToken = currentCharacter();
                        switch (cToken) {
                            // termination of string
                            case '"': {
                                // skip terminating "
                                advanceIndexBy(1);
                                return s_view.substr(stringStart, t_input_index - stringStart);
                            }
                            // possible escape of a character (eg: ")
                            case '\\': {
                                // skip \ and next character
                                advanceIndexBy(1);
                                // external advanceIndexBy will move it by 1 again, so this is fine
                                break;
                            }
                        }
                        advanceIndexBy(1);
                    }
                    // return unterminated string
                    return s_view.substr(stringStart, t_input_index - stringStart);
                }
                // handle comments
                case ';': {
                    size_t commentStart = t_input_index;
                    // step over ;
                    advanceIndexBy(1);
                    // skip over comments till newline
                    while(!isAtEndOfInput() && currentCharacter() != '\n') {
                        advanceIndexBy(1);
                    }
                    return s_view.substr(commentStart, t_input_index - commentStart);
                    break;
                }
                // handles everything else:
                /*
                * symbols -> identifiers or atoms
                * numbers 
                * "true"
                * "false"
                * "nil"
                * 
                * we parse it as an inverse of the other characters we have already seen
                */
                default: {
                    size_t start = t_input_index;
                    bool done = false;
                    while (!isAtEndOfInput() && !done) {
                        char c = currentCharacter();
                        switch (c) {
                            case ' ':
                            case '\t':
                            case '\n':
                            case '\r':
                            case ',':
                            case '~': 
                            case '[':
                            case ']':
                            case '{':
                            case '}':
                            case '(':
                            case ')':
                            case '\'':
                            case '`':
                            case '@':
                            case '^':    
                            case '"':
                            case ';': {
                                done = true; // don't include into this token
                                bufferedChar = true;
                                break; 
                            }
                            default: {
                                // include in this token
                                advanceIndexBy(1);
                            }
                        }
                    }
                    return s_view.substr(start, t_input_index - start);
                }
            }
            // if there is no unprocessed character,
            // unprocessed characters can result from inside the default branch of above switch
            // because we broke from internal default while loop on that character, 
            // we need to make sure we process it in the next external loop
            if (!bufferedChar) {
                // move index forward after processing character
                advanceIndexBy(1);
            } else {
                bufferedChar = false;
            }
        }

        return {};
    }

    void advanceIndexBy(int charCount) {
        if (!isAtEndOfInput())
            t_input_index += charCount;
    }

    char currentCharacter() {
        return t_input.at(t_input_index);
    }

    char peekCharacter() {
        return isAtEndOfInput() ? '\0' : t_input.at(t_input_index+1);
    }

    bool isAtEndOfInput() {
        return t_input_index >= t_input.length();
    }

private:
    string &t_input;
    // default to starting at 0
    size_t t_input_index {0};
};

class Reader {
public:
    Reader(vector < string_view > &tokens)
    : r_tokens { tokens } { }

    // returns current token and increments
    // index
    optional< string_view > next() {
        if (!isAtEndOfTokenVector())
            return r_tokens.at(index++);
        return {};
    }

    // returns current token without incrementing
    // index
    optional< string_view > peek() {
        if (!isAtEndOfTokenVector())
            return r_tokens.at(index);
        return {};
    }

    bool isAtEndOfTokenVector() {
        return index >= r_tokens.size();
    }

private:
    vector < string_view > &r_tokens;
    size_t index { 0 };
};

vector < string_view > tokenize(string &input);

optional < MalType* > read_str(string &input);

optional < MalType* > read_form(Reader &reader);

optional < MalType* > read_list(Reader &reader);

optional < MalType* > read_vector(Reader &reader);

optional < MalType* > read_hashmap(Reader &reader);

optional < MalType* > read_atom(Reader &reader);

optional < MalString* > read_string(Reader &reader);

optional < MalType* > read_metadata_w_object(Reader &reader);

optional < MalType* > read_quoted_val(Reader &reader);

optional < MalType* > read_dereferenced_val(Reader &reader);
