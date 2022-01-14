#pragma once

#include <string>
#include <vector>
#include <string_view>

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