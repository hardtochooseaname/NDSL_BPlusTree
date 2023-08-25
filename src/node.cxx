#include "node.h"
    
BPlusNode::BPlusNode(){}
BPlusNode::BPlusNode(bool leaf, int size): leaf(leaf), size(size){}
BPlusNode::BPlusNode(bool leaf, int size, vector<key_type> &keys_vec, vector<BPlusNode*> &children_vec): 
        leaf(leaf), size(size), keys(keys_vec), children(children_vec){}
BPlusNode::BPlusNode(bool leaf, int size, vector<key_type> &keys_vec, vector<value_type> &value_vec): 
        leaf(leaf), size(size), keys(keys_vec), values(value_vec){}

BPlusNode::~BPlusNode(){}


// 键值的比较
int BPlusNode::cmpKeys(const key_type &key1, const key_type &key2)
{
    if(key1 < key2)
        return -1;
    if(key1 == key2)
        return 0;
    return 1;
}

// 查看是否是叶节点
bool BPlusNode::isLeaf(){
    return leaf;
}    

// 获取索引对应的key值
key_type BPlusNode::getKey(int index){
    return keys[index];
}

// 获取节点中的key列表
vector<key_type> & BPlusNode::getKeys(){ 
    return keys;
}    

// 获取索引对应的value值
value_type BPlusNode::getValue(int index){
    return values[index];
}

// 获取节点大小，即存放的键的数目
int BPlusNode::getSize(){
    return size;
}

// 获取孩子指针
BPlusNode *BPlusNode::getChild(int index){ 
    return children[index];
}

void BPlusNode::setValue(int index, const value_type &v){
    values[index] = v;
}