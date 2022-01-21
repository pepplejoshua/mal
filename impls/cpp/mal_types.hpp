#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <string_view>
#include <functional>
#include <map>

using namespace std;

class MalList;
class MalVector;
class MalHashMap;
class MalSymbol;
class MalKeyword;
class MalString;
class MalNil;
class MalBoolean;
class MalInt;
class MalFunc;
class MalSequence;
class MalSpreader;

enum Type {
    List, Vector, HashMap, Symbol,
    Keyword, String, Nil, Boolean, Int,
    Func, Seq, Spreader
};

class MalType {
public:
    virtual Type type() = 0;
    virtual string inspect(bool readably=true) = 0;
    virtual ~MalType() { }
    MalList* as_list();
    MalVector* as_vector();
    MalHashMap* as_hashmap();
    MalSymbol* as_symbol();
    MalKeyword* as_keyword();
    MalString* as_string();
    MalNil* as_nil();
    MalBoolean* as_boolean();
    MalInt* as_int();
    MalFunc* as_func();
    MalSequence* as_sequence();
    MalSpreader* as_spreader();
};

class TypeException : exception {
public:
    virtual const char* what() const throw()
    {
        return errMessage.c_str();
    }

    string errMessage;
};

class RuntimeException : exception {
public:
    virtual const char* what() const throw()
    {
        return errMessage.c_str();
    }

    string errMessage;
};

class MalSequence : public MalType {
public:
    MalSequence() { }
    virtual ~MalSequence() { }

    string contents(bool readable=true) {
        string out = "";
        for (int i = 0; stored.size() > i; ++i) {
            auto item = stored[i];       
            if ((!readable) && (item->type() == List || item->type() == Vector)) {
                out += item->as_sequence()->contents(readable);
                if (i + 1 != stored.size())
                    out += " ";
                continue;
            } 
            auto nonseq = item->inspect(readable);
            out += nonseq;

            if (i + 1 != stored.size())
                out += " ";
        }
        return out;
    }
    
    // add new item to list
    void append(MalType* item) {
        stored.push_back(item);
    }

    auto items() {
        return stored;
    }

protected:
    vector < MalType* > stored;
};

class MalList : public MalSequence {
public:
    MalList() { }
    MalList(vector < MalType* > items) {
        stored = items;
    }

    Type type() {
        return List;
    }

    string inspect(bool readably=true) {
        string out = "(";
        out += contents(readably);
        out += ')';

        return out;
    }
};

class MalVector : public MalSequence {
public:
    MalVector() { }
    MalVector(vector < MalType* > items) {
        stored = items;
    }

    Type type() {
        return Vector;
    }

    string inspect(bool readably=true) {
        string out = "[";
        out += contents(readably);
        out += ']';

        return out;
    }
};

class MalHashMap : public MalType {
public:
    MalHashMap() {}

    Type type() {
        return HashMap;
    }

    void set(MalType* key, MalType* val) {
        hmap[key] = val;
    }

    MalType* get(MalType* key) {
        auto search = hmap.find(key);

        if (search != hmap.end()) {
            return search->second;
        }

        return nullptr;
    }

    auto items() {
        return hmap;
    }

    string inspect(bool readably=true) {
        string out = "{";
        for (auto item : hmap) {
            out += item.first->inspect(readably) + " ";
            out += item.second->inspect(readably) + " ";
        }
        // overwrite the last append space 
        // only if we have list items
        if (out.length() > 1) {
            out[out.length()-1] = '}';
        }
        else
            out += '}';

        return out;
    }

private:
    map < MalType*, MalType* > hmap;
};


class MalSymbol : public MalType {
public:
    MalSymbol(string_view str): s_str {str} { }

    Type type() {
        return Symbol;
    }

    string str() {
        return s_str;
    }

    string inspect(bool readably=true) {
        return str();
    }

protected:
    string s_str;
};

class MalSpreader : public MalSymbol {
public:
    MalSpreader() : MalSymbol {"..."} { }

    Type type() {
        return Spreader;
    }
};

class MalKeyword : public MalType {
public:
    MalKeyword(string_view str): k_str {str} { }

    Type type() {
        return Keyword;
    }

    string inspect(bool readably=true) {
        return ":" + k_str;
    }

private:
    string k_str;
};

class MalString : public MalType {
public:
    MalString(string_view str): s_str { str } { }

    Type type() {
        return String;
    }   

    string content() {
        if (s_str.size() > 1)
            return s_str.substr(1, s_str.size()-2);
        return "";
    }

    string inspect(bool readably=true) {
        return readably ? escape() : unescape(s_str);
    }

    string unescape(string_view s) {
        if (s.size() == 0)
            return string(s);
        auto ns = s;
        string out = "";
        for (int i = 0; ns.size() > i; ++i) {
            char c = ns[i];
            switch (c) {
                case '\\': {
                    char n = ns[++i];
                    switch (n) {
                        case '\\':
                            out += '\\';
                            break;
                        case 'n':
                            out += '\n';
                            break;
                        case '"':
                            out += '"';
                            break;
                        default:
                            out += n;
                            break;
                    }
                    break;
                }
                default: {
                    out += c;
                }
            }
        }
        return out;
    }

    string escape() {
        if (s_str.size() == 0) {
            return "\"" + s_str + "\"";
        }
        // extract string content without the parenthesis
        auto stringContent = s_str;
        string finalStr = "";
        for (size_t i = 0; i < stringContent.size(); ++i) {
            char c = stringContent[i];
            switch(c) {
                case '\\': {
                    ++i;
                    if (i >= stringContent.size()) {
                        auto r_except = RuntimeException();
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
                case '"': {
                    finalStr += "\\\"";
                    break;
                }
                default:    
                    finalStr += c;
            }
        }
        return "\"" + finalStr + "\"";
    }


private:
    string s_str;
};

class MalNil : public MalType {
public:
    MalNil() { }

    Type type() {
        return Nil;
    }

    string inspect(bool readably=true) {
        return "nil";
    }
};

class MalBoolean : public MalType {
public:
    MalBoolean(bool val) : value {val} { }

    Type type() {
        return Boolean;
    }

    bool val() {
        return value;
    }

    string inspect(bool readably=true) {
        return value ? "true" : "false";
    }

private:
    bool value;
};

class MalInt : public MalType {
public:
    MalInt(long val) : value {val} { }

    Type type() {
        return Int;
    }

    long to_long() {
        return value;
    }

    string inspect(bool readably=true) {
        return to_string(value);
    }

private:
    long value;
};

// this means:
// a FuncPtr points to an (unnamed) function that returns a pointer to a 
// Maltype, and takes 2 arguments:
// first is a pointer to an array of MalTypes which are the MalFunc's
// true arguments and the number of arguments in this arguments array
using Function = function< MalType*(MalType**, size_t) >;

class MalFunc : public MalType {
public:
    MalFunc(Function fn, string fnNameTag) : m_fn{fn}, nameTag{fnNameTag} 
    { }

    Type type() {
        return Func;
    }

    Function callable() {
        return m_fn;
    }

    string inspect(bool readably=true) {
        return "{function " + nameTag + "}";
    }

    string name() {
        return nameTag;
    }

    void setName(string name) {
        nameTag = name;
    }

private:
    Function m_fn { NULL };
    string nameTag;
};