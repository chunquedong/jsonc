//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "jencoder.hpp"
#include "jcompress.hpp"
#include "jparser.hpp"
#include <time.h>

using namespace jc;

void testParse() {
    std::ifstream in("./data.json");
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) str += tmp;
    
    clock_t t0 = clock();
    Value value0;
    for (int i=0; i<500; ++i) {
        JsonParser parser;
        Value value = parser.parse(str);
        if (i == 0) value0 = value;
        else value.free();
    }
    
    clock_t t1 = clock();
    printf("parse: %ld\n", (t1-t0)*1000/CLOCKS_PER_SEC);
    /*std::string jstr;
    value0.toJson(jstr);
    printf("%s\n", jstr.c_str());*/
    value0.free();
}

void writeTest2() {
    std::ifstream in("./data.json");
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) str += tmp;
    
    std::ofstream out("./data_out.jc2", std::ofstream::binary | std::ofstream::out);
    JEncoder c;
    c.packJson(str, out);
}

void readTest2() {
    std::ifstream inStream("./data_out.jc2", std::ifstream::binary|std::ifstream::in);

    inStream.seekg(0, std::ios::end);
    long length = inStream.tellg();
    inStream.seekg(0, std::ios::beg);
    
    char *buffer = (char*)malloc(length);
    inStream.read(buffer, length);

    clock_t t0 = clock();
    Value *value0;
    for (int i = 0; i < 500; ++i) {
        value0 = JEncoder::decode(buffer, length);
    }
    clock_t t1 = clock();
    printf("decode: %ld\n", (t1 - t0) * 1000 / CLOCKS_PER_SEC);

    /*std::string str;
    value0->toJson(str);
    printf("%s\n", str.c_str());*/

    free(buffer);
}

void writeTest() {
    std::ifstream in("./data.json");
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) str += tmp;

    std::ofstream out("./data_out.jc", std::ofstream::binary | std::ofstream::out);
    JCWriter c;
    c.packJson(str, out);
}

void readTest() {
    std::ifstream inStream("./data_out.jc", std::ifstream::binary | std::ifstream::in);
    std::stringstream tmp;
    tmp << inStream.rdbuf();

    clock_t t0 = clock();
    Value value0;
    for (int i = 0; i < 500; ++i) {
        tmp.seekg(0);
        JCReader r;
        Value value = r.read(tmp);
        if (i == 0) value0 = value;
        else value.free();
    }
    clock_t t1 = clock();
    printf("decompress: %ld\n", (t1 - t0) * 1000 / CLOCKS_PER_SEC);

    /*std::string str;
    value0.toJson(str);
    printf("%s\n", str.c_str());*/

    value0.free();
}

void convert(const char *from, const char *to) {
    std::ifstream in(from);
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) str += tmp;
    
    std::ofstream out(to);
    JEncoder c;
    c.packJson(str, out);
}

void dump(const char *from) {
    std::ifstream inStream(from);
    inStream.seekg(0, std::ios::end);
    long length = inStream.tellg();
    inStream.seekg(0, std::ios::beg);

    char *buffer = (char*)malloc(length);
    inStream.read(buffer, length);

    Value *json = JEncoder::decode(buffer, length);
    std::string str;
    json->toJson(str);
    printf("%s\n", str.c_str());
    json->free();
}

int main(int argc, const char * argv[]) {
    
    if (argc == 2) {
        if (!strcmp(argv[1], "test")) {
            testParse();
            writeTest();
            readTest();
            writeTest2();
            readTest2();
            return 0;
        }
    }
    else if (argc >= 3) {
        bool dmp = false;
        int i = 1;
        while (i<argc) {
            if (*argv[i] == '-') {
                if (!strcmp(argv[i], "-d")) {
                    dmp = true;
                }
                ++i;
            } else {
                break;
            }
        }
        if (dmp == false && i+1 < argc) {
            const char *from = argv[i];
            const char *to = argv[i+1];
            convert(from, to);
            return 0;
        }
        else if (dmp && i < argc) {
            const char *from = argv[i];
            dump(from);
            return 0;
        }
    }
    
    printf("Arg Error\n");
    return 1;
}
