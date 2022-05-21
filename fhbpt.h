#ifndef FHBPT_H
#define FHBPT_H

#include <stdio.h>
#include "predefined.h"
#include <map>

#define MAX_NODE_SIZE 4
#define INTERNAL true
#define LEAF false
#define DEFAULT_LOWER -2
#define DEFAULT_UPPER 2
#define INVAL 0

struct internal_node_t;
struct node_t {
    bool node_attr;
    internal_node_t* parent;
};

struct internal_node_t {
    const bool node_attr = INTERNAL;
    node_t* parent;
    size_t imax = 0;
    kwd_t kwds[MAX_NODE_SIZE + 1];
    node_t* children[MAX_NODE_SIZE + 1];

    internal_node_t() {};
    internal_node_t(node_t* parent) {
        this->parent = parent;
    };
    std::string toString() {
        std::string s = "(";
        for(int i = 0; i < imax; ++i) {
            s += std::to_string(kwds[i]) + " ";
        }
        return s + ")";
    }
};

struct leaf_t {
    const bool node_attr = LEAF;
    node_t* parent;
    size_t imax = 0;
    ct_t cts[MAX_NODE_SIZE + 1];
    cd_t cds[MAX_NODE_SIZE + 1];
    leaf_t* lbro;
    leaf_t* rbro;
    cd_t lower;
    cd_t upper;
    leaf_t() {};
    leaf_t(cd_t lower, cd_t upper) {
        this->lower = lower;
        this->upper = upper;
    }
    leaf_t(cd_t lower, cd_t upper, node_t* parent) {
        this->lower = lower;
        this->upper = upper;
        this->parent = parent;
    }
    std::string toString() {
        std::string s = "(";
        for(int i = 0; i < imax; ++i) {
            s += cts[i] + "," + std::to_string(cds[i]) + " ";
        }
        return s + ")";
    }
};

class fhbpt {
    public:
    fhbpt();
    cd_t getCode(pos_t pos);
    void insert(pos_t pos, ct_t ct);
    std::string traverse();
    private:
    node_t* root;

    // return cd, lower, upper
    cd_t* encode_(leaf_t* leaf, pos_t pos);
    // return lower, upper
    cd_t* recode_(leaf_t* leaf);
    cd_t getCode_(node_t* node, pos_t pos);
    // return cd, lower, upper
    cd_t* insert_(node_t* node, pos_t pos, ct_t ct);
    void rebalance_(node_t* node);
    std::string traverse_();
    std::string traverseGraphviz_();
};

#endif