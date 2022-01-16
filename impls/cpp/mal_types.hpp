#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <map>

using namespace std;

class MalType {
public:
    virtual string typeID() = 0;
    virtual string inspect() = 0;
};

class MalList : public MalType{
public:
    MalList() { }
    string typeID() {
        return "list";
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

private:
    vector < MalType* > l_items;
};

class MalVector : public MalType{
public:
    MalVector() { }
    string typeID() {
        return "vector";
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

private:
    vector < MalType* > v_items;
};

class MalHashMap : public MalType {
public:
    MalHashMap() {}

    string typeID() {
        return "hashmap";
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

    string typeID() {
        return "symbol";
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

    string typeID() {
        return "keyword";
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

    string typeID() {
        return "string";
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

class MalNil : public MalType {
public:
    MalNil() { }

    string typeID() {
        return "nil";
    }

    string inspect() {
        return "nil";
    }
};

class MalBoolean : public MalType {
public:
    MalBoolean(bool val) : value {val} { }

    string typeID() {
        return "boolean";
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

    string typeID() {
        return "int";
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