//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "jparser.hpp"
#include <stdlib.h>
#include <stdexcept>

using namespace jc;

static bool isDigit(char c) {
    if (c == '-' || c == 'e' || c == 'E' || c == '.' || (c <= '9' && c >= '0') ) {
        return true;
    }
    return false;
}

std::exception JsonParser::err(const std::string &msg) {
    int s = pos - 10;
    if (s < 0) s = 0;
    int e = pos + 10;
    if (e > input.size()) e = (int)input.size();
    
    std::string context = input.substr(s, pos-s) + "^" + input.substr(pos, e-pos);
    printf("parse error at: %s\n", context.c_str());
    
    return std::logic_error(msg);
}

Value JsonParser::parseObj() {
    Value obj(Type::Object);
    skipWhitespace();
    expect((char)JsonToken::objectStart);
    while (true)
    {
        skipWhitespace();
        if (maybe((char)JsonToken::objectEnd)) return obj;
        parsePair(obj);
        if (!maybe((char)JsonToken::comma)) break;
    }
    
    expect((char)JsonToken::objectEnd);
    return obj;
}

void JsonParser::parsePair(Value obj) {
    skipWhitespace();
    Value value(Type::String);
    std::string *str = value.asStr();
    parseStr(*str);
    
    skipWhitespace();
    
    expect((char)JsonToken::colon);
    skipWhitespace();
    
    Value val = parseVal();
    skipWhitespace();
    
    obj.set(value, val);
}

Value JsonParser::parseVal() {
    if (cur == (char)JsonToken::quote) {
        Value Value(Type::String);
        std::string *str = Value.asStr();
        parseStr(*str);
        return Value;
    }
    else if (isDigit(cur)) return parseNum();
    else if (cur == (char)JsonToken::objectStart) return parseObj();
    else if (cur == (char)JsonToken::arrayStart) {
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
    
    if (cur < 0) throw err("Unexpected end of stream");
    throw err("Unexpected token ");
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
    expect((char)JsonToken::quote);
    while( cur != (char)JsonToken::quote) {
        //if (cur < 0) throw err("Unexpected end of str literal");
        if (cur == '\\') {
            str += escape();
        }
        else {
            str += (cur);
            consume();
        }
    }
    expect((char)JsonToken::quote);
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
    expect((char)JsonToken::arrayStart);
    skipWhitespace();
    if (maybe((char)JsonToken::arrayEnd)) return array;
        
    while (true)
    {
        skipWhitespace();
        Value val = parseVal();
        array.add(val);
        skipWhitespace();
        if (!maybe((char)JsonToken::comma)) break;
    }
    skipWhitespace();
    expect((char)JsonToken::arrayEnd);
    return array;
}
