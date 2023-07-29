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

/////////////////////////////////////////////////////////////////
// JSON Value
/////////////////////////////////////////////////////////////////

void Value::set_int(const int64_t other) {
    set_type(Type::Integer);
    value.i = other;
}

void Value::set_double(const double other) {
    set_type(Type::Double);
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
    else if (_type == Type::Double) {
        return (int64_t)value.d;
    }
    return 0;
}

double Value::as_double() {
    if (_type == Type::Double) {
        return value.d;
    }
    else if (_type == Type::Integer) {
        return (double)value.i;
    }
    return 0;
}
bool Value::as_bool() {
    if (_type != Type::Boolean) return false;
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
            if (strcmp(p->value.str, name) == 0) {
                return false;
            }
            ++p;
        }
    }
    return NULL;
}

bool JsonNode::set(JsonNode* key, JsonNode* val) {
    assert(_type == Type::Object);
    for (auto p = begin(); p != end(); ++p) {
        if (strcmp(p->value.str, key->value.str) == 0) {
            return false;
        }
        ++p;
    }

    insert_pair(key, val);
    return true;
}

void JsonNode::insert_pair(JsonNode* key, JsonNode* val) {
    assert(_type == Type::Object);
    val->_next = key;
    key->_next = value.child;
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
        if (parent->_type != Type::Object) {
            cur = cur->_next;
        }
        else {
            cur = cur->_next->_next;
        }
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
        if (parent->_type != Type::Object) {
            return cur;
        }
        else {
            return cur->_next;
        }
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
char* JsonIterator::get_name() const {
    if (parent->_type != Type::Object) return NULL;
    if (parent->_flag == 0) {
        return cur->value.str;
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

void Value::to_json(std::string& json, int level) {
    Type ttype = type();
    switch (ttype) {
    case Type::String:
        json += ("\"");
        strEscape(json, as_str());
        json += ("\"");
        break;

    case Type::Integer: {
        char buf[256];
        snprintf(buf, 256, "%lld", value.i);
        json += buf;
        break;
    }
    case Type::Double: {
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
        json += "[\n";
        for (auto p = begin(); p != end(); ++p) {
            if (p != begin()) json += ",\n";
            makesp(json, level + 1);
            p->to_json(json, level + 1);
        }
        json += '\n';
        makesp(json, level);
        json += "]";
        break;
    }
    case Type::Object: {
        json += ("{\n");
        for (auto p = begin(); p != end(); ++p) {
            if (p != begin()) {
                json += ",\n";
            }
            makesp(json, level + 1);
            char* key = p.get_name();
            strEscape(json, key);
            json += ": ";
            p->to_json(json, level + 1);
        }
        json += '\n';
        makesp(json, level);
        json += ("}");
        break;
    }
    default:
        break;
    }
}
}