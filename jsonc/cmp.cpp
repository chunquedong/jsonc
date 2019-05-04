//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "cmp.hpp"
#include <stdlib.h>

using namespace jc;

bool JCWriter::packJson(std::string &input, std::ostream &out) {
    JsonParser parser;
    Value v = parser.parse(input);
    write(v, out);
    v.free();
    return true;
}

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
/*
static bool tryWriteAsCoords(std::string &str, std::ostream &out) {
    
    if (str.size() < 50) return false;
    
    char *pos = NULL;
    const char *start = str.c_str();
    
    vector<double> coords;
    char sepa1 = 0;
    char sepa2 = 0;
    int component = 0;
    
    const char *instr = start;
    while (instr) {
        pos = NULL;
        double d = strtod(instr, &pos);
        if (pos == instr || pos == NULL) {
            return false;
        }
        
        if (*pos == 0) {
            break;
        }
        
        coords.push_back(d);
        
        if (sepa1 == 0) {
            sepa1 = *pos;
        }
        else if (sepa2 == 0) {
            sepa2 = *pos;
            component = (int)coords.size();
        }
        else if (sepa2 == *pos) {
            if (coords.size() % component != 0) {
                return false;
            }
        }
        else if (sepa1 != *pos) {
            return false;
        }
        
        ++pos;
        instr = pos;
    }
    
    if (coords.size() == 0 || coords.size() < component*2) return false;
    
    vector<double> min;
    vector<double> max;
    min.resize(component);
    max.resize(component);
    for (int i=0; i<component; ++i) {
        double d = coords[i];
        min[i] = d;
        max[i] = d;
    }
    
    for (size_t i=component,n=coords.size(); i<n; ++i) {
        size_t j = i % component;
        double d = coords[i];
        if (min[j] > d) {
            min[j] = d;
        }
        else if (max[j] < d) {
            max[j] = d;
        }
    }

    vector<double> width;
    width.resize(component);
    for (int i=0; i<component; ++i) {
        double w = max[i] - min[i];
        width[i] = w;
    }
    
    writeType(out, jc_coninate, coords.size());
    out << (uint8_t)component;
    
    for (int i=0; i<component; ++i) {
        double d = min[i];
        out << d;
    }
    for (int i=0; i<component; ++i) {
        double d = max[i];
        out << d;
    }
    
    for (size_t i=0,n=coords.size(); i<n; ++i) {
        size_t j = i % component;
        double s = ((coords[i] - min[j])/width[j]);
        uint8_t p = (uint8_t)(s * UINT8_MAX);
        out << p;
    }
    
    return true;
}
*/
void JCWriter::makeStrPool(Value &v) {
    switch (v.type()) {
        case Type::String: {
            std::string &str = *v.asStr();
            auto it = stringTable.find(str);
            if (it == stringTable.end()) {
                size_t i = stringTable.size();
                stringTable[str] = i;
            }
            break;
        }
        case Type::Object: {
            for (int i=0; i<v.size(); ++i) {
                Value name;
                Value val;
                v.getProp(i, name, val);
                makeStrPool(name);
                
                //set value
                makeStrPool(val);
            }
            break;
        }
        case Type::Array: {
            for (int i=0; i<v.size(); ++i) {
                Value sv = v[i];
                makeStrPool(sv);
            }
            break;
        }
        default:
            break;
    }
}

void JCWriter::write(Value &v, std::ostream &out) {
    makeStrPool(v);
    int32_t n = (int32_t)stringTable.size();
    out.write((char*)(&n), 4);
    
    for (auto itr = stringTable.begin(); itr != stringTable.end(); ++itr) {
        const std::string &str = itr->first;
        uint8_t type = jc_string;
        writeSize(out, type, str.size());
        out.write(str.data(), str.size());
    }
    
    writeValue(v, out);
}

void JCWriter::writeValue(Value &v, std::ostream &out) {
    switch (v.type()) {
        case Type::String: {
            std::string &str = *v.asStr();
            auto it = stringTable.find(str);
            if (it == stringTable.end()) {
                printf("ERROR: miss str pool: %s\n", str.c_str());
                break;
            }

            uint8_t type = jc_ref;
            writeSize(out, type, it->second);
            //puts("~");
            
            /*
            if (left.size() > 0) {
                jute::jValue strVal(JSTRING);
                strVal.set_string(left);
                write(strVal, out);
            }*/
            break;
        }
        case Type::Object: {
            writeSize(out, jc_object, v.size());
            for (int i=0; i<v.size(); ++i) {
                Value name;
                Value val;
                v.getProp(i, name, val);
                writeValue(name, out);
                
                //set value
                writeValue(val, out);
            }
            break;
        }
        case Type::Array: {
            writeSize(out, jc_array, v.size());
            for (int i=0; i<v.size(); ++i) {
                Value sv = v[i];
                writeValue(sv, out);
            }
            break;
        }
        case Type::Boolean: {
            bool val = v.asBool();
            uint8_t subType;
            uint8_t type = jc_primi;
            subType = val ? jc_true : jc_false;
            type = (type << 4) | subType;
            out.write((char*)(&type), 1);
            break;
        }
        case Type::Integer: {
            int64_t i = v.asInt();
            writeSize(out, jc_int, i);
            break;
        }
        case Type::Double: {
            double f = v.asDouble();
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

static int64_t readSize(std::istream &inStream, jc_type &type, uint8_t subType) {
    switch (subType) {
        case 11:
            return -1;
            break;
        case 12: {
            int8_t i = 0;
            inStream.read((char*)(&i), 1);
            return i;
            break;
        }
        case 13:{
            int16_t i = 0;
            inStream.read((char*)(&i), 2);
            return i;
            break;
        }
        case 14:{
            int32_t i = 0;
            inStream.read((char*)(&i), 4);
            return i;
            break;
        }
        case 15:{
            int64_t i = 0;
            inStream.read((char*)(&i), 8);
            return i;
            break;
        }
        default:
            break;
    }
    return subType;
}

Value JCReader::read(std::istream &inStream) {
    if (inStream.eof() || inStream.fail()) {
        return Value();
    }
    
    int32_t poolSize = 0;
    inStream.read((char*)(&poolSize), 4);
    pool.resize(poolSize);
    for (int32_t i=0; i<poolSize; ++i) {
        pool[i] = readValue(inStream);
    }
    
    return readValue(inStream);
}

Value JCReader::readValue(std::istream &inStream) {
    if (inStream.eof()) {
        return Value();
    }
    
    uint8_t t = 0;
    inStream.read((char*)(&t), 1);
    jc_type type = (jc_type)((t >> 4) & 0xff);
    uint8_t subType =  t & 0x0f;
    
    switch (type) {
        case jc_int: {
            Value Value(Type::Integer);
            int64_t size = readSize(inStream, type, subType);
            Value = size;
            return Value;
            break;
        }
        case jc_float: {
            Value Value(Type::Double);
            if (subType < 11) {
                Value = (double)subType;
            } else if (subType == jc_int64) {
                double d;
                inStream.read((char*)(&d), sizeof(double));
                Value = d;
            } else if (subType == jc_int32) {
                float d;
                inStream.read((char*)(&d), sizeof(float));
                Value = d;
            } else {
                //error
            }
            return Value;
            break;
        }
        case jc_primi: {
            if (subType == jc_null) {
                return Value(Type::Null);
            }
            else if (subType == jc_true) {
                Value Value(Type::Boolean);
                Value = true;
                return Value;
            }
            else if (subType == jc_false) {
                Value Value(Type::Boolean);
                Value = false;
                return Value;
            }
            else {
                return Value();
            }
            break;
        }
        case jc_array: {
            int64_t size = readSize(inStream, type, subType);
            Value value(Type::Array, size);
            for (int i=0; i<size; ++i) {
                Value v = readValue(inStream);
                value.add(v);
            }
            return value;
            break;
        }
        case jc_string: {
            Value value(Type::String);
            int64_t size = readSize(inStream, type, subType);
            std::string *s = value.asStr();
            s->resize(size);
            inStream.read((char*)s->data(), size);
            //stringTable.push_back(*s);
            //printf("%s\n", s.c_str());
            return value;
            break;
        }
        case jc_ref: {
            int64_t size = readSize(inStream, type, subType);
            if (size < pool.size()) {
                return pool[size].clone();
            }
            return Value(Type::Null);
            break;
        }
        case jc_object: {
            int64_t size = readSize(inStream, type, subType);
            Value value(Type::Object, size);
            for (int i=0; i<size; ++i) {
                Value k = readValue(inStream);
                Value v = readValue(inStream);
                value.set(k, v);
            }
            return value;
            break;
        }
        default: {
            printf("ERROR: unknow type: %d", type);
            break;
        }
    }

    Value Value;
    return Value;
}
