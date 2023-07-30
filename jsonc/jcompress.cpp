//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "jcompress.hpp"
#include <stdlib.h>

using namespace jc;


static void writeSize(std::ostream &out, uint8_t type, int64_t val) {
    uint8_t subType = 0;
    if (val <= 10 && val >= 0) {
        subType = val;
        type = (type << 4) | subType;
        out.write((char*)(&type), 1);
        return;
    }
    else if (val == -1) {
        subType = 11;
        type = (type << 4) | subType;
        out.write((char*)(&type), 1);
        return;
    }
    else if (val <= INT8_MAX && val >= INT8_MIN) {
        subType = jc_int8;
        type = (type << 4) | subType;
        out.write((char*)(&type), 1);
        int8_t i = (int8_t)val;
        out.write((char*)(&i), 1);
        return;
    }
    else if (val <= INT16_MAX && val >= INT16_MIN) {
        subType = jc_int16;
        type = (type << 4) | subType;
        out.write((char*)(&type), 1);
        int16_t i = (int16_t)val;
        out.write((char*)(&i), 2);
        return;
    }
    else if (val <= INT32_MAX && val >= INT32_MIN) {
        subType = jc_int32;
        type = (type << 4) | subType;
        out.write((char*)(&type), 1);
        int32_t i = (int32_t)val;
        out.write((char*)(&i), 4);
        return;
    }
    else if (val <= INT64_MAX && val >= INT64_MIN) {
        subType = jc_int64;
        type = (type << 4) | subType;
        out.write((char*)(&type), 1);
        int64_t i = (int64_t)val;
        out.write((char*)(&i), 8);
        return;
    }
}

void JCWriter::makeStrPool(Value *v) {
    switch (v->type()) {
        case Type::String: {
            auto str = v->as_str();
            auto it = stringTable.find(str);
            if (it == stringTable.end()) {
                size_t i = stringTable.size();
                stringTable[str] = i;
                stringPool.push_back(str);
            }
            break;
        }
        case Type::Object: {
            for (auto p = v->begin(); p != v->end(); ++p) {
                Value key(Type::String);
                key.value.str = (char*)p.get_name();
                Value *val = *p;
                
                makeStrPool(&key);
                
                //set value
                makeStrPool(val);
            }
            break;
        }
        case Type::Array: {
            for (auto p = v->begin(); p != v->end(); ++p) {
                makeStrPool(*p);
            }
            break;
        }
        default:
            break;
    }
}

void JCWriter::write(Value *v, std::ostream &out) {
    out.write("JCXX", 4);
    int32_t version = 1;
    out.write((char*)(&version), 4);

    makeStrPool(v);
    int32_t n = (int32_t)stringPool.size();
    out.write((char*)(&n), 4);
    
    for (int i = 0; i < n; ++i) {
        const std::string &str = stringPool[i];
        uint8_t type = jc_string;
        writeSize(out, type, str.size());
        out.write(str.c_str(), str.size()+1);
    }
    
    writeValue(v, out);
}

void JCWriter::writeValue(Value *v, std::ostream &out) {
    switch (v->type()) {
        case Type::String: {
            auto str = v->as_str();
            auto it = stringTable.find(str);
            if (it == stringTable.end()) {
                printf("ERROR: miss str pool: %s\n", str);
                break;
            }

            uint8_t type = jc_ref;
            writeSize(out, type, it->second);
            //puts("~");
            break;
        }
        case Type::Object: {
            writeSize(out, jc_object, v->size());
            for (auto p = v->begin(); p != v->end(); ++p) {
                Value key(Type::String);
                key.value.str = (char*)p.get_name();
                Value* val = *p;
                writeValue(&key, out);
                //set value
                writeValue(val, out);
            }
            break;
        }
        case Type::Array: {
            writeSize(out, jc_array, v->size());
            for (auto p = v->begin(); p != v->end(); ++p) {
                writeValue(*p, out);
            }
            break;
        }
        case Type::Boolean: {
            bool val = v->as_bool();
            uint8_t subType;
            uint8_t type = jc_primi;
            subType = val ? jc_true : jc_false;
            type = (type << 4) | subType;
            out.write((char*)(&type), 1);
            break;
        }
        case Type::Integer: {
            int64_t i = v->as_int();
            writeSize(out, jc_int, i);
            break;
        }
        case Type::Float: {
            double f = v->as_float();
            uint8_t subType = 0;
            uint8_t type = jc_float;
            if (f == 0) {
                subType = 0;
                type = (type << 4) | subType;
                out.write((char*)(&type), 1);
            } else {
                subType = jc_int64;
                type = (type << 4) | subType;
                out.write((char*)(&type), 1);
                out.write((char*)(&f), sizeof(double));
            }
            break;
        }
        case Type::Null: {
            uint8_t subType;
            uint8_t type = jc_primi;
            subType = jc_null;
            type = (type << 4) | subType;
            out.write((char*)(&type), 1);
            break;
        }
        default:
            break;
    }
}


/////////////////////////////////////////////////////////////////
// Reader
/////////////////////////////////////////////////////////////////

bool JCReader::readData(char* data, int len) {
    if (pos + len > this->size) return false;
    memcpy(data, buffer + pos, len);
    pos += len;
    return true;
}

static int64_t readSize(JCReader *inStream, jc_type &type, uint8_t subType) {
    switch (subType) {
        case 11:
            return -1;
            break;
        case 12: {
            int8_t i = inStream->readByte();
            //inStream.read((char*)(&i), 1);
            return i;
            break;
        }
        case 13:{
            int16_t i = 0;
            inStream->readData((char*)(&i), 2);
            return i;
            break;
        }
        case 14:{
            int32_t i = 0;
            inStream->readData((char*)(&i), 4);
            return i;
            break;
        }
        case 15:{
            int64_t i = 0;
            inStream->readData((char*)(&i), 8);
            return i;
            break;
        }
        default:
            break;
    }
    return subType;
}

Value* JCReader::read() {
    pos = 8;

    if (pos >= size) {
        return NULL;
    }

    int32_t poolSize = 0;
    readData((char*)(&poolSize), 4);

    pool.resize(poolSize);
    for (int32_t i=0; i<poolSize; ++i) {
        pool[i] = readValue();
    }

    return readValue();
}

JsonNode* JCReader::readValue(JsonNode* out) {

    int c = readByte();
    if (c == EOF) return NULL;
    uint8_t t = c;
    //inStream.read((char*)(&t), 1);
    jc_type type = (jc_type)((t >> 4) & 0xff);
    uint8_t subType =  t & 0x0f;
    
    switch (type) {
        case jc_int: {
            JsonNode*val = alloc(Type::Integer);
            int64_t size = readSize(this, type, subType);
            val->value.i = size;
            return val;
            break;
        }
        case jc_float: {
            JsonNode* val = alloc(Type::Float);
            if (subType < 11) {
                val->value.d = (double)subType;
            } else if (subType == jc_int64) {
                double d;
                readData((char*)(&d), sizeof(double));
                val->value.d = d;
            } else if (subType == jc_int32) {
                float d;
                readData((char*)(&d), sizeof(float));
                val->value.d = d;
            } else {
                val->value.d = 0;
            }
            return val;
            break;
        }
        case jc_primi: {
            if (subType == jc_null) {
                return alloc(Type::Null);
            }
            else if (subType == jc_true) {
                JsonNode* val = alloc(Type::Boolean);
                val->value.b = true;
                return val;
            }
            else if (subType == jc_false) {
                JsonNode* val = alloc(Type::Boolean);
                val->value.b = false;
                return val;
            }
            else {
                return alloc(Type::Null);
            }
            break;
        }
        case jc_array: {
            int64_t size = readSize(this, type, subType);
            JsonNode* value = alloc(Type::Array);
            for (int i=0; i<size; ++i) {
                JsonNode*v = readValue();
                value->insert(v);
            }
            value->reverse();
            return value;
            break;
        }
        case jc_string: {
            JsonNode* value = alloc(Type::String);
            int64_t size = readSize(this, type, subType);

            value->value.str = buffer + pos;
            pos += size + 1;

            /*char *str = (char*)malloc(size+1);
            readData(str, size);
            str[size] = 0;
            value.setStr(str, false);*/

            //stringTable.push_back(*s);
            //printf("%s\n", s.c_str());
            return value;
            break;
        }
        case jc_ref: {
            int64_t size = readSize(this, type, subType);
            if (size < pool.size()) {
                JsonNode*v = pool[size];
                JsonNode* val = out;
                if (!val) val = alloc(Type::String);
                val->value.str = v->value.str;
                return val;
            }
            return alloc(Type::Null);
            break;
        }
        case jc_object: {
            int64_t size = readSize(this, type, subType);
            JsonNode* value = alloc(Type::Object);
            for (int i=0; i<size; ++i) {
                JsonNode key;
                readValue(&key);
                JsonNode* v = readValue();
                value->insert_pair(key.value.str, v);
            }
            value->reverse();
            return value;
            break;
        }
        default: {
            printf("ERROR: unknow type: %d", type);
            break;
        }
    }

    return alloc(Type::Null);
}
