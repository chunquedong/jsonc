//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#ifndef encoder_hpp
#define encoder_hpp

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
#include "jparser.hpp"

namespace jc {

class JEncoder {
    std::map<const std::string, size_t> stringTable;
    std::vector<std::string> stringPool;
    std::vector<char> buffer;
public:    
    std::vector<char> &encode(Value &v);

    static Value *decode(char *buffer, int size) {
        int32_t *jmp = (int32_t*)buffer;
        if (*jmp > size) return NULL;
        return (Value*)(buffer + *jmp);
    }
private:
    void makeStrPool(Value &v);
    void writeValue(Value &v);
    void writeData(const char *data, int size, int at = -1);
    void fillEmpty(int size);
};

}//ns
#endif /* cmp_hpp */

