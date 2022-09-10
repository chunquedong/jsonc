//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#ifndef jsonc_hpp
#define jsonc_hpp

#include "jvalue.hpp"

namespace jc {
    
    enum class JsonToken {
        objectStart = '{',
        objectEnd = '}',
        colon = ':',
        arrayStart = '[',
        arrayEnd = ']',
        comma = ',',
        quote = '"',
        grave = '`',
    };
    
    class JsonParser {
        std::string str;
        std::string &input;
        
        char cur;
        int pos;
        
    public:
        JsonParser() : input(str) {}
        
        Value parse(std::string &str) {
            input = str;
            pos = -1;
            cur = -1;
            consume();
            skipWhitespace();
            return parseVal();
        }
        
    private:
        Value parseObj();
        void parsePair(Value obj);
        Value parseVal();
        Value parseNum();
        void parseStr(std::string &str);
        Value parseArray();
        std::string escape();
    private:
        std::exception err(const std::string &msg);
    
        void consume(int i = 1) {
            pos += i;
            if (pos < input.size()) {
                cur = input[pos];
            } else {
                cur = -1;
            }
        }
        
        bool maybe(char tt) {
            if (cur != tt) return false;
            consume();
            return true;
        }
        
        void expect(char tt) {
            if (cur < 0) throw err("Unexpected end of stream");
            if (cur != tt) throw err("Expected Error");
            consume();
        }
        
        void skipWhitespace() {
            while (isspace(cur))
                consume();
        }
    };
    
}

#endif /* jsonc_hpp */
