#include "fhope.h"
#include <string>
#include<iostream>
#include <sstream>

int cnt = 0;
static string unicodes[] = {"\u2080", "\u2081", "\u2082", "\u2083", "\u2084", "\u2085", "\u2086", "\u2087", "\u2088", "\u2089"};
ct_t rndEnc(int pt) {
    string rndSubNum = to_string(cnt++);
    stringstream ss;
    ss << to_string(pt) << "\u2091";
    for(char& c: rndSubNum) {
        ss << unicodes[c - '0'];
    }
    return ss.str();
}
int rndDec(ct_t ct) {
    return stoi(ct.substr(0, ct.find("\u2091")));
}
int fhope::enc(int pt) {
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
    return pos;
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
    ss << "digraph g {" << endl;
    ss << "node [shape = record,height=.1];" << endl;
    // digraph g {
    // node [shape = record,height=.1];
    //     0[label = "{plaintext|1|3|4}|{count|4|5|6}"];
    // }
    ss << "    0[label = \"{plaintext";
    for(auto kv: stCli) {
        ss << "|";
        ss << kv.first;
    }
    ss << "}|{count";
    for(auto kv: stCli) {
        ss << "|";
        ss << kv.second;
    }
    ss << "}\"];" << endl;
    ss << "}" << endl;
    return ss.str();
}
string fhope::serTraverse() {
    return stSer.traverse();
}