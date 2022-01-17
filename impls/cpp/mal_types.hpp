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

enum Type {
    List, Vector, HashMap, Symbol,
    Keyword, String, Nil, Boolean, Int,
    Func, Seq
};

class MalType {
public:
    virtual Type type() = 0;
    virtual string inspect() = 0;
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
    string contents(bool readable=true) {
        string out = "";
        for (int i = 0; stored.size() > i; ++i) {
            auto item = stored[i];
            if (item->type() == List || item->type() == Vector) {
                if (!readable) {
                    out += item->as_sequence()->contents(readable) + " ";
                    continue;
                }
            }                
            out += item->inspect();

            if (i + 1 != stored.size())
                out += " ";
        }
        return out;
    }

protected:
    vector < MalType* > stored;
};

class MalList : public MalSequence {
public:
    MalList() { }

    Type type() {
        return List;
    }

    // add new item to list
    void append(MalType* item) {
        stored.push_back(item);
    }

    string inspect() {
        string out = "(";
        out += contents();
        out += ')';

        return out;
    }

    vector < MalType* > items() {
        return stored;
    }
};

class MalVector : public MalSequence {
public:
    MalVector() { }

    Type type() {
        return Vector;
    }

    // add new item to list
    void append(MalType* item) {
        stored.push_back(item);
    }

    string inspect() {
        string out = "[";
        out += contents();
        out += ']';

        return out;
    }

    vector < MalType* > items() {
        return stored;
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

    string inspect() {
        string out = "{";
        for (auto item : hmap) {
            out += item.first->inspect() + " ";
            out += item.second->inspect() + " ";
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

    string inspect() {
        return str();
    }

private:
    string s_str;
};

class MalKeyword : public MalType {
public:
    MalKeyword(string_view str): k_str {str} { }

    Type type() {
        return Keyword;
    }

    string inspect() {
        return ":" + k_str;
    }

private:
    string k_str;
};

class MalString : public MalType {
public:
    MalString(string_view str): s_str {str} { }

    Type type() {
        return String;
    }   

    string content() {
        return s_str.substr(1, s_str.size()-2);
    }

    string inspect() {
        return s_str;
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

    string inspect() {
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

    string inspect() {
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

    string inspect() {
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

    string inspect() {
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