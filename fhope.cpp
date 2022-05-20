#include "fhope.h"
#include <string>
#include<iostream>
#include <sstream>

int cnt = 0;
ct_t rndEnc(int pt) {
    return to_string(pt) + "c" + to_string(cnt++);
}
int rndDec(ct_t ct) {
    return stoi(ct.substr(0, ct.find('c')));
}
ct_t fhope::enc(int pt) {
    ct_t ct = rndEnc(pt);
    int lessCount = 0;
    for(auto itr = stCli.begin(); itr != stCli.upper_bound(pt - 1); ++itr) {
        lessCount += itr->second;
    }
    int pos;
    if (stCli.count(pt)) {
        int ptCount = stCli[pt];
        pos = lessCount + rand() % (ptCount + 1);
        stCli[pt]++;
    } else {
        pos = lessCount;
        stCli[pt] = 1;
    }
    // server
    stSer.insert(pos, ct);
    return ct;
}
int fhope::dec(ct_t ct) {
    return rndDec(ct);
}
cd_t fhope::search(int pt) {
    int pos = 0;
    for(auto itr = stCli.begin(); itr != stCli.upper_bound(pt - 1); ++itr) {
        pos += itr->second;
    }
    // server
    return stSer.getCode(pos);
}

string fhope::cliTraverse() {
    stringstream ss;
    for(auto kv: stCli) {
        ss << kv.first << ":" << kv.second << endl;
    }
    return ss.str();
}
string fhope::serTraverse() {
    return stSer.traverse();
}