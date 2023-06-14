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
        
        int cur;
        int pos;
        std::string error;
    public:
        JsonParser() : input(str), cur(-1), pos(-1) {}
        
        Value parse(std::string &str) {
            input = str;
            pos = -1;
            cur = -1;
            consume();
            skipWhitespace();
            return parseVal();
        }

        const std::string& getError() { return error; }
        
    private:
        Value parseObj();
        void parsePair(Value obj);
        Value parseVal();
        Value parseNum();
        void parseStr(std::string &str);
        Value parseArray();
        std::string escape();
    private:
    
        void consume(int i = 1) {
            pos += i;
            if (pos < input.size()) {
                cur = (unsigned char)input[pos];
            } else {
                cur = -1;
            }
        }
        
        bool maybe(int tt) {
            if (cur != tt) return false;
            consume();
            return true;
        }
        
        void expect(int tt) {
            if (cur < 0) {
                error = ("Unexpected end of stream");
                return;
            }
            else if (cur != tt) {
                char buf[128];
                snprintf(buf, 128, "'%c' Expected, got '%c' at %d", tt, cur, pos);
                error = buf;
                return;
            }
            consume();
        }
        
        void skipWhitespace() {
            while (isspace(cur))
                consume();
        }
    };
    
}

#endif /* jsonc_hpp */
