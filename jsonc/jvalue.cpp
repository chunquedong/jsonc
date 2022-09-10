//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "jValue.hpp"
#include <stdlib.h>
#include <stdexcept>

/////////////////////////////////////////////////////

using namespace jc;

JCMap::~JCMap() {
    for (int i=0; i<properties.size(); ++i) {
        properties[i].free();
    }
}

JCArray::~JCArray() {
    for (Value &Value : array) {
        Value.free();
    }
}

Value JCMap::get(const std::string &name) {
    auto it = map.find(name);
    if (it == map.end()) {
        return Value();
    }
    size_t i = it->second;
    return properties[i*2+1];
}

bool JCMap::has(const std::string &name) {
    return map.find(name) != map.end();
}

void JCMap::set(Value key, Value val) {
    size_t i = properties.size()/2;
    properties.push_back(key);
    properties.push_back(val);
    map[*key.asStr()] = i;
}

void JCMap::reserve(size_t size) {
    properties.reserve(size);
    map.reserve(size);
}

void Value::init(size_t reserveSize) {
    switch (_type) {
        case Type::String:
            value.str = new std::string();
            break;
        case Type::Array:
            value.array = new JCArray();
            value.array->reserve(reserveSize);
            break;
        case Type::Object:
            value.map = new JCMap();
            value.map->reserve(reserveSize);
            break;
        default:
            memset(&value, 0, sizeof value);
            break;
    };
}

Value::Value(Type t, size_t reserveSize) : _type(t) {
    init(reserveSize);
}

void Value::free() {
    switch (_type) {
        case Type::String:
            delete value.str;
            break;
        case Type::Array:
            delete value.array;
            break;
        case Type::Object:
            delete value.map;
            break;
        default:
            break;
    };
    memset(&value, 0, sizeof value);
}

/////////////////////////////////////////////////////

Value &Value::operator=(const int64_t other) {
    setType(Type::Integer);
    value.i = other;
    return *this;
}

Value &Value::operator=(const double other) {
    setType(Type::Double);
    value.d = other;
    return *this;
}

Value &Value::operator=(const bool other) {
    setType(Type::Boolean);
    value.b = other;
    return *this;
}

void Value::setType(Type t) {
    if (t == _type) return;
    if (_type != Type::Null) {
        throw std::logic_error("type error");
    }
    _type = t;
    init();
}

std::string *Value::asStr(std::string *defVal) {
    if (_type != Type::String) return defVal;
    return value.str;
}

int64_t Value::asInt() {
    switch (_type) {
        case Type::Double:
            return (int64_t)value.d;
            break;
        case Type::Integer:
            return value.i;
            break;
        default:
            return 0;
            break;
    }
    return 0;
}
double Value::asDouble() {
    switch (_type) {
        case Type::Double:
            return value.d;
            break;
        case Type::Integer:
            return value.i;
            break;
        default:
            return 0;
            break;
    }
    return 0;
}
bool Value::asBool() {
    if (_type != Type::Boolean) return false;
    return value.b;
}

size_t Value::size() {
    if (type() == Type::Array) {
        return value.array->array.size();
    }
    else if (type() == Type::Object) {
        return value.map->size();
    }
    return 0;
}

bool Value::has(const std::string &name) {
    if (type() == Type::Object) {
        return value.map->has(name);
    }
    return false;
}

Value Value::operator[](size_t i) {
    if (type() == Type::Array) {
        if (i < value.array->array.size()) {
            return (value.array->array)[i];
        }
    }
    return Value();
}

Value Value::operator[](const std::string &name) {
    if (type() == Type::Object) {
        return value.map->get(name);
    }
    return Value();
}

bool Value::set(Value key, Value val) {
    if (type() == Type::Null) {
        setType(Type::Object);
    }
    else if (type() != Type::Object) {
        return false;
    }
    value.map->set(key, val);
    return true;
}

bool Value::add(Value val) {
    if (type() == Type::Null) {
        setType(Type::Array);
    }
    else if (type() != Type::Array) {
        return false;
    }
    value.array->array.push_back(val);
    return true;
}

bool Value::getProp(int i, Value &key, Value &val) {
    if (type() == Type::Object) {
        if (i < value.map->size()) {
            key = value.map->key(i);
            val = value.map->val(i);
            return true;
        }
    }
    return false;
}

Value Value::clone() {
    Value n = *this;
    
    switch (_type) {
        case Type::String:
            n.value.str = new std::string(*value.str);
            break;
        case Type::Array:
            n.value.array = new JCArray(*value.array);
            break;
        case Type::Object:
            n.value.map = new JCMap(*value.map);
            break;
        default:
            break;
    };
    
    return n;
}

/////////////////////////////////////////////////////

static void makesp(std::string &s, int d) {
    if (d < 0) return;
    while (d--) s += "  ";
}

static void strEscape(std::string &json, std::string &raw) {
    const char *c = raw.c_str();
    for (size_t i=0,n=raw.size(); i<n; ++i) {
        switch (*c) {
            case '\b': json+=('\\'); json+=('b'); break;
            case '\f': json+=('\\'); json+=('f'); break;
            case '\n': json+=('\\'); json+=('n'); break;
            case '\r': json+=('\\'); json+=('r'); break;
            case '\t': json+=('\\'); json+=('t'); break;
            case '\\': json+=('\\'); json+=('\\'); break;
            case '"':  json+=('\\'); json+=('"'); break;
            default: json += *c;
        }
        ++c;
    }
}

void Value::toJson(std::string &json, int level) {
    Type ttype = type();
    switch (ttype) {
        case Type::String:
            json += ("\"");
            strEscape(json, *asStr());
            json += ("\"");
            break;
            
        case Type::Integer: {
            char buf[256];
            snprintf(buf, 256, "%lld", asInt());
            json += buf;
            break;
        }
        case Type::Double: {
            char buf[256];
            snprintf(buf, 256, "%f", asDouble());
            json += buf;
            break;
        }
        case Type::Boolean:
            json += asBool()?"true":"false";
            break;
        case Type::Null:
            json += "null";
            break;
        case Type::Array: {
            json += "[";
            for (int i=0,n=(int)size(); i<n; i++) {
                if (i) json += ", ";
                (*this)[i].toJson(json, level+1);
            }
            json += "]";
            break;
        }
        case Type::Object: {
            json += ("{\n");
            for (int i=0,n=(int)size(); i<n; i++) {
                Value p = value.map->key(i);
                Value v = value.map->val(i);
                makesp(json, level+1);
                json += "\"" + *p.asStr() + "\": ";
                v.toJson(json, level+1);
                json += (i==n-1?"":",");
                json += ("\n");
            }
            makesp(json, level);
            json += ("}");
            break;
        }
        default:
            break;
    }
}
