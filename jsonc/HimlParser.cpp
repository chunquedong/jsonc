//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "HimlParser.hpp"
#include <stdlib.h>
#include <stdexcept>

using namespace jsonc;

JsonNode* HimlParser::parseObj(const char* tagName, bool isRoot) {
    JsonNode* obj = alloc(Type::Object);
    obj->value.child = NULL;

    if (tagName) {
        JsonNode* tag_name = alloc(Type::String);
        tag_name->value.str = (char*)tagName;
        obj->insert_pair("_type", tag_name);
    }

    if (isRoot) {
        skipWhitespace();
    }
    else {
        ++cur;
        skipWhitespace();
        if (*cur == '}') {
            ++cur;
            return obj;
        }
    }

    JsonNode* children = alloc(Type::Array);
    obj->insert_pair("_children", children);

    int count = 0;
    while (true)
    {
        ++count;
        int lines = 0;
        //noname object
        if (*cur == '{') {
            JsonNode* obj = parseObj();
            if (!obj) return obj;
            children->insert(obj);
            lines = skipWhitespace();
        }
        else {
            //read name
            JsonNode key;
            key._type = Type::String;
            parseStr(&key);
            lines = skipWhitespace();

            char c = *cur;
            //read pair
            if (c == '=') {
                ++cur;
                lines += skipWhitespace();
                JsonNode* val = parseVal();
                if (!key.value.str || !val) return NULL;
                lines += skipWhitespace();
                if (*cur == '{') {
                    //tag name
                    JsonNode* nobj = parseObj(val->value.str);
                    obj->insert_pair(key.value.str, nobj);
                }
                else {
                    obj->insert_pair(key.value.str, val);
                }
                lines += skipWhitespace();
            }
            //read named object
            else if (c == '{') {
                //tag name
                JsonNode* obj = parseObj(key.value.str);
                children->insert(obj);
                lines = skipWhitespace();
            }
            else {
                JsonNode* child = alloc(Type::String);
                child->value.str = key.value.str;
                children->insert(child);
            }
        }

        char c = *cur;
        if (c == ',') {
            ++cur;
            skipWhitespace();
            continue;
        }
        else if (!isRoot && c == '}') {
            ++cur;
            break;
        }
        else if (c == 0) {
            if (!isRoot) {
                snprintf(error, 128, "Unexpected end of stream");
                return obj;
            }
            break;
        }
        else {
            if (lines) {
                continue;
            }
            snprintf(error, 128, "Unexpected token '%c' at %s", *cur, cur);
            return obj;
        }
    }
    children->reverse();
    obj->reverse();
    return obj;
}

JsonNode* HimlParser::parseVal() {
begin:
    char c = *cur;
    switch (c) {
    case '"':
        return parseStr();
    case '{':
        return parseObj();
    case '\t':
    case '\n':
    case '\r':
    case ' ': {
        ++cur;
        goto begin;
    }
    case 0:
        snprintf(error, 128, "Unexpected end of stream");
        return NULL;
    default:
        return parseStr();
    }
}

void HimlParser::parseEscape(char* str)
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
    }
}

JsonNode* HimlParser::parseStr(JsonNode* val) {
    if (delayStrTerminate && delayStrTerminate < cur) {
        *delayStrTerminate = 0;
        delayStrTerminate = NULL;
    }
    bool hasQuote = false;
    if (*cur == '"') {
        ++cur;
        hasQuote = true;
    }

    if (!val) {
        val = alloc(Type::String);
    }
    val->value.str = cur;
    char *str = cur;
    while(true) {
        switch (*cur)
        {
        case 0:
            snprintf(error, 128, "Unexpected end of stream");
            return NULL;
        case '\\':
            ++cur;
            parseEscape(str);
            break;
        case '"':
            if (hasQuote) {
                *str = 0;
                ++cur;
                return val;
            }
            else {
                ++cur;
                ++str;
                break;
            }
        case ' ':
        case '\r':
        case '\n':
        case '\t':
        case '{':
        case '}':
        case '=':
        case ',':
            if (!hasQuote) {
                //trim str
                while (*str == ' ' && str != val->value.str) {
                    --str;
                }
                if (str < cur) {
                    ++str;
                    *str = 0;
                    ++cur;
                }
                else {
                    delayStrTerminate = cur;
                }
                return val;
            }
            else {
                ++cur;
                ++str;
                break;
            }
        default:
            ++cur;
            ++str;
            break;
        }
    }
    return val;
}
