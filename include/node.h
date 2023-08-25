#ifndef __NODE_H__
#define __NODE_H__

#include "utils.h"
//#include "tree.h"

class BPlusNode{
    friend class BPlusTree;

private:
    bool leaf;
    int size;
    vector<key_type> keys;
    vector<value_type> values;
    vector<BPlusNode*> children;
    BPlusNode * next_leaf = nullptr;

public:
    BPlusNode();
    BPlusNode(bool leaf, int size);
    BPlusNode(bool leaf, int size, vector<key_type> &keys_vec, vector<BPlusNode*> &children_vec);
    BPlusNode(bool leaf, int size, vector<key_type> &keys_vec, vector<value_type> &value_vec);
    ~ BPlusNode();

    static int cmpKeys(const key_type &key1, const key_type &key2);

    bool isLeaf();
    key_type getKey(int index);
    vector<key_type> & getKeys();
    value_type getValue(int index);
    int getSize();
    BPlusNode *getChild(int index);
    
    void setValue(int index, const value_type &v);
};




#endif
