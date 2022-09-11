//
// Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
//  Created by Jed Young on 17/4/30.
//  Copyright © 2017 chunquedong. All rights reserved.
//


#include "jcompress.hpp"
#include "jparser.hpp"

using namespace jc;

void testParse() {
    std::ifstream in("./data.json");
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) str += tmp;
    
    clock_t t0 = clock();
    for (int i=0; i<100; ++i) {
        JsonParser parser;
        Value Value = parser.parse(str);
        Value.free();
    }
    
    clock_t t1 = clock();
    printf("t1: %ld\n", (t1-t0)*1000/CLOCKS_PER_SEC);
}

void writeTest() {
    std::ifstream in("./data.json");
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) str += tmp;
    
    std::ofstream out("./data_out.jc");
    JCWriter c;
    c.packJson(str, out);
}

void readTest() {
    std::ifstream inStream("./data_out.jc");
    
    clock_t t0 = clock();
    for (int i=0; i<100; ++i) {
        inStream.seekg(0);
        JCReader r;
        Value Value2 = r.read(inStream);
        Value2.free();
    }
    clock_t t1 = clock();
    printf("t1: %ld\n", (t1-t0)*1000/CLOCKS_PER_SEC);
}

void convert(const char *from, const char *to) {
    std::ifstream in(from);
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) str += tmp;
    
    std::ofstream out(to);
    JCWriter c;
    c.packJson(str, out);
}

void dump(const char *from) {
    std::ifstream inStream(from);
    JCReader r;
    Value Value2 = r.read(inStream);
    std::string json;
    Value2.toJson(json);
    printf("%s\n", json.c_str());
    Value2.free();
}

int main(int argc, const char * argv[]) {
    
    if (argc == 2) {
        if (!strcmp(argv[1], "test")) {
            testParse();
            writeTest();
            readTest();
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
