#include <map>
#include "predefined.h"
#include "fhbpt.h"
using namespace std;

class fhope {
    public:
    fhope() {}
    int enc(int pt);
    int dec(ct_t ct);
    cd_t search(int pt);
    string cliTraverse();
    string serTraverse();
    private:
    map<int, int> stCli;
    fhbpt stSer;
};
