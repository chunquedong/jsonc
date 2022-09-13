//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#ifndef cmp_hpp
#define cmp_hpp

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

enum jc_type {
    jc_primi = 0,//0:null, 1:true, 2:false
    
    //const:0~10, 11:-1, 12:int8, 13:int16, 14:int32,15:int64
    jc_int,
    jc_float,
    jc_array,
    jc_object,
    
    jc_string,
    jc_ref,
    
    //jc_other,
};

enum jc_typeExt {
    jc_null = 0,
    jc_true = 1,
    jc_false = 2,
    
    jc_int8 = 12,
    jc_int16 = 13,
    jc_int32 = 14,
    jc_int64 = 15,
};

class JCWriter {
    std::map<const std::string, size_t> stringTable;
    std::vector<std::string> stringPool;
public:    
    void write(Value &v, std::ostream &outStream);
private:
    void writeValue(Value &v, std::ostream &outStream);
    void makeStrPool(Value &v);
};

class JCReader {
    std::vector<Value> pool;
    char* buffer;
    int size;
    int pos;
public:
    JCReader(char* buf, int size) : buffer(buf), size(size), pos(0) {}
    Value read();
private:

    /**
    * Must keep buf memery invalide.
    */
    Value readValue();

public:
    bool readData(char* data, int len);
    int readByte() {
        if (pos < size) {
            return buffer[pos++];
        }
        return -1;
    }
};

}//ns
#endif /* cmp_hpp */

