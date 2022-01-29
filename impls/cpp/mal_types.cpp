#include "mal_types.hpp"

MalString* LIST = new MalString("List");
MalString* VEC = new MalString("Vector");
MalString* PAIR = new MalString("Pair");
MalString* HASHMAP = new MalString("HashMap");
MalString* SYM = new MalString("Symbol");
MalString* SPREADR = new MalString("Spreader");
MalString* KEYWORD = new MalString("Keyword");
MalString* STR = new MalString("String");
MalString* NIL_V = new MalString("Nil");
MalString* BOOL = new MalString("Boolean");
MalString* NUM = new MalString("Int");
MalString* FN = new MalString("Func");
MalString* TCOFN = new MalString("TCOFunc");
MalString* ATOM = new MalString("Atom");

MalList* MalType::as_list() {
    assert(type() == List);
    return static_cast<MalList *>(this);
}

MalVector* MalType::as_vector() {
    assert(type() == Vector);
    return static_cast<MalVector *>(this);
}

MalPair* MalType::as_pair() {
    assert(type() == Pair);
    return static_cast<MalPair *>(this);
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
    if (type() == List || type() == Vector || type() == Pair)
        return static_cast<MalSequence *>(this);
    assert(false);
}

MalSpreader* MalType::as_spreader() {
    assert(type() == Spreader);
    return static_cast<MalSpreader *>(this);
}

MalTCOptFunc* MalType::as_tcoptfunc() {
    assert(type() == TCOptFunc);
    return static_cast<MalTCOptFunc *>(this);
}

MalAtom* MalType::as_atom() {
    assert(type() == Atom);
    return static_cast<MalAtom *>(this);
}
