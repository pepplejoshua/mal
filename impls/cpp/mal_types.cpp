#include "mal_types.hpp"

 MalList* MalType::as_list() {
    assert(type() == List);
    return static_cast<MalList *>(this);
}

 MalVector* MalType::as_vector() {
    assert(type() == Vector);
    return static_cast<MalVector *>(this);
}

 MalHashMap* MalType::as_hashmap() {
    assert(type() == HashMap);
    return static_cast<MalHashMap *>(this);
}

 MalSymbol* MalType::as_symbol() {
    assert(type() == Symbol);
    return static_cast<MalSymbol *>(this);
}

 MalKeyword* MalType::as_keyword() {
    assert(type() == Keyword);
    return static_cast<MalKeyword *>(this);
}

 MalString* MalType::as_string() {
    assert(type() == String);
    return static_cast<MalString *>(this);
}

 MalNil* MalType::as_nil() {
    assert(type() == Nil);
    return static_cast<MalNil *>(this);
}

 MalBoolean* MalType::as_boolean() {
    assert(type() == Boolean);
    return static_cast<MalBoolean *>(this);
}

 MalInt* MalType::as_int() {
    assert(type() == Int);
    return static_cast<MalInt *>(this);
}

 MalFunc* MalType::as_func() {
    assert(type() == Func);
    return static_cast<MalFunc *>(this);
}

 MalSequence* MalType::as_sequence() {
    assert(type() == List || type() == Vector);
    return static_cast<MalSequence *>(this);
}

 MalSpreader* MalType::as_spreader() {
    assert(type() == Spreader);
    return static_cast<MalSpreader *>(this);
}
