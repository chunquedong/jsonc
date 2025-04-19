//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#ifndef jsonc_hpp
#define jsonc_hpp

#include "jvalue.hpp"

namespace jsonc {

    class JsonParser {
        char* src;
        char* cur;
        char error[128];
        JsonAllocator* allocator;
    public:
        JsonParser() : src(NULL), cur(NULL), allocator(NULL) {
            error[0] = 0;
        }
        JsonParser(JsonAllocator* allocator) : src(NULL), cur(NULL) {
            error[0] = 0;
            this->allocator = allocator;
        }
        void init(JsonAllocator* allocator) { this->allocator = allocator; }
        
        Value* parse(char* src) {
            this->src = src;
            cur = src;
            return parseVal();
        }

        char* get_end() { return cur; }
        bool has_error() { return error[0] == 0; }
        const char* get_error() { return error; }
        
    private:
        inline JsonNode* alloc(Type type) {
            JsonNode* v = (JsonNode*)allocator->allocate(sizeof(JsonNode));
            v->_type = type;
            return v;
        }
        JsonNode* parseObj();
        JsonNode* parseVal();
        JsonNode* parseNum();
        JsonNode* parseStr(JsonNode* val = NULL);
        JsonNode* parseArray();
        void parseEscape(char* str);
    private:
    
        inline void consume() {
            ++cur;
        }
        
        inline void expect(char tt) {
            if (*cur != tt) {
                snprintf(error, 128, "'%c' Expected, got '%c' at %s", tt, *cur, cur);
                return;
            }
            ++cur;
        }
        
        void skipWhitespace() {
            while (isspace(*cur)) ++cur;
        }
    };
    
}

#endif /* jsonc_hpp */
