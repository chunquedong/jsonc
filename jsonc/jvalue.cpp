//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "jValue.hpp"
#include <stdlib.h>
#include <stdexcept>
#include <assert.h>

namespace jc {

/////////////////////////////////////////////////////////////////
// Allocator
/////////////////////////////////////////////////////////////////

#define JSON_BLOCK_SIZE 4096

void *JsonAllocator::allocate(size_t size) {
    size = (size + 7) & ~7;

    if (head && head->used + size <= JSON_BLOCK_SIZE) {
        char *p = (char *)head + head->used;
        head->used += size;
        return p;
    }

    size_t allocSize = sizeof(Block) + size;
    Block *zone = (Block *)calloc(1, allocSize <= JSON_BLOCK_SIZE ? JSON_BLOCK_SIZE : allocSize);
    if (zone == nullptr)
        return nullptr;
    zone->used = allocSize;
    if (allocSize <= JSON_BLOCK_SIZE || head == nullptr) {
        zone->next = head;
        head = zone;
    }
    else {
        zone->next = head->next;
        head->next = zone;
    }
    return (char *)zone + sizeof(Block);
}

JsonAllocator::~JsonAllocator() {
    while (head) {
        Block *next = head->next;
        free(head);
        head = next;
    }
}

JsonNode* link_reverse(JsonNode* head) {
    JsonNode* new_head = NULL;
    JsonNode* temp = NULL;
    if (head == NULL || head->_next == NULL) {
        return head;
    }
    while (head != NULL)
    {
        temp = head;
        head = head->_next;
        temp->_next = new_head;
        new_head = temp;
    }
    return new_head;
}

void JsonAllocator::swap(JsonAllocator& other) {
    Block* head = this->head;
    this->head = other.head;
    other.head = head;
}

/////////////////////////////////////////////////////////////////
// JSON Value
/////////////////////////////////////////////////////////////////

void Value::set_int(const int64_t other) {
    set_type(Type::Integer);
    value.i = other;
}

void Value::set_float(const double other) {
    set_type(Type::Float);
    value.d = other;
}

void Value::set_bool(const bool other) {
    set_type(Type::Boolean);
    value.b = other;
}

void Value::set_str(char* str) {
    set_type(Type::String);
    value.str = str;
}

const char* Value::as_str(const char* defVal) {
    if (_type != Type::String) return defVal;
    if (_flag == 1) {
        char* buffer = ((char*)this) - value.imuInfo.selfOffset;
        return (buffer + value.imuInfo.sizeOrPos);
    }
    return value.str;
}

int64_t Value::as_int() {
    if (_type == Type::Integer) {
        return value.i;
    }
    else if (_type == Type::Float) {
        return (int64_t)value.d;
    }
    else if (_type == Type::String) {
        return atoll(as_str());
    }
    return 0;
}

double Value::as_float() {
    if (_type == Type::Float) {
        return value.d;
    }
    else if (_type == Type::Integer) {
        return (double)value.i;
    }
    else if (_type == Type::String) {
        return atof(as_str());
    }
    return 0;
}
bool Value::as_bool() {
    if (_type != Type::Boolean) return false;
    else if (_type == Type::String) {
        return strcmp("true", as_str()) == 0;
    }
    return value.b;
}

size_t Value::size() {
    if (_type == Type::Array || _type == Type::Object) {
        if (_flag == 1) {
            return value.imuInfo.sizeOrPos;
        }
        size_t count = 0;
        for (auto p = begin(); p != end(); ++p) {
            ++count;
        }
        return count;
    }
    return 0;
}

Value* Value::get(const char* name) {
    if (_type == Type::Object) {
        for (auto p = begin(); p != end(); ++p) {
            if (strcmp(p.get_name(), name) == 0) {
                return *p;
            }
        }
    }
    return NULL;
}

bool JsonNode::set(const char* key, JsonNode* val) {
    assert(_type == Type::Object);
    for (auto p = begin(); p != end(); ++p) {
        if (strcmp(p.get_name(), key) == 0) {
            return false;
        }
        ++p;
    }

    insert_pair(key, val);
    return true;
}

void JsonNode::insert_pair(const char* key, JsonNode* val) {
    assert(_type == Type::Object);
    val->name = key;
    val->_next = value.child;
    value.child = val;
}

void JsonNode::insert(JsonNode* val) {
    assert(_type == Type::Array);
    val->_next = value.child;
    value.child = val;
}

void JsonNode::append(JsonNode* val) {
    assert(_type == Type::Array);
    JsonNode* p = value.child;
    if (!p) {
        value.child = val;
        return;
    }
    while (true) {
        if (!p->_next) {
            p->_next = val;
            break;
        }
        p = p->_next;
    }
}

void JsonNode::reverse() {
    value.child = link_reverse(value.child);
}

JsonIterator Value::begin() {
    assert(_type == Type::Array || _type == Type::Object);
    JsonIterator it;
    it.parent = this;
    if (_flag == 0) {
        it.cur = value.child;
    }
    else {
        it.index = 0;
    }
    return it;
}

JsonIterator Value::end() {
    assert(_type == Type::Array || _type == Type::Object);
    JsonIterator it;
    it.parent = this;
    if (_flag == 0) {
        it.cur = NULL;
    }
    else {
        it.index = value.imuInfo.sizeOrPos;
    }
    return it;
}

/////////////////////////////////////////////////////////////////
// JsonIterator
/////////////////////////////////////////////////////////////////

void JsonIterator::operator++() {
    if (parent->_flag == 0) {
        cur = cur->_next;
    }
    else {
        ++index;
    }
}
bool JsonIterator::operator!=(const JsonIterator& x) const {
    if (parent->_flag == 0) {
        return cur != x.cur;
    }
    else {
        return index != x.index;
    }
}
Value* JsonIterator::operator*() const {
    if (parent->_flag == 0) {
        return cur;
    }
    else {
        char* buffer = ((char*)parent) - parent->value.imuInfo.selfOffset;
        uint32_t* index = (uint32_t*)(parent + 1);
        if (parent->_type != Type::Object) {
            const char* pos = buffer + index[this->index];
            return ((Value*)pos);
        }
        else {
            const char* pos = buffer + index[this->index * 2 + 1];
            return ((Value*)pos);
        }
    }
}
Value* JsonIterator::operator->() const {
    return operator*();
}
const char* JsonIterator::get_name() const {
    if (parent->_type != Type::Object) return NULL;
    if (parent->_flag == 0) {
        return cur->name;
    }
    else {
        char* buffer = ((char*)parent) - parent->value.imuInfo.selfOffset;
        uint32_t* index = (uint32_t*)(parent + 1);
        char* akey = buffer + index[this->index * 2];
        return akey;
    }
}

/////////////////////////////////////////////////////////////////
// Value to JSON string
/////////////////////////////////////////////////////////////////

static void makesp(std::string& s, int d) {
    if (d < 0) return;
    while (d--) s += "  ";
}

static void strEscape(std::string& json, const char* raw) {
    const char* c = raw;
    if (!c) return;
    while (*c) {
        switch (*c) {
        case '\b': json += ('\\'); json += ('b'); break;
        case '\f': json += ('\\'); json += ('f'); break;
        case '\n': json += ('\\'); json += ('n'); break;
        case '\r': json += ('\\'); json += ('r'); break;
        case '\t': json += ('\\'); json += ('t'); break;
        case '\\': json += ('\\'); json += ('\\'); break;
        case '"':  json += ('\\'); json += ('"'); break;
        default: json += *c;
        }
        ++c;
    }
}

static bool strNeedEscape(const char* raw) {
    const char* c = raw;
    if (!c) return false;
    while (true) {
        switch (*c)
        {
        case '"':
        case '//':
        case ' ':
        case '\r':
        case '\n':
        case '\t':
        case '{':
        case '}':
        case '=':
        case ',':
            return true;
        case 0:
            return false;
        default:
            break;
        }
        ++c;
    }
    return false;
}

void Value::to_json(std::string& json, bool isHiml, int level) {
    Type ttype = type();
    switch (ttype) {
    case Type::String: {
        const char* str = as_str();
        if (!isHiml || strNeedEscape(str)) {
            json += ("\"");
            strEscape(json, str);
            json += ("\"");
        }
        else {
            json += str;
        }
        break;
    }
    case Type::Integer: {
        char buf[256];
        snprintf(buf, 256, "%lld", value.i);
        json += buf;
        break;
    }
    case Type::Float: {
        char buf[256];
        snprintf(buf, 256, "%g", value.d);
        json += buf;
        break;
    }
    case Type::Boolean:
        json += value.b ? "true" : "false";
        break;
    case Type::Null:
        json += "null";
        break;
    case Type::Array: {
        json += isHiml ? "{\n" : "[\n";
        for (auto p = begin(); p != end(); ++p) {
            if (p != begin()) json += ",\n";
            makesp(json, level + 1);
            p->to_json(json, isHiml, level + 1);
        }
        json += '\n';
        makesp(json, level);
        json += isHiml ? "}" : "]";
        break;
    }
    case Type::Object: {
        if (isHiml) {
            Value* typeName = objectType();
            if (typeName) {
                typeName->to_json(json, isHiml, level);
                json += " ";
            }
        }
        json += ("{\n");
        bool isFirst = true;
        for (auto p = begin(); p != end(); ++p) {
            const char* key = p.get_name();
            if (isHiml) {
                if (strcmp(key, "_children") == 0 || strcmp(key, "_type") == 0) {
                    continue;
                }
            }

            if (!isFirst) {
                json += isHiml ? "\n": ",\n";
            }
            makesp(json, level + 1);

            if (isHiml) {
                strEscape(json, key);
                json += "=";
            }
            else {
                json += ("\"");
                strEscape(json, key);
                json += ("\"");
                json += ": ";
            }

            p->to_json(json, isHiml, level + 1);
            isFirst = false;
        }
        json += '\n';

        if (isHiml) {
            Value* c = children();
            if (c) {
                for (auto p = c->begin(); p != c->end(); ++p) {
                    if (p != c->begin()) json += "\n";
                    makesp(json, level + 1);
                    p->to_json(json, isHiml, level + 1);
                }
                json += '\n';
            }
        }
        makesp(json, level);
        json += ("}");
        break;
    }
    default:
        break;
    }
}
}