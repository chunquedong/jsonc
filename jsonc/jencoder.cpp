//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "jencoder.hpp"
#include <stdlib.h>

using namespace jc;

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

void JEncoder::makeStrPool(Value &v) {
    switch (v.type()) {
        case Type::String: {
            auto str = v.asStr();
            auto it = stringTable.find(str);
            if (it == stringTable.end()) {
                size_t i = stringTable.size();
                stringTable[str] = i;
                stringPool.push_back(str);
            }
            break;
        }
        case Type::Object: {
            for (int i=0; i<v.size(); ++i) {
                std::string name;
                Value *val = v.getProp(i, name);
                Value key;
                key = name;
                makeStrPool(key);
                
                //set value
                makeStrPool(*val);
            }
            break;
        }
        case Type::Array: {
            for (int i=0; i<v.size(); ++i) {
                Value *sv = v[i];
                makeStrPool(*sv);
            }
            break;
        }
        default:
            break;
    }
}

void JEncoder::writeValue(Value &v) {
    switch (v.type()) {
    case Type::String: {
        Value dup = v;
        auto str = v.asStr();
        dup._type = Type::ImuString;
        dup.value.imuInfo.bufferOffset = buffer.size();
        dup.value.imuInfo.sizeOrPos = stringTable[str];
        writeData((char*)(&dup), sizeof(Value));
        break;
    }
    case Type::Object: {
        Value dup = v;
        dup._type = Type::ImuObject;
        dup.value.imuInfo.bufferOffset = buffer.size();
        dup.value.imuInfo.sizeOrPos = v.size();
        writeData((char*)(&dup), sizeof(Value));

        int pos = buffer.size();
        int size = v.size();
        fillEmpty(sizeof(int32_t) * size * 2);
        for (int i = 0; i < size; ++i) {
            std::string name;
            Value *val = v.getProp(i, name);

            int32_t key_pos = stringTable[name];
            writeData((char*)(&key_pos), 4, pos);
            pos += 4;

            int32_t val_pos = buffer.size();
            writeData((char*)(&val_pos), 4, pos);
            pos += 4;

            writeValue(*val);
        }
        break;
    }
    case Type::Array: {
        Value dup = v;
        dup._type = Type::ImuArray;
        dup.value.imuInfo.bufferOffset = buffer.size();
        dup.value.imuInfo.sizeOrPos = v.size();
        writeData((char*)(&dup), sizeof(Value));

        int pos = buffer.size();
        int size = v.size();
        fillEmpty(sizeof(int32_t)*size);
        for (int i = 0; i < size; ++i) {
            int32_t val_pos = buffer.size();
            writeData((char*)(&val_pos), 4, pos);
            pos += 4;

            Value *sv = v[i];
            writeValue(*sv);
        }
        break;
    }
    default:
        writeData((char*)(&v), sizeof(Value));
        break;
    }
}

std::vector<char> &JEncoder::encode(Value &v) {
    makeStrPool(v);

    writeData("JCXX", 4);
    int32_t version = 2;
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
