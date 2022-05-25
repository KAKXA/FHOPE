#include<string>
#include<iostream>
#include<string.h>
#include"fhbpt.h"
#include"fhope.h"
#include<queue>
#include<math.h>
#include<fstream>
using namespace std;

int main () {
    fhope f;
    // srand(time(NULL) % UINT32_MAX);
    for(int i = 0; i < 20; ++i) {

        int rt = rand() % 5;
        int rank = f.enc(rt);
        cout << "Insert " << rt << " as at position " << rank << endl;

        ofstream ofCli("cli_dot/" + to_string(i) + ".dot");
        ofCli << f.cliTraverse() << endl;

        ofstream ofSer("ser_dot/" + to_string(i) + ".dot");
        ofSer << f.serTraverse() << endl;
    }
}