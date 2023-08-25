#ifndef __TREE_H__
#define __TREE_H__

#include "utils.h"
#include "node.h"

class BPlusTree{
private:
    int leaf_max_degree = 4;
    int leaf_min_degree = (leaf_max_degree+1)/2;
    int nonleaf_max_degree = leaf_max_degree;
    int nonleaf_min_degree = (nonleaf_max_degree+1)/2;

    BPlusNode *root = nullptr;

    string data_file;
    std::ifstream from_file;
    std::ofstream to_file;

    void serializeNodeToFile(BPlusNode* node);
    BPlusNode* deserializeNodeFromFile();

public:
    BPlusTree() = default;
    BPlusTree(int degree);
    ~BPlusTree();
    bool set_degree(int degree);
    int getDegree();


    /************** 封装 ***************/
    BPlusNode *getRoot();

    /************** 查询与修改 ***************/
    bool searchKeyValue(const key_type &key, value_type &value);
    bool modifyKeyValue(const key_type &key, const value_type &value);

    /************** 插入 ***************/
    int find_key_index(BPlusNode *node, const key_type &key);
    void insert_into_leaf(BPlusNode *leaf, const key_type &key, const value_type &value);
    void insert_into_nonleaf(BPlusNode *node, const key_type &key, BPlusNode *child);
    key_type split_leaf(BPlusNode *leaf);
    key_type split_nonleaf(BPlusNode *node, BPlusNode *&new_nonleaf);
    bool insertKeyValue(const key_type &key, const value_type &value);

    /************** 删除 ***************/
    bool deleteKeyValue(const key_type &key);
    void adjustLeafNode(BPlusNode *node, vector<BPlusNode*> path);
    void adjustNonLeafNode(vector<BPlusNode*> path);
    int child_index(BPlusNode *parent, BPlusNode *node);
    void change_index(BPlusNode *current_node, vector<BPlusNode*> path);

    void build_tree_from(string file_name);
    void save_to_file();
    void clear_tree();
    bool is_bplustree();
    void verify();

};

void printBPT(BPlusNode* root);

#endif
