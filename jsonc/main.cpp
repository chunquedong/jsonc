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
    for (int i=0; i<500; ++i) {
        JsonParser parser;
        Value value = parser.parse(str);
        value.free();
    }
    clock_t t1 = clock();
    printf("parse: %ld\n", (t1-t0)*1000/CLOCKS_PER_SEC);

    
    /*JsonParser parser;
    Value value0 = parser.parse(str);
    printf("%s\n", parser.getError().c_str());
    std::string jstr;
    value0.toJson(jstr);
    printf("%s\n", jstr.c_str());
    value0.free();*/
}

/////////////////////////////////////////////////////////////////
// Min Size
/////////////////////////////////////////////////////////////////

bool packJson(std::string& jsonstr, std::ostream& out) {
    JsonParser parser;
    Value v = parser.parse(jsonstr);
    JCWriter c;
    c.write(v, out);
    v.free();
    return true;
}


void writeTest() {
    std::ifstream in("./data.json");
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) str += tmp;

    std::ofstream out("./data_out.jc", std::ofstream::binary | std::ofstream::out);
    packJson(str, out);
}

void readTest() {
    std::ifstream inStream("./data_out.jc", std::ifstream::binary | std::ifstream::in);
    inStream.seekg(0, std::ios::end);
    long length = inStream.tellg();
    inStream.seekg(0, std::ios::beg);

    char* buffer = (char*)malloc(length);
    inStream.read(buffer, length);

    clock_t t0 = clock();
    for (int i = 0; i < 500; ++i) {
        JCReader r(buffer, length);
        Value value = r.read();
        value.free();
    }
    clock_t t1 = clock();
    printf("decompress: %ld\n", (t1 - t0) * 1000 / CLOCKS_PER_SEC);

    //JCReader r(buffer, length);
    //Value value0 = r.read();
    //std::string str;
    //value0.toJson(str);
    //printf("%s\n", str.c_str());
    //value0.free();

    free(buffer);
}

/////////////////////////////////////////////////////////////////
// Fast Decode
/////////////////////////////////////////////////////////////////

bool packJson2(std::string& jsonstr, std::ostream& out) {
    JsonParser parser;
    Value v = parser.parse(jsonstr);
    JEncoder encoder;
    auto buffer = encoder.encode(v);
    out.write(buffer.data(), buffer.size());
    v.free();
    return true;
}

void writeTest2() {
    std::ifstream in("./data.json");
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) str += tmp;
    
    std::ofstream out("./data_out.jc2", std::ofstream::binary | std::ofstream::out);
    packJson2(str, out);
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

    //std::string str;
    //value0->toJson(str);
    //printf("%s\n", str.c_str());

    free(buffer);
}

/////////////////////////////////////////////////////////////////
// Tools
/////////////////////////////////////////////////////////////////

void convert(const char *from, const char *to, int version) {
    std::ifstream in(from, std::ifstream::binary | std::ifstream::in);
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) str += tmp;
    
    std::ofstream out(to);
    if (version == 2) {
        packJson2(str, out);
    }
    else {
        packJson(str, out);
    }
}

void dump(const char *from) {
    std::ifstream inStream(from, std::ifstream::binary | std::ifstream::in);
    inStream.seekg(0, std::ios::end);
    long length = inStream.tellg();
    inStream.seekg(0, std::ios::beg);

    char* buffer = (char*)malloc(length);
    inStream.read(buffer, length);

    int32_t *version = (int32_t*)(buffer + 4);

    if (*version == 2) {
        Value* json = JEncoder::decode(buffer, length);
        std::string str;
        json->toJson(str);
        printf("%s\n", str.c_str());
    }
    else {
        JCReader r(buffer, length);
        Value value = r.read();
        std::string str;
        value.toJson(str);
        printf("%s\n", str.c_str());
        value.free();
    }

    free(buffer);
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
        int version = 1;
        int i = 1;
        while (i<argc) {
            if (*argv[i] == '-') {
                if (!strcmp(argv[i], "-d")) {
                    dmp = true;
                }
                else if (!strcmp(argv[i], "-v2")) {
                    version = 2;
                }
                ++i;
            } else {
                break;
            }
        }


        if (dmp == false && i+1 < argc) {
            const char *from = argv[i];
            const char *to = argv[i+1];
            convert(from, to, version);
            return 0;
        }
        else if (dmp && i < argc) {
            const char *from = argv[i];
            dump(from);
            return 0;
        }
    }
    
    printf("Usage:\n");
    printf("  jsonc [-v2] <input json file> <output file>\n");
    printf("  jsonc -d <input binary file>\n");
    return 1;
}
