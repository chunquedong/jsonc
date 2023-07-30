//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "jencoder.hpp"
#include "jcompress.hpp"
#include "jparser.hpp"
#include "HimlParser.hpp"
#include <time.h>
#include <fstream>

using namespace jc;

#define LOOP_TIMES 1000

//#define TEST_FILE "D:\\workspace\\source\\nativejson-benchmark\\data\\twitter.json"
#define TEST_FILE "./data.json"

void testParse() {
    const char* file = TEST_FILE;

    std::ifstream in(file);
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) str += tmp;
    
    clock_t t0 = clock();
    for (int i=0; i< LOOP_TIMES; ++i) {
        std::string copy = str;
        JsonAllocator allocator;
        JsonParser parser(&allocator);
        Value *value = parser.parse((char*)copy.c_str());
    }
    clock_t t1 = clock();
    printf("parse: %ld\n", (t1-t0)*1000/CLOCKS_PER_SEC);

#if 0
    if (true) {
        std::string copy = str;
        JsonAllocator allocator;
        JsonParser parser(&allocator);
        Value *value0 = parser.parse((char*)copy.c_str());

        if (parser.get_error()[0] == 0) {
            std::string jstr;
            value0->to_json(jstr);
            puts(jstr.c_str());
        }
        else {
            puts(parser.get_error());
        }
    }
#endif
}

/////////////////////////////////////////////////////////////////
// Min Size
/////////////////////////////////////////////////////////////////
#ifndef JC_NO_COMPRESS

bool packJson(std::string& jsonstr, std::ostream& out) {
    JsonAllocator allocator;
    JsonParser parser(&allocator);
    Value* v = parser.parse((char*)jsonstr.c_str());
    JCWriter c;
    c.write(v, out);
    return true;
}

void writeTest() {
    std::ifstream in(TEST_FILE);
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
    for (int i = 0; i < LOOP_TIMES; ++i) {
        JsonAllocator allocator;
        JCReader r(buffer, length, &allocator);
        Value* value = r.read();
        //value.free();
    }
    clock_t t1 = clock();
    printf("decompress: %ld\n", (t1 - t0) * 1000 / CLOCKS_PER_SEC);

#if 0
    JsonAllocator allocator;
    JCReader r(buffer, length, &allocator);
    Value* value0 = r.read();
    std::string str;
    value0->to_json(str);
    printf("%s\n", str.c_str());
    //value0.free();
#endif
    free(buffer);
}
#endif
/////////////////////////////////////////////////////////////////
// Fast Decode
/////////////////////////////////////////////////////////////////
#ifndef NO_JC_ENCODER

bool packJson2(std::string& jsonstr, std::ostream& out) {
    JsonAllocator allocator;
    JsonParser parser(&allocator);
    Value* v = parser.parse((char*)jsonstr.c_str());
    JEncoder encoder;
    auto buffer = encoder.encode(v);
    out.write(buffer.data(), buffer.size());
    //v.free();
    return true;
}

void writeTest2() {
    std::ifstream in(TEST_FILE);
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
    for (int i = 0; i < LOOP_TIMES; ++i) {
        value0 = JEncoder::decode(buffer, length);
    }
    clock_t t1 = clock();
    printf("decode: %ld\n", (t1 - t0) * 1000 / CLOCKS_PER_SEC);

#if 0
    std::string str;
    value0->to_json(str);
    puts(str.c_str());
#endif
    free(buffer);
}
#endif
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
        printf("unsupport version:%d\n", *version);
    }
    else if (*version == 3) {
        Value* json = JEncoder::decode(buffer, length);
        std::string str;
        json->to_json(str);
        puts(str.c_str());
    }
    else if (*version == 1) {
        JsonAllocator allocator;
        JCReader r(buffer, length, &allocator);
        Value* value = r.read();
        std::string str;
        value->to_json(str);
        puts(str.c_str());
        //value.free();
    }

    free(buffer);
}

/////////////////////////////////////////////////////////////////
// Himl
/////////////////////////////////////////////////////////////////

void testHiml() {
    const char* file = "test.himl";

    std::ifstream in(file);
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) {
        str += tmp;
        str += "\r\n";
    }

    std::string copy = str;
    JsonAllocator allocator;
    HimlParser parser(&allocator);
    Value* value0 = parser.parse((char*)copy.c_str());

    if (parser.get_error()[0] == 0) {
        std::string jstr;
        value0->to_json(jstr, true);
        puts(jstr.c_str());
    }
    else {
        puts(parser.get_error());
    }
}

int main(int argc, const char * argv[]) {
    
    if (argc == 2) {
        if (!strcmp(argv[1], "test")) {
            testParse();
            writeTest();
            readTest();
            writeTest2();
            readTest2();
            //testHiml();
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
