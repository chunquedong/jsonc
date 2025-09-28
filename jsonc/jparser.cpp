//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "jparser.hpp"
#include <stdlib.h>
#include <stdexcept>

using namespace jsonc;

JsonNode* JsonParser::parseObj() {
    JsonNode* obj = alloc(Type::Object);
    obj->value.child = NULL;
    ++cur;

    skipWhitespace();
    if (*cur == '}') {
        ++cur;
        return obj;
    }
    while (true)
    {
        if (*cur != '"') {
            snprintf(error, 128, "object key must be string, got '%c' at %s", *cur, cur);
            return NULL;
        }
        JsonNode key;
        key._type = Type::String;
        parseStr(&key);
        skipWhitespace();
        expect(':');
        JsonNode*val = parseVal();
        if (!key.value.str || !val) return NULL;
        obj->insert_pair(key.value.str, val);

        skipWhitespace();
        if (*cur == ',') {
            ++cur;
            skipWhitespace();
            continue;
        }
        else {
            break;
        }
    }
    expect('}');
    obj->reverse();
    return obj;
}

JsonNode* JsonParser::parseVal() {
begin:
    switch (*cur) {
    case '"':
        return parseStr();
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '-':
        return parseNum();
    case '{':
        return parseObj();
    case '[':
        return parseArray();
    case 't': {
        //"true"
        if (cur[1] == 'r' && cur[2] == 'u' && cur[3] == 'e') {
            cur += 4;
            JsonNode*val = alloc(Type::Boolean);
            val->value.b = true;
            return val;
        }
        else {
            goto error;
        }
    }
    case 'f': {
        //"false"
        if (cur[1] == 'a' && cur[2] == 'l' && cur[3] == 's' && cur[4] == 'e') {
            cur += 5;
            JsonNode*val = alloc(Type::Boolean);
            val->value.b = false;
            return val;
        }
        else {
            goto error;
        }
    }
    case 'n': {
        //"null"
        if (cur[1] == 'u' && cur[2] == 'l' && cur[3] == 'l') {
            cur += 4;
            JsonNode*val = alloc(Type::Null);
            return val;
        }
        else {
            goto error;
        }
    }
    case '\t':
    case '\n':
    case '\r':
    case ' ': {
        ++cur;
        goto begin;
    }
    default:
        break;
    }
error:
    if (*cur == 0) snprintf(error, 128, "Unexpected end of stream");
    else {
        snprintf(error, 128, "Unexpected token '%c' at %s", *cur, cur);
    }
    return NULL;
}

JsonNode* JsonParser::parseNum() {
    JsonNode*val = alloc(Type::Integer);

    char *start = cur;
    if (*cur == '-') ++cur;
    while (true) {
        char c = *cur;
        if (c == '.' || c == 'E' || c == 'e') {
            val->_type = Type::Float;
            break;
        }
        else if ((c <= '9' && c >= '0')) {
            ++cur;
            continue;
        }
        else {
            break;
        }
    }

    if (val->_type == Type::Float) {
        char *end;
        double v = strtod(start, &end);
        val->value.d = v;
        cur = end;
    }
    else {
        int64_t v = atoll(start);
        val->value.i = v;
    }
    return val;
}

static inline bool isxdigit(char c) {
    return (c >= '0' && c <= '9') || ((c & ~' ') >= 'A' && (c & ~' ') <= 'F');
}

static inline int char2int(char c) {
    if (c <= '9')
        return c - '0';
    return (c & ~' ') - 'A' + 10;
}

void JsonParser::parseEscape(char* str)
{
    // check basics
    switch (*cur) {
    case 'b':   *str = '\b'; ++cur; break;
    case 'f':   *str = '\f'; ++cur; break;
    case 'n':   *str = '\n'; ++cur; break;
    case 'r':   *str = '\r'; ++cur; break;
    case 't':   *str = '\t'; ++cur; break;
    case '"':   *str = '"'; ++cur; break;
    case '\\':  *str = '\\'; ++cur; break;
    case '/':   *str = '/'; ++cur; break;
    case 'u':
        int c = 0;
        for (int i = 0; i < 4; ++i) {
            if (isxdigit(*++cur)) {
                c = c * 16 + char2int(*cur);
            }
            else {
                snprintf(error, 128, "unknow hex char %d", *cur);
            }
        }
        if (c < 0x80) {
            *str = c;
        }
        else if (c < 0x800) {
            *str++ = 0xC0 | (c >> 6);
            *str = 0x80 | (c & 0x3F);
        }
        else {
            *str++ = 0xE0 | (c >> 12);
            *str++ = 0x80 | ((c >> 6) & 0x3F);
            *str = 0x80 | (c & 0x3F);
        }
        break;
    }
}

JsonNode* JsonParser::parseStr(JsonNode* val) {
    ++cur;
    if (!val) {
        val = alloc(Type::String);
    }
    val->value.str = cur;
    char *str = cur;
    while(true) {
        switch (*cur)
        {
        case 0:
            snprintf(error, 128, "Unexpected end of stream in paserStr");
            return NULL;
        case '\\':
            ++cur;
            parseEscape(str);
            break;
        case '"':
            *str = 0;
            ++cur;
            return val;
        
        default:
            ++cur;
            ++str;
            break;
        }
    }
    return NULL;
}

JsonNode* JsonParser::parseArray() {
    ++cur;
    JsonNode* array = alloc(Type::Array);
    array->value.child = NULL;

    skipWhitespace();
    if (*cur == ']') {
        ++cur;
        return array;
    }
    while (true)
    {
        JsonNode* val = parseVal();
        if (!val) return NULL;
        array->insert(val);
        skipWhitespace();
        if (*cur != ',') break;
        else {
            ++cur;
        }
    }

    expect(']');
    array->reverse();
    return array;
}
