#include "fhbpt.h"
#include <map>
#include <functional>
#include <queue>
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
using namespace std;

fhbpt::fhbpt()
{
    root = (node_t *)new leaf_t(DEFAULT_LOWER, DEFAULT_UPPER);
}

// update the new cd at leaf[pos]
cd_t *fhbpt::encode_(leaf_t *leaf, pos_t pos)
{
    cd_t left = leaf->lower;
    cd_t right = leaf->upper;
    cd_t lower = INVAL, upper = INVAL;
    if (pos > 0)
    {
        left = leaf->cds[pos - 1];
    }
    if (pos < leaf->imax - 1)
    {
        right = leaf->cds[pos];
    }
    cd_t cd = (cd_t)ceil((left + right) / 2.0);
    if (abs(right - left) <= 1)
    {
        // if encode_ can't give a valid cd to leaf[pos]
        // let recode_ do it
        cd_t *lu = recode_(leaf);
        lower = lu[0];
        upper = lu[1];
        cd = leaf->cds[pos];
    }
    else
    {
        leaf->cds[pos] = cd;
    }
    return new cd_t[3]{cd, lower, upper};
}

// recode all necessary leaf in this layer
cd_t *fhbpt::recode_(leaf_t *leaf)
{
    leaf_t *lleaf = leaf;
    leaf_t *rleaf = leaf;
    size_t imax = leaf->imax;
    while (rleaf->upper - lleaf->lower < imax)
    {
        if (lleaf->lbro)
        {
            lleaf = lleaf->lbro;
            imax += lleaf->imax;
        }
        else
        {
            lleaf->lower *= 2;
        }
        if (rleaf->rbro)
        {
            rleaf = rleaf->rbro;
            imax += rleaf->imax;
        }
        else
        {
            rleaf->upper *= 2;
        }
    }
    int frag = (rleaf->upper - lleaf->lower) / imax;
    // tmp leaf
    leaf_t *cleaf = lleaf;
    cd_t cd = lleaf->lower;

    for (int i = 0; i < cleaf->imax; ++i)
    {
        cd += frag;
        cleaf->cds[i] = cd;
    }
    while (cleaf != rleaf)
    {
        cleaf->upper = cd;
        cleaf = cleaf->rbro;
        cleaf->lower = cd;
        for (int i = 0; i < cleaf->imax; ++i)
        {
            cd += frag;
            cleaf->cds[i] = cd;
        }
    }
    return new cd_t[2]{lleaf->lower, rleaf->upper};
}

cd_t fhbpt::getCode_(node_t *node_, pos_t pos)
{
    if (node_->node_attr == LEAF)
    {
        leaf_t *node = (leaf_t *)node_;
        return node->cds[pos];
    }
    else
    {
        internal_node_t *node = (internal_node_t *)node_;
        int i;
        for (i = 0; i < node->imax && pos > node->kwds[i]; ++i)
        {
            pos = pos - node->kwds[i];
        }
        return getCode_(node->children[i], pos);
    }
}

cd_t *fhbpt::insert_(node_t *node_, pos_t pos, ct_t ct)
{
    if (node_->node_attr == LEAF)
    {
        leaf_t *node = (leaf_t *)node_;

        for (int i = node->imax; i > pos; --i)
        {
            node->cts[i] = node->cts[i - 1];
            node->cds[i] = node->cds[i - 1];
        }
        node->cts[pos] = ct;
        node->imax++;

        cd_t *clu = encode_(node, pos);
        if (node->imax > MAX_NODE_SIZE)
        {
            rebalance_((node_t *)node);
            clu[0] = node->cds[pos];
        }
        node->cds[pos] = clu[0];
        return clu;
    }
    else
    {
        internal_node_t *node = (internal_node_t *)node_;
        int i;
        for (i = 0; i < node->imax && pos > node->kwds[i]; ++i)
        {
            pos -= node->kwds[i];
        }
        node->kwds[i]++;
        return insert_(node->children[i], pos, ct);
    }
}

void fhbpt::rebalance_(node_t *node_)
{
    int newNodeImax = (MAX_NODE_SIZE / 2);
    int nodeSubTreeSize = 0;
    int newNodeSubTreeSize = 0;
    node_t *resNode, *resNewNode;
    if (node_->node_attr == LEAF)
    {
        leaf_t *node = (leaf_t *)node_;
        int newNodeStartIndex = node->imax - newNodeImax;
        cd_t oldNodeUpper = node->cds[newNodeStartIndex - 1];
        cd_t newNodeLower = oldNodeUpper;
        cd_t newNodeUpper = node->upper;
        leaf_t *rightBro = node->rbro;

        leaf_t *newNode = new leaf_t(newNodeLower, newNodeUpper);
        node->imax -= newNodeImax;
        newNode->imax = newNodeImax;
        for (int i = 0; i < newNodeImax; ++i)
        {
            newNode->cds[i] = node->cds[i + newNodeStartIndex];
            newNode->cts[i] = node->cts[i + newNodeStartIndex];
        }
        node->rbro = newNode;
        newNode->lbro = node;
        newNode->rbro = rightBro;
        node->upper = oldNodeUpper;

        nodeSubTreeSize = node->imax;
        newNodeSubTreeSize = newNode->imax;

        resNode = (node_t *)node;
        resNewNode = (node_t *)newNode;
    }
    else
    {
        internal_node_t *node = (internal_node_t *)node_;
        int newNodeStartIndex = node->imax - newNodeImax;

        internal_node_t *newNode = new internal_node_t();
        node->imax -= newNodeImax;
        newNode->imax = newNodeImax;
        memcpy(newNode->kwds, node->kwds + newNodeStartIndex, newNodeImax * sizeof(kwd_t));
        memcpy(newNode->children, node->children + newNodeStartIndex, newNodeImax * sizeof(node_t *));
        for (int i = 0; i < newNode->imax; ++i)
        {
            newNode->children[i]->parent = newNode;
        }

        for (int i = 0; i < node->imax; ++i)
        {
            nodeSubTreeSize += node->kwds[i];
        }
        for (int i = 0; i < newNode->imax; ++i)
        {
            newNodeSubTreeSize += newNode->kwds[i];
        }
        resNode = (node_t *)node;
        resNewNode = (node_t *)newNode;
    }

    if (!node_->parent)
    {
        internal_node_t *root_ = new internal_node_t();
        root_->imax = 1;
        root_->children[0] = resNode;
        resNode->parent = root_;

        root = (node_t *)root_;
    }

    internal_node_t *parent = resNode->parent;
    int nodeIndex;
    for (nodeIndex = 0; nodeIndex < parent->imax; ++nodeIndex)
    {
        if (parent->children[nodeIndex] == resNode)
        {
            break;
        }
    }

    if (nodeIndex == parent->imax)
    {
        cout << "index out of bound";
        throw(exception());
    }

    parent->imax++;
    for (int i = parent->imax - 1; i > nodeIndex + 1; --i)
    {
        parent->kwds[i] = parent->kwds[i - 1];
        parent->children[i] = parent->children[i - 1];
    }

    parent->kwds[nodeIndex] = nodeSubTreeSize;

    parent->kwds[nodeIndex + 1] = newNodeSubTreeSize;
    parent->children[nodeIndex + 1] = resNewNode;
    resNewNode->parent = parent;

    if (parent->imax > MAX_NODE_SIZE)
    {
        rebalance_((node_t *)parent);
    }
}

cd_t fhbpt::getCode(pos_t pos)
{
    return getCode_(root, pos);
}

void fhbpt::insert(pos_t pos, ct_t ct)
{
    insert_(root, pos, ct);
}

#define PTR 0
#define STR 1

struct blk_t
{
    bool blkAttr;
    string s;
    node_t *child;
    blk_t(bool blkAttr_, string s_ = "", node_t *child_ = NULL)
    {
        blkAttr = blkAttr_;
        s = s_;
        child = child_;
    }
};
string fhbpt::traverse_()
{
    queue<blk_t *> q;
    if (root->node_attr == LEAF)
    {
        return ((leaf_t *)root)->toString();
    }
    internal_node_t *tmpRoot = (internal_node_t *)root;
    q.push(new blk_t(STR, tmpRoot->toString(), NULL));
    for (int i = 0; i < tmpRoot->imax; ++i)
    {
        q.push(new blk_t(PTR, "", tmpRoot->children[i]));
    }
    string s = "";
    while (!q.empty())
    {
        int qLen = q.size();
        for (int i = 0; i < qLen; ++i)
        {
            blk_t *blk = q.front();
            if (blk->blkAttr == STR)
            {
                s += blk->s + "\t";
            }
            else
            {
                if (blk->child->node_attr == LEAF)
                {
                    leaf_t *tmpNode = (leaf_t *)blk->child;
                    q.push(new blk_t(STR, tmpNode->toString(), NULL));
                }
                else
                {
                    internal_node_t *tmpNode = (internal_node_t *)blk->child;
                    q.push(new blk_t(STR, tmpNode->toString(), NULL));
                    for (int i = 0; i < tmpNode->imax; ++i)
                    {
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

struct traverseNode_t
{
    // bool nodeAttr;
    unsigned nodeId;
    node_t *node;
    unsigned *childrenId;

    traverseNode_t(unsigned nodeId_, node_t *node_, unsigned *childrenId_)
    {
        nodeId = nodeId_;
        node = node_;
        childrenId = childrenId_;
    }
};

string fhbpt::traverseGraphviz_()
{
    vector<traverseNode_t *> nodeList;
    vector<pair<int, int>> edgeList;
    unsigned id = 0;
    function<void(node_t *, unsigned)> dfs = [&](node_t *cur_, unsigned curId)
    {
        if (cur_->node_attr == INTERNAL)
        {
            internal_node_t *cur = (internal_node_t *)cur_;
            unsigned *childrenId = new unsigned[cur->imax];
            nodeList.push_back(new traverseNode_t(curId, cur_, childrenId));
            for (int i = 0; i < cur->imax; ++i)
            {
                unsigned tmpId = id++;
                childrenId[i] = tmpId;
                edgeList.push_back(make_pair(curId, tmpId));
                dfs(cur->children[i], tmpId);
            }
        }
        else
        {
            nodeList.push_back(new traverseNode_t(curId, cur_, NULL));
        }
    };
    dfs(root, id++);

    stringstream ss;
    ss << "digraph g {" << endl;
    ss << "node [shape = record,height=.1];" << endl;

    for (traverseNode_t *node : nodeList)
    {
        ss << "    ";
        ss << node->nodeId;
        ss << "[label = \"";
        if (node->node->node_attr == INTERNAL)
        {
            // 0[label = "<1>10|<2>20"];
            internal_node_t *inode = (internal_node_t *)node->node;
            for (int i = 0; i < inode->imax - 1; ++i)
            {
                ss << "<";
                ss << node->childrenId[i];
                ss << ">";
                ss << inode->kwds[i];
                ss << "|";
            }
            ss << "<";
            ss << node->childrenId[inode->imax - 1];
            ss << ">";
            ss << inode->kwds[inode->imax - 1];
        }
        else
        {
            // 1[label = "{ct1|1}|{ct2|3}|{ct3|5}"];
            // 2[label = "{ct4|9}|{ct5|10}|{ct6|11}"];
            leaf_t *lnode = (leaf_t *)node->node;
            for (int i = 0; i < lnode->imax - 1; ++i)
            {
                ss << "{";
                ss << lnode->cds[i];
                ss << "|";
                ss << lnode->cts[i];
                ss << "}|";
            }
            ss << "{";
            ss << lnode->cds[lnode->imax - 1];
            ss << "|";
            ss << lnode->cts[lnode->imax - 1];
            ss << "}";
        }
        ss << "\"];" << endl;
    }
    // "0":1 -> "1"
    // "0":2 -> "2"
    ss << endl;
    for (auto edge : edgeList)
    {
        ss << "    ";
        ss << "\"";
        ss << edge.first;
        ss << "\":";
        ss << edge.second;
        ss << " -> \"";
        ss << edge.second;
        ss << "\"" << endl;
    }
    ss << "}";
    return ss.str();
}

string fhbpt::traverse()
{
    // cout << traverse_() << endl;
    return traverseGraphviz_();
}