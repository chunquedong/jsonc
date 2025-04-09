//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "jencoder.hpp"
#include <stdlib.h>

using namespace jsonc;

void JEncoder::fillEmpty(int size) {
    buffer.resize(buffer.size()+size);
}

void JEncoder::writeData(const char *data, int size, int at) {
    if (at == -1) {
        for (int i = 0; i < size; ++i) {
            buffer.push_back(data[i]);
        }
    }
    else {
        for (int i = 0; i < size; ++i) {
            buffer[at+i] = data[i];
        }
    }
}

void JEncoder::makeStrPool(Value *v) {
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
                Value* val = *p;
                makeStrPool(&key);
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

void JEncoder::writeValue(Value *v) {
    switch (v->type()) {
    case Type::String: {
        Value dup;
        auto str = v->as_str();
        dup._type = Type::String;
        dup._flag = 1;
        dup.value.imuInfo.selfOffset = buffer.size();
        dup.value.imuInfo.sizeOrPos = stringTable[str];
        writeData((char*)(&dup), sizeof(Value));
        break;
    }
    case Type::Object: {
        Value dup;
        dup._type = Type::Object;
        dup._flag = 1;
        dup.value.imuInfo.selfOffset = buffer.size();
        int size = v->size();
        dup.value.imuInfo.sizeOrPos = size;
        writeData((char*)(&dup), sizeof(Value));

        int pos = buffer.size();
        fillEmpty(sizeof(int32_t) * size * 2);
        for (auto p = v->begin(); p != v->end(); ++p) {
            Value key(Type::String);
            key.value.str = (char*)p.get_name();
            Value* val = *p;

            std::string name = key.value.str;
            int32_t key_pos = stringTable[name];
            writeData((char*)(&key_pos), 4, pos);
            pos += 4;

            int32_t val_pos = buffer.size();
            writeData((char*)(&val_pos), 4, pos);
            pos += 4;

            writeValue(val);
        }
        break;
    }
    case Type::Array: {
        Value dup;
        dup._type = Type::Array;
        dup._flag = 1;
        dup.value.imuInfo.selfOffset = buffer.size();
        int size = v->size();
        dup.value.imuInfo.sizeOrPos = size;
        writeData((char*)(&dup), sizeof(Value));

        int pos = buffer.size();
        fillEmpty(sizeof(int32_t) * size);
        for (auto p = v->begin(); p != v->end(); ++p) {
            int32_t val_pos = buffer.size();
            writeData((char*)(&val_pos), 4, pos);
            pos += 4;

            writeValue(*p);
        }
        break;
    }
    default:
        Value dup = *v;
        writeData((char*)(&dup), sizeof(Value));
        break;
    }
}

std::vector<char> &JEncoder::encode(Value *v) {
    makeStrPool(v);

    writeData("JCXX", 4);
    int32_t version = 3;
    writeData((char*)(&version), 4);

    int header = 8;
    int32_t start = 0;
    writeData((char*)(&start), 4);

    int32_t n = (int32_t)stringPool.size();
    writeData((char*)(&n), 4);
    
    for (int i = 0; i < n; ++i) {
        const std::string &str = stringPool[i];
        int32_t n = str.size();
        //rewrite to memory position
        stringTable[str] = buffer.size();
        
        writeData(str.c_str(), n+1);
    }

    int32_t jump = buffer.size();
    writeData((char*)(&jump), 4, header);
    
    writeValue(v);

    return buffer;
}
