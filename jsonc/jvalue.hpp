//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#ifndef jValue_hpp
#define jValue_hpp

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <cstring>

namespace jc {
    
    enum class Type {
        Null,
        String,
        Array,
        Object,
        Integer,
        Double,
        Boolean,
        ImuString,
        ImuArray,
        ImuObject,
        StringRef,
    };
    
    class JCMap;
    class JCArray;

    struct ImuInfo {
        uint32_t bufferOffset;
        uint32_t sizeOrPos;
    };
    
    class Value {
        friend class JsonParser;
        friend class JEncoder;
        friend class JCReader;

        union {
            int64_t i;
            double d;
            bool b;
            char *str; //String, StringRef
            JCArray *array;
            JCMap *map;
            ImuInfo imuInfo; //ImuString, ImuArray, ImuObject
        } value;
        
        Type _type;

    private:
        void init(size_t reserveSize = 10);
    public:
        Value() : _type(Type::Null) { value.i = 0; }
        Value(Type t, size_t reserveSize = 10);
        void free();
        
        Value &operator=(const int64_t other);
        Value &operator=(const std::string& other);
        Value &operator=(const double other);
        Value &operator=(const bool other);
        void setStr(char *str, bool copy);
        
        Type type() { return _type; }
        void setType(Type t);
        
        const char *asStr(const char *defVal = "");
        int64_t asInt();
        double asDouble();
        bool asBool();
        bool isNull() { return _type == Type::Null; }
        
        size_t size();
        bool has(const std::string &name);
        
        Value* operator[](size_t i) { return get(i); }
        Value* operator[](const std::string& name) { return get(name); }
        Value* get(size_t i);
        Value* get(const std::string& name);

        bool set(const std::string& key, Value val);
        bool add(Value val);
        Value *getProp(int i, std::string& key);
        
        void toJson(std::string &json, int level = 0);
        
        Value clone();
    private:
        void _add(const std::string& key, Value val);
    };
    

    
}//ns


#endif /* jValue_hpp */
