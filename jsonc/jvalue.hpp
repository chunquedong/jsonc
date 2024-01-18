//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#ifndef jValue_hpp
#define jValue_hpp

#include <inttypes.h>
#include <vector>
#include <string>
#include <string.h>

namespace jc {

    class Value;
    struct JsonNode;
    JsonNode* link_reverse(JsonNode* head);
    
    enum class Type : uint8_t {
        Null,
        String,
        Array,
        Object,
        Integer,
        Float,
        Boolean,
    };
    
    struct ImuInfo {
        //self offset in buffer
        uint32_t selfOffset;
        //Array/Object size or String position
        uint32_t sizeOrPos;
    };

    struct JsonIterator {
        Value* parent;
        union
        {
            int index;
            JsonNode* cur;
        };
        void operator++();
        bool operator!=(const JsonIterator& x) const;
        Value* operator*() const;
        Value* operator->() const;
        const char* get_name() const;
    };

    class Value {
    public:
        union {
            int64_t i;
            double d;
            bool b;
            char *str; //String, StringRef
            ImuInfo imuInfo; //_flag == 1
            JsonNode* child; //Array, Object
        } value;
        Type _type;
        uint8_t _flag;
 
    public:
        Value(Type t = Type::Null) : _type(t), _flag(0) { value.i = 0; }
        
        void set_int(const int64_t other);
        void set_float(const double other);
        void set_bool(const bool other);
        void set_str(char *str);
        
        Type type() { return _type; }
        void set_type(Type t) { _type = t; }
        
        const char *as_str(const char *defVal = "");
        int64_t as_int();
        double as_float();
        bool as_bool();
        bool is_null() {
            return _type == Type::Null ||
                (_type == Type::String || strcmp("null", as_str()) == 0);
        }
        
        size_t size();
        Value* get(const char* name);

        JsonIterator begin();
        JsonIterator end();

        //himl
        Value* children() { return get("_children"); }
        Value* objectType() { return get("_type"); }
        
        /**
        dump to json
        */
        void to_json(std::string &json, bool isHiml = false, int level = 0);
    };

    struct JsonNode: public Value {
        JsonNode* _next;
        const char* name;

        JsonNode() : _next(NULL), name(NULL) {}

        /** insert and overwrite by key */
        bool set(const char* key, JsonNode* val);
        void insert_pair(const char* key, JsonNode* val);

        void insert(JsonNode* val);
        void reverse();

        /** slow than insert */
        void append(JsonNode* val);
    };
    
    class JsonAllocator {
        struct Block {
            Block* next;
            size_t used;
        } *head;

    public:
        void swap(JsonAllocator& other);
        JsonAllocator() : head(nullptr) {};
        ~JsonAllocator();
        void* allocate(size_t size);

        inline JsonNode* allocNode(Type type) {
            JsonNode* v = (JsonNode*)allocate(sizeof(JsonNode));
            v->_type = type;
            return v;
        }

        inline char* strdup(const char* s) {
            char* str = (char*)allocate(strlen(s) + 1);
            strcpy(str, s);
            return str;
        }
    };
}//ns


#endif /* jValue_hpp */
