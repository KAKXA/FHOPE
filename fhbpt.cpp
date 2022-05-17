#include "fhbpt.h"
#include <math.h>

fhbpt::fhbpt() {
    root = (node_t *)(new leaf_t(DEFAULT_LOWER, DEFAULT_UPPER));
}

cd_t* fhbpt::encode_(leaf_t* leaf, pos_t pos) {
    cd_t left = leaf->lower;
    cd_t right = leaf->upper;
    cd_t lower = INVAL, upper = INVAL;
    if (pos > 1) {
        left = leaf->cds[pos - 1];
    }
    if (pos < IMAX) {
        right = leaf->cds[pos];
    }
    cd_t cd = (cd_t) ceil((left + right) / 2.0);
    if (abs(right - left) <= 1) {
        cd_t* lu = recode_(leaf);
        lower = lu[0];
        upper = lu[1];
        cd = leaf->cds[pos];
    }
    return new cd_t[3] {cd, lower, upper};
}

cd_t* fhbpt::recode_(leaf_t* leaf) {
    leaf_t* lleaf = leaf;
    leaf_t* rleaf = leaf;
    size_t imax = IMAX;
    while (rleaf->upper - lleaf->lower < imax) {
        if (lleaf->lbro) {
            lleaf = lleaf->lbro;
            imax += IMAX;
        } else if (lleaf->lower < 0) {
            lleaf->lower *= 2;
        }
        if (rleaf->rbro) {
            rleaf = rleaf->rbro;
            imax += IMAX;
        } else {
            rleaf->upper *= 2;
        }
    }
    int frag = (rleaf->upper - lleaf->lower) / imax;
    // tmp leaf
    leaf_t* cleaf = lleaf;
    cd_t cd = lleaf->lower;

    for(int i = 0; i < IMAX; ++i) {
        cd += frag;
        cleaf->cds[i] = cd;
    }
    while (cleaf != rleaf) {
        cleaf->upper = cd;
        cleaf = cleaf->rbro;
        cleaf->lower = cd;
        for(int i = 0; i < IMAX; ++i) {
            cd += frag;
            cleaf->cds[i] = cd;
        }
    }

    return new cd_t[2] {lleaf->lower, rleaf->upper};
}
