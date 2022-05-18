#include "fhbpt.h"
#include <map>
#include <queue>
#include <string>
#include <iostream>
#include <math.h>
using namespace std;

fhbpt::fhbpt() {
    root = (node_t *) new leaf_t(DEFAULT_LOWER, DEFAULT_UPPER);
}

// update the new cd at leaf[pos]
cd_t* fhbpt::encode_(leaf_t* leaf, pos_t pos) {
    cd_t left = leaf->lower;
    cd_t right = leaf->upper;
    cd_t lower = INVAL, upper = INVAL;
    if (pos > 0) {
        left = leaf->cds[pos - 1];
    }
    if (pos < leaf->imax - 1) {
        right = leaf->cds[pos];
    }
    cd_t cd = (cd_t) ceil((left + right) / 2.0);
    if (abs(right - left) <= 1) {
        // if encode_ can't give a valid cd to leaf[pos]
        // let recode_ do it
        cd_t* lu = recode_(leaf);
        lower = lu[0];
        upper = lu[1];
        cd = leaf->cds[pos];
    } else {
        leaf->cds[pos] = cd;
    }
    return new cd_t[3] {cd, lower, upper};
}

// recode all necessary leaf in this layer
cd_t* fhbpt::recode_(leaf_t* leaf) {
    leaf_t* lleaf = leaf;
    leaf_t* rleaf = leaf;
    size_t imax = leaf->imax;
    while (rleaf->upper - lleaf->lower < imax) {
        if (lleaf->lbro) {
            lleaf = lleaf->lbro;
            imax += lleaf->imax;
        } else {
            lleaf->lower *= 2;
        }
        if (rleaf->rbro) {
            rleaf = rleaf->rbro;
            imax += rleaf->imax;
        } else {
            rleaf->upper *= 2;
        }
    }
    int frag = (rleaf->upper - lleaf->lower) / imax;
    // tmp leaf
    leaf_t* cleaf = lleaf;
    cd_t cd = lleaf->lower;

    for(int i = 0; i < cleaf->imax; ++i) {
        cd += frag;
        cleaf->cds[i] = cd;
    }
    while (cleaf != rleaf) {
        cleaf->upper = cd;
        cleaf = cleaf->rbro;
        cleaf->lower = cd;
        for(int i = 0; i < cleaf->imax; ++i) {
            cd += frag;
            cleaf->cds[i] = cd;
        }
    }
    return new cd_t[2] {lleaf->lower, rleaf->upper};
}

cd_t fhbpt::getCode_(node_t* node_, pos_t pos) {
    if (node_->node_attr == LEAF) {
        leaf_t* node = (leaf_t*) node_;
        return node->cds[pos];
    } else {
        internal_node_t* node = (internal_node_t*) node_;
        int i;
        for(i = 0; i < node->imax && pos > node->kwds[i]; ++i) {
            pos = pos - node->kwds[i];
        }
        return getCode_(node->children[i], pos);
    }
}

cd_t* fhbpt::insert_(node_t* node_, pos_t pos, ct_t ct) {
    if (node_->node_attr == LEAF) {
        leaf_t* node = (leaf_t*) node_;

        for(int i = pos; i < node->imax; ++i) {
            node->cts[i + 1] = node->cts[i];
            node->cds[i + 1] = node->cds[i];
        }
        node->cts[pos] = ct;
        node->imax++;

        cd_t* clu = encode_(node, pos);
        if (node->imax > MAX_NODE_SIZE) {
            rebalance_((node_t*) node);
            clu[0] = node->cds[pos];
        }
        node->cds[pos] = clu[0];
        return clu;
    } else {
        internal_node_t * node = (internal_node_t*) node_;
        int i;
        for(i = 0; i < node->imax && pos > node->kwds[i]; ++i) {
            pos -= node->kwds[i];
        }
        node->kwds[i]++;
        return insert_(node->children[i], pos, ct);
    }
}

void fhbpt::rebalance_(node_t* node_) {
    int newNodeImax = (MAX_NODE_SIZE / 2);
    int nodeSubTreeSize = 0;
    int newNodeSubTreeSize = 0;
    node_t* resNode, * resNewNode;
    if (node_->node_attr == LEAF) {
        leaf_t* node = (leaf_t*) node_;
        int newNodeStartIndex = node->imax - newNodeImax;
        cd_t oldNodeUpper = node->cds[newNodeStartIndex - 1];
        cd_t newNodeLower = oldNodeUpper;
        cd_t newNodeUpper = node->upper;
        leaf_t* rightBro = node->rbro;

        leaf_t* newNode = new leaf_t(newNodeLower, newNodeUpper);
        node->imax -= newNodeImax;
        newNode->imax = newNodeImax;
        memcpy(newNode->cds, node->cds + newNodeStartIndex, newNodeImax * sizeof(cd_t));
        memcpy(newNode->cts, node->cts + newNodeStartIndex, newNodeImax * sizeof(ct_t));
        node->rbro = newNode;
        newNode->lbro = node;
        newNode->rbro = rightBro;
        node->upper = oldNodeUpper;

        nodeSubTreeSize = node->imax;
        newNodeSubTreeSize = newNode->imax;

        resNode = (node_t*) node;
        resNewNode = (node_t*) newNode;
    } else {
        internal_node_t* node = (internal_node_t*) node_;
        int newNodeStartIndex = node->imax - newNodeImax;

        internal_node_t* newNode = new internal_node_t();
        node->imax -= newNodeImax;
        newNode->imax = newNodeImax;
        memcpy(newNode->kwds, node->kwds + newNodeStartIndex, newNodeImax * sizeof(kwd_t));
        memcpy(newNode->children, node->children + newNodeStartIndex, newNodeImax * sizeof(node_t*));
        for(int i = 0; i < newNode->imax; ++i) {
            newNode->children[i]->parent = newNode;
        }

        for(int i = 0; i < node->imax; ++i) {
            nodeSubTreeSize += node->kwds[i];
        }
        for(int i = 0; i < newNode->imax; ++i) {
            newNodeSubTreeSize += newNode->kwds[i];
        }
        resNode = (node_t*) node;
        resNewNode = (node_t*) newNode;
    }

    if (!node_->parent) {
        internal_node_t* root_ = new internal_node_t();
        root_->imax = 1;
        root_->children[0] = resNode;
        resNode->parent = root_;
    
        root = (node_t*) root_;
    }

    internal_node_t* parent = resNode->parent;
    int nodeIndex;
    for(nodeIndex = 0; nodeIndex < parent->imax; ++nodeIndex) {
        if (parent->children[nodeIndex] == resNode) {
            break;
        }
    }

    parent->imax++;
    memcpy(parent->kwds + nodeIndex + 2, parent->kwds + nodeIndex + 1, (parent->imax - nodeIndex - 2) * sizeof(kwd_t));
    memcpy(parent->children + nodeIndex + 2, parent->children + nodeIndex + 1, (parent->imax - nodeIndex - 2) * sizeof(node_t*));

    parent->kwds[nodeIndex] = nodeSubTreeSize;

    parent->kwds[nodeIndex + 1] = newNodeSubTreeSize;
    parent->children[nodeIndex + 1] = resNewNode;
    resNewNode->parent = parent;

    if (parent->imax > MAX_NODE_SIZE) {
        rebalance_((node_t*)parent);
    }
}

cd_t fhbpt::getCode(pos_t pos) {
    return getCode_(root, pos);
}

void fhbpt::insert(pos_t pos, ct_t ct) {
    insert_(root, pos, ct);
}

#define PTR 0
#define STR 1

struct blk_t {
    bool blkAttr;
    string s;
    node_t* child;
    blk_t(bool blkAttr_, string s_="", node_t* child_=NULL) {
        blkAttr = blkAttr_;
        s = s_;
        child = child_;
    }
};
string fhbpt::traverse_() {
    queue<blk_t*> q;
    if (root->node_attr == LEAF) {
        return ((leaf_t*) root)->toString();
    }
    internal_node_t* tmpRoot = (internal_node_t*) root;
    q.push(new blk_t(STR, tmpRoot->toString(), NULL));
    for(int i = 0; i < tmpRoot->imax; ++i) {
        q.push(new blk_t(PTR, "", tmpRoot->children[i]));
    }
    string s = "";
    while (!q.empty()) {
        int qLen = q.size();
        for (int i = 0; i < qLen; ++i) {
            blk_t* blk = q.front();
            if (blk->blkAttr == STR) {
                s += blk->s + "\t";
            } else {
                if (blk->child->node_attr == LEAF) {
                    leaf_t* tmpNode = (leaf_t*) blk->child;
                    q.push(new blk_t(STR, tmpNode->toString(), NULL));
                } else {
                    internal_node_t* tmpNode = (internal_node_t*) blk->child;
                    q.push(new blk_t(STR, tmpNode->toString(), NULL));
                    for(int i = 0; i < tmpNode->imax; ++i) {
                        q.push(new blk_t(PTR, "", tmpNode->children[i]));
                    }
                }
            }
            q.pop();
        }
        s += "\n";
    }
    return s;
}

string fhbpt::traverse() {
    return traverse_();
}