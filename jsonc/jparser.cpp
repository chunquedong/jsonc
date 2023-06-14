//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "jparser.hpp"
#include <stdlib.h>
#include <stdexcept>

using namespace jc;

static bool isDigit(int c) {
    if (c == '-' || c == 'e' || c == 'E' || c == '.' || (c <= '9' && c >= '0') ) {
        return true;
    }
    return false;
}

Value JsonParser::parseObj() {
    Value obj(Type::Object);
    skipWhitespace();
    expect((int)JsonToken::objectStart);
    while (true)
    {
        skipWhitespace();
        if (maybe((int)JsonToken::objectEnd)) return obj;
        parsePair(obj);
        if (!maybe((int)JsonToken::comma)) break;
        if (error.size() > 0) break;
    }
    
    expect((int)JsonToken::objectEnd);
    /*if (error.size() > 0) {
        return Value();
    }*/
    return obj;
}

void JsonParser::parsePair(Value obj) {
    skipWhitespace();
    //Value value(Type::String);
    std::string str;
    parseStr(str);
    //value = str;
    
    skipWhitespace();
    
    expect((int)JsonToken::colon);
    skipWhitespace();
    
    Value val = parseVal();
    skipWhitespace();
    
    obj._add(str, val);
}

Value JsonParser::parseVal() {
    if (cur == (int)JsonToken::quote) {
        Value value(Type::String);
        std::string str;
        parseStr(str);
        value = str;
        return value;
    }
    else if (isDigit(cur)) return parseNum();
    else if (cur == (int)JsonToken::objectStart) return parseObj();
    else if (cur == (int)JsonToken::arrayStart) {
        return parseArray();
    }
    else if (cur == 't') {
        //"true"
        consume(4);
        Value Value(Type::Boolean);
        Value = true;
        return Value;
    }
    else if (cur == 'f') {
        //"false"
        consume(5);
        Value Value(Type::Boolean);
        Value = false;
        return Value;
    }
    else if (cur == 'n') {
        //"null"
        consume(4);
        Value Value(Type::Null);
        return Value;
    }
    
    if (cur < 0) error = ("Unexpected end of stream");
    else {
        char buf[128];
        snprintf(buf, 128, "Unexpected token '%c' at %d", cur, pos);
        error = buf;
    }
}

Value JsonParser::parseNum() {
    std::string str;
    while (true) {
        if (isDigit(cur)) {
            str += cur;
            consume();
        } else {
            break;
        }
    }
    Value Value;
    if (str.find('.') != std::string::npos || str.find('E') != std::string::npos) {
        double d = atof(str.c_str());
        Value = d;
        return Value;
    } else {
        int64_t i = atoll(str.c_str());
        Value = i;
        return Value;
    }
}

void JsonParser::parseStr(std::string &str) {
    expect((int)JsonToken::quote);
    while( cur != (int)JsonToken::quote) {
        if (cur < 0) {
            error = ("Unexpected end of str literal");
            break;
        }
        if (cur == '\\') {
            str += escape();
        }
        else {
            str += (cur);
            consume();
        }
    }
    expect((int)JsonToken::quote);
}

std::string JsonParser::escape()
{
    // consume slash
    expect('\\');
    std::string str;
    
    // check basics
    switch (cur) {
        case 'b':   consume(); str += '\b'; break;
        case 'f':   consume(); str += '\f'; break;
        case 'n':   consume(); str += '\n'; break;
        case 'r':   consume(); str += '\r'; break;
        case 't':   consume(); str += '\t'; break;
        case '"':   consume(); str += '"'; break;
        case '\\':  consume(); str += '\\'; break;
        case '/':   consume(); str += '/'; break;
    }
    return str;
}

Value JsonParser::parseArray() {
    Value array(Type::Array);
    expect((int)JsonToken::arrayStart);
    skipWhitespace();
    if (maybe((int)JsonToken::arrayEnd)) return array;
        
    while (true)
    {
        skipWhitespace();
        Value val = parseVal();
        array.add(val);
        skipWhitespace();
        if (!maybe((int)JsonToken::comma)) break;
        if (error.size() > 0) break;
    }
    skipWhitespace();
    expect((int)JsonToken::arrayEnd);
    return array;
}
