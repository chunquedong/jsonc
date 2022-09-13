//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "jValue.hpp"
#include <stdlib.h>
#include <stdexcept>

namespace jc {

/////////////////////////////////////////////////////////////////
// Array and Map
/////////////////////////////////////////////////////////////////

class JCArray {
public:
    std::vector<Value> array;
public:
    ~JCArray();

    void reserve(size_t size) { array.reserve(size); }
};

class JCMap {
    std::vector<std::pair<std::string, Value> > properties;
    std::unordered_map<std::string, size_t> map;
public:
    ~JCMap();

    void reserve(size_t size);

    size_t size() { return properties.size(); }
    const std::string& key(size_t i);
    Value& val(size_t i);
    Value& get(const std::string& name);
    void set(const std::string& key, Value val);
    void _add(const std::string& key, Value val);
    bool has(const std::string& name);
private:
    std::unordered_map<std::string, size_t>& getMap();
};

JCArray::~JCArray() {
    for (Value& Value : array) {
        Value.free();
    }
}

JCMap::~JCMap() {
    for (int i = 0; i < properties.size(); ++i) {
        properties[i].second.free();
    }
}

std::unordered_map<std::string, size_t>& JCMap::getMap() {
    return map;
}

const std::string& JCMap::key(size_t i) {
    return properties.at(i).first;
}

Value& JCMap::val(size_t i) {
    return properties.at(i).second;
}

Value& JCMap::get(const std::string& name) {
    //auto map = getMap();
    auto it = map.find(name);
    if (it == map.end()) {
        return Value();
    }
    size_t i = it->second;
    return properties[i].second;
}

bool JCMap::has(const std::string& name) {
    //auto map = getMap();
    return map.find(name) != map.end();
}

void JCMap::_add(const std::string& key, Value val) {
    size_t i = properties.size();
    properties.push_back(std::pair<std::string, Value>(key, val));
    map[key] = i;
}

void JCMap::set(const std::string& key, Value val) {
    //auto map = getMap();
    if (map.find(key) != map.end()) {
        properties[map[key]].second = val;
        return;
    }
    size_t i = properties.size();
    properties.push_back(std::pair<std::string, Value>(key, val));
    map[key] = i;
}

void JCMap::reserve(size_t size) {
    properties.reserve(size);
    map.reserve(size);
}

/////////////////////////////////////////////////////////////////
// JSON Value
/////////////////////////////////////////////////////////////////

void Value::init(size_t reserveSize) {
    switch (_type) {
    case Type::String:
    case Type::StringRef:
        value.str = NULL;
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
        value.i = 0;
        break;
    };
}

Value::Value(Type t, size_t reserveSize) : _type(t) {
    init(reserveSize);
}

void Value::free() {
    switch (_type) {
    case Type::String:
        ::free(value.str);
        break;
    case Type::StringRef:
        break;
    case Type::Array:
        delete value.array;
        break;
    case Type::Object:
        delete value.map;
        break;
    case Type::ImuArray:
    case Type::ImuObject:
    case Type::ImuString:

        return;
    default:
        break;
    };
    value.i = 0;
}

Value& Value::operator=(const int64_t other) {
    setType(Type::Integer);
    value.i = other;
    return *this;
}

Value& Value::operator=(const double other) {
    setType(Type::Double);
    value.d = other;
    return *this;
}

Value& Value::operator=(const bool other) {
    setType(Type::Boolean);
    value.b = other;
    return *this;
}

Value& Value::operator=(const std::string& other) {
    setType(Type::String);
    value.str = strdup(other.c_str());
    return *this;
}

void Value::setStr(char* str, bool copy) {
    setType(Type::String);
    if (copy) value.str = strdup(str);
    else value.str = str;
}

void Value::setType(Type t) {
    if (t == _type) return;
    if (_type != Type::Null) {
        throw std::logic_error("type error");
    }
    _type = t;
    init();
}

const char* Value::asStr(const char* defVal) {
    if (_type == Type::ImuString) {
        char* buffer = ((char*)this) - value.imuInfo.bufferOffset;
        return (buffer + value.imuInfo.sizeOrPos);
    }
    if (_type != Type::String && _type != Type::StringRef) return defVal;
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
    switch (_type) {
    case Type::Array:
        return value.array->array.size();
        break;
    case Type::Object:
        return value.map->size();
        break;
    case Type::ImuArray:
    case Type::ImuObject:
        return value.imuInfo.sizeOrPos;
    default:
        return 0;
        break;
    }
    return 0;
}

bool Value::has(const std::string& name) {
    if (_type == Type::ImuObject) {
        for (int i = 0; i < value.imuInfo.sizeOrPos; ++i) {
            uint32_t* index = (uint32_t*)(this + 1);
            char* buffer = ((char*)this) - value.imuInfo.bufferOffset;
            const char* key = buffer + index[i * 2];
            if (strcmp(key, name.c_str()) == 0) {
                return true;
            }
        }
    }

    if (_type == Type::Object) {
        return value.map->has(name);
    }

    return false;
}

Value* Value::get(size_t i) {
    if (_type == Type::ImuArray) {
        if (i < value.imuInfo.sizeOrPos) {
            char* buffer = ((char*)this) - value.imuInfo.bufferOffset;
            uint32_t* index = (uint32_t*)(this + 1);
            const char* pos = buffer + index[i];
            return ((Value*)pos);
        }
    }

    if (_type == Type::Array) {
        if (i < value.array->array.size()) {
            return &(value.array->array)[i];
        }
    }
    return NULL;
}

Value* Value::get(const std::string& name) {
    if (_type == Type::ImuObject) {
        for (int i = 0; i < value.imuInfo.sizeOrPos; ++i) {
            char* buffer = ((char*)this) - value.imuInfo.bufferOffset;
            uint32_t* index = (uint32_t*)(this + 1);
            const char* key = buffer + index[i * 2];
            if (strcmp(key, name.c_str()) == 0) {
                const char* pos = buffer + index[i * 2 + 1];
                return ((Value*)pos);
            }
        }
    }
    if (type() == Type::Object) {
        return &value.map->get(name);
    }
    return NULL;
}

bool Value::set(const std::string& key, Value val) {
    if (type() == Type::Null) {
        setType(Type::Object);
    }
    else if (type() != Type::Object) {
        return false;
    }
    value.map->set(key, val);
    return true;
}

void Value::_add(const std::string& key, Value val) {
    value.map->_add(key, val);
}

bool Value::add(Value val) {
    if (_type == Type::Null) {
        setType(Type::Array);
    }
    else if (_type != Type::Array) {
        return false;
    }
    value.array->array.push_back(val);
    return true;
}

Value* Value::getProp(int i, std::string& key) {
    if (_type == Type::ImuObject) {
        if (i < value.imuInfo.sizeOrPos) {
            char* buffer = ((char*)this) - value.imuInfo.bufferOffset;
            uint32_t* index = (uint32_t*)(this + 1);
            const char* akey = buffer + index[i * 2];
            key = akey;
            const char* pos = buffer + index[i * 2 + 1];
            return ((Value*)pos);
        }
    }
    if (_type == Type::Object) {
        if (i < value.map->size()) {
            key = value.map->key(i);
            return &value.map->val(i);
        }
    }
    return NULL;
}


Value Value::clone() {
    Value n = *this;

    switch (_type) {
    case Type::String:
        n.value.str = strdup(value.str);
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

void Value::toJson(std::string& json, int level) {
    Type ttype = type();
    switch (ttype) {
    case Type::String:
    case Type::StringRef:
    case Type::ImuString:
        json += ("\"");
        strEscape(json, asStr());
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
        json += asBool() ? "true" : "false";
        break;
    case Type::Null:
        json += "null";
        break;
    case Type::Array:
    case Type::ImuArray: {
        json += "[";
        for (int i = 0, n = (int)size(); i < n; i++) {
            if (i) json += ", ";
            (*this)[i]->toJson(json, level + 1);
        }
        json += "]";
        break;
    }
    case Type::Object:
    case Type::ImuObject: {
        json += ("{\n");
        for (int i = 0, n = (int)size(); i < n; i++) {
            std::string p;
            Value* v = getProp(i, p);

            makesp(json, level + 1);
            json += std::string("\"") + p + "\": ";
            v->toJson(json, level + 1);
            json += (i == n - 1 ? "" : ",");
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
}