//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#ifndef HimlParser_hpp
#define HimlParser_hpp

#include "jvalue.hpp"

namespace jsonc {

    /*
    * HIML is a JSON like text format
    */
    class HimlParser {
        char* src;
        char* cur;
        char error[128];
        JsonAllocator* allocator;
        char *delayStrTerminate;
    public:
        bool insertTopLevelObject = false;
        HimlParser() : src(NULL), cur(NULL), delayStrTerminate(NULL), allocator(NULL) {
            error[0] = 0;
        }
        HimlParser(JsonAllocator* allocator) : src(NULL), cur(NULL), delayStrTerminate(NULL) {
            error[0] = 0;
            this->allocator = allocator;
        }
        void init(JsonAllocator* allocator) { this->allocator = allocator; }
        
        Value* parse(char* src);

        const char* get_error() { return error; }
        
    private:
        inline JsonNode* alloc(Type type) {
            JsonNode* v = (JsonNode*)allocator->allocate(sizeof(JsonNode));
            v->_type = type;
            return v;
        }
        JsonNode* parseObj(const char* tagName = NULL, bool isRoot = false);
        JsonNode* parseVal();
        JsonNode* parseStr(JsonNode* val = NULL);
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

        int skipWhitespace() {
            int lines = 0;
            while (true) {
                switch (*cur)
                {
                case ' ':
                case '\t':
                    ++cur;
                    break;
                case '\r':
                case '\n':
                    ++lines;
                    ++cur;
                    break;
                case '/':
                    //skipComment
                    if (*(cur + 1) == '/') {
                        cur += 2;
                        char c = *cur;
                        while (c != '\r' && c != '\n' && c != 0) {
                            ++cur;
                            c = *cur;
                        }
                        break;
                    }
                    else {
                        return lines;
                    }
                default:
                    return lines;
                }
            }
            return lines;
        }
    };
    
}

#endif /* jsonc_hpp */
