//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#ifndef jValue_hpp
#define jValue_hpp

#include <vector>
#include <map>
#include <string>
#include <cstring>

namespace jc {
    
    enum class Type {
        String,
        Array,
        Object,
        Integer,
        Double,
        Boolean,
        Null,
        //Unknow,
    };
    
    class JCMap;
    class JCArray;
    
    class Value {
        union {
            int64_t i;
            double d;
            bool b;
            std::string *str;
            JCArray *array;
            JCMap *map;
        } value;
        Type _type;
    private:
        void init();
    public:
        Value(Type t = Type::Null);
        void free();
        
        //Value(const Value &other);
        //Value &operator=(const Value& other);
        
        Value &operator=(const int64_t other);
        Value &operator=(const std::string& other);
        Value &operator=(const double other);
        Value &operator=(const bool other);
        
        Type type() { return _type; }
        void setType(Type t);
        
        std::string *asStr(std::string *defVal = nullptr);
        int64_t asInt();
        double asDouble();
        bool asBool();
        
        size_t size();
        bool has(std::string &name);
        
        Value operator[](size_t i);
        Value operator[](std::string &name);
        
        bool set(Value key, Value val);
        bool add(Value val);
        bool getProp(int i, Value &key, Value &val);
        
        void toJson(std::string &json, int level = 0);
    };
    
    class JCArray {
    public:
        std::vector<Value> array;
    public:
        ~JCArray();
    };
    
    class JCMap {
        std::vector<Value> properties;
        std::map<std::string, size_t> map;
    public:
        ~JCMap();
        
        size_t size() { return properties.size()/2; }
        Value key(size_t i) { return properties[i*2]; }
        Value val(size_t i) { return properties[i*2+1]; }
        Value get(std::string &name);
        void set(Value key, Value val);
        bool has(std::string &name);
    };
    
}//ns


#endif /* jValue_hpp */
