#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <string_view>
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
class MalUserFn;

enum Type {
    List, Vector, HashMap, Symbol,
    Keyword, String, Nil, Boolean, Int,
    Func, UserFn
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
    MalUserFn* as_userfn();
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


class MalList : public MalType{
public:
    MalList() { }

    Type type() {
        return List;
    }

    // add new item to list
    void append(MalType* item) {
        l_items.push_back(item);
    }

    string inspect() {
        string out = "(";
        for (auto item : l_items) {
            out += item->inspect() + " ";
        }
        // overwrite the last append space 
        // only if we have list items
        if (out.length() > 1) {
            out[out.length()-1] = ')';
        }
        else
            out += ')';

        return out;
    }

    vector < MalType* > items() {
        return l_items;
    }

private:
    vector < MalType* > l_items;
};

class MalVector : public MalType{
public:
    MalVector() { }

    Type type() {
        return Vector;
    }

    // add new item to list
    void append(MalType* item) {
        v_items.push_back(item);
    }

    string inspect() {
        string out = "[";
        for (auto item : v_items) {
            out += item->inspect() + " ";
        }
        // overwrite the last append space 
        // only if we have list items
        if (out.length() > 1) {
            out[out.length()-1] = ']';
        }
        else
            out += ']';

        return out;
    }

    vector < MalType* > items() {
        return v_items;
    }

private:
    vector < MalType* > v_items;
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
using FuncPtr = MalType* (*)(MalType** args, size_t argc);

class MalFunc : public MalType {
public:
    MalFunc(FuncPtr fn, string fnNameTag) : m_fn{fn}, nameTag{fnNameTag} 
    { }

    Type type() {
        return Func;
    }

    FuncPtr callable() {
        return m_fn;
    }

    string inspect() {
        return "{function " + nameTag + "}";
    }

private:
    FuncPtr m_fn;
    string nameTag;
};

class MalUserFn : public MalType {
public:
    MalUserFn() { }

    Type type() {
        return UserFn;
    }

    string inspect() {
        return "#<function>";
    }

private:

};