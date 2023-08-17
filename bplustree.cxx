#include <string>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <queue>
#include <algorithm>
#include <random>

// 名字声明
using std::vector;
using std::string;
using std::cout;
using std::endl;

// B+树基本属性
constexpr int leaf_max_degree = 4;
constexpr int leaf_min_degree = (leaf_max_degree+1)/2;
constexpr int nonleaf_max_degree = leaf_max_degree;
constexpr int nonleaf_min_degree = (nonleaf_max_degree+1)/2;


// 定义键值的类型
typedef int key_type;
typedef std::string value_type;


// 键值的比较
int cmpKeys(const key_type &key1, const key_type &key2)
{
    if(key1 < key2)
        return -1;
    if(key1 == key2)
        return 0;
    return 1;
}



// 树节点定义
class BPlusNode{
    // 声明友元，让树可以直接访问节点的属性
    friend class BPlusTree;

public:
    BPlusNode(bool leaf, int size): leaf(leaf), size(size){}
    BPlusNode(bool leaf, int size, vector<key_type> &keys_vec, vector<BPlusNode*> &children_vec): 
            leaf(leaf), size(size), keys(keys_vec), children(children_vec){}
    BPlusNode(bool leaf, int size, vector<key_type> &keys_vec, vector<value_type> &value_vec): 
            leaf(leaf), size(size), keys(keys_vec), values(value_vec){}

    // 查看是否是叶节点
    bool isLeaf(){return leaf;}    

    // 获取索引对应的key值
    key_type getKey(int index){return keys[index];}
    // 获取节点中的key列表
    vector<key_type> & getKeys(){ return keys;}    
    // 获取索引对应的value值
    value_type getValue(int index){return values[index];}

    // 获取节点大小，即存放的键的数目
    int getSize(){return size;}

    // 获取孩子指针
    BPlusNode *getChild(int index){ return children[index];}

    void setValue(int index, const value_type &v){
        values[index] = v;
    }

private:
    bool leaf;
    int size;
    vector<key_type> keys;
    vector<value_type> values;
    vector<BPlusNode*> children;
    BPlusNode * next_leaf = nullptr;

};


// B+树的定义
class BPlusTree{
public:
    BPlusTree() = default;

    // 获取根结点
    BPlusNode *getRoot(){ return root; }

    // 搜索key，并通过引用传递返回对应的value值，函数返回值表示是否成功查找到
    bool searchKeyValue(const key_type &key, value_type &value){
        if(this->getRoot() == nullptr){
            std::cerr << "Error: search failed: tree is empty!" << endl;
            return false;
        }

        // 确定key所在的节点
        BPlusNode *p = root;
        while(!p->isLeaf()){
            int i = 0;
            for(; i < p->getSize() && cmpKeys(key, p->getKey(i)) >= 0; i++);
            p = p->getChild(i);
        }

        // 确定key在节点中的索引
        int j = 0;
        for(; j < p->getSize() && p->getKey(j) != key; j++);

        // 确定对应的value值
        if(j == p->getSize()){
            std::cerr << "Error: search failed: key '" << key << "' desn't exist!" << endl;
            return false;
        }
        value = p->getValue(j);
        return true;
    }

    // 在节点中找到key对应的位置
    int find_key_index(BPlusNode *node, const key_type &key){
        int j = 0;
        for(; j < node->getSize() && cmpKeys(key, node->getKey(j)) > 0; j++);
        return j;
    }

    // 插入数据到叶节点
    void insert_into_leaf(BPlusNode *leaf, const key_type &key, const value_type &value){
        int index = find_key_index(leaf, key);
        leaf->keys.insert(leaf->keys.begin()+index, key);
        leaf->values.insert(leaf->values.begin()+index, value);
        leaf->size++;
    }

    // 插入数据到内部节点
    void insert_into_nonleaf(BPlusNode *node, const key_type &key, BPlusNode *child){
        if(node == nullptr){    // 父内部节点为空，即刚才分裂的是根节点
            BPlusNode* new_root = new BPlusNode(false, 1);
            new_root->keys.push_back(key);
            new_root->children.push_back(root);
            new_root->children.push_back(child);
            root = new_root;
        }
        else{
            int index = find_key_index(node, key);
            node->keys.insert(node->keys.begin()+index, key);
            node->children.insert(node->children.begin()+index+1, child);
            node->size++;            
        }

    }

    // 分裂叶节点，返回值为新叶子中最小key值
    key_type split_leaf(BPlusNode *leaf){
        // 确定分裂点，数值上等于旧节点中保留的key数目
        int split_point = leaf_max_degree/2;    

        // 创建新叶子
        vector<key_type> tail_keys(leaf->keys.begin()+split_point, leaf->keys.end());
        vector<value_type> tail_values(leaf->values.begin()+split_point, leaf->values.end());
        BPlusNode *new_leaf = new BPlusNode(true, leaf_max_degree - split_point, tail_keys, tail_values);
        // 链上新叶子
        new_leaf->next_leaf = leaf->next_leaf;
        leaf->next_leaf = new_leaf;
        // 更新旧叶子
        leaf->keys.resize(split_point);
        leaf->values.resize(split_point);
        leaf->size = split_point;

        return new_leaf->keys[0];
    }

    // 分裂内部节点
    key_type split_nonleaf(BPlusNode *node, BPlusNode *&new_nonleaf){
        int split_point = nonleaf_max_degree/2;
        key_type split_key = node->keys[split_point];

        // 创建新节点
        vector<key_type> tail_keys(node->keys.begin()+split_point+1, node->keys.end());
        vector<BPlusNode*> tail_children(node->children.begin()+split_point+1, node->children.end());
        BPlusNode *new_node = new BPlusNode(false, leaf_max_degree-split_point-1, tail_keys, tail_children);
        // 更新旧节点
        node->keys.resize(split_point);
        node->children.resize(split_point+1);
        node->size = split_point;
    
        new_nonleaf = new_node;
        return split_key;
    }


    // 插入键值对
    bool insertKeyValue(const key_type &key, const value_type &value){
        // 树为空
        if(getRoot() == nullptr){
            root = new BPlusNode(true, 1);
            root->keys.push_back(key);
            root->values.push_back(value);
            return true;
        }

        // 树非空
        vector<BPlusNode*> path;
        BPlusNode *p = root;

        // 找到该插入的叶节点
        while(!p->isLeaf()){
            path.push_back(p);  // 把内部节点加入搜索路径
            int i = 0;
            for(; i < p->getSize() && cmpKeys(key, p->getKey(i)) >= 0; i++);
            p = p->getChild(i);
        }

        // 插入键值对到叶节点
        insert_into_leaf(p, key, value);
        // 节点溢出，需要分裂
        if(p->size == leaf_max_degree){
            BPlusNode *current_node = p, *new_node;
            BPlusNode *parent = nullptr;
            key_type split_key;

            // 分裂叶节点
            split_key = split_leaf(current_node);
            new_node = current_node->next_leaf;
            if(path.empty()) {  // 叶节点是根节点
                insert_into_nonleaf(nullptr, split_key, current_node->next_leaf);
                return true;
            }
            else{   // 叶节点不是根节点
                parent = path.back();
                path.pop_back();
                insert_into_nonleaf(parent, split_key, current_node->next_leaf);
                current_node = parent;
            }

            // 往上分裂内部节点
            while(current_node->size == nonleaf_max_degree){
                split_key = split_nonleaf(current_node, new_node);
                if(path.empty()) { // 路径为空，已经向上分裂到根结点
                    insert_into_nonleaf(nullptr, split_key, new_node);
                    break;
                }     
                else{
                    parent = path.back();
                    path.pop_back();
                    insert_into_nonleaf(parent, split_key, new_node);
                    current_node = parent;
                }
            }            
        }

        return true;
    }

    bool modifyKeyValue(const key_type &key, const value_type &value){
        if(this->getRoot() == nullptr){
            std::cerr << "Error: search failed: tree is empty!" << endl;
            return false;
        }

        // 确定key所在的节点
        BPlusNode *p = root;
        while(!p->isLeaf()){
            int i = 0;
            for(; i < p->getSize() && cmpKeys(key, p->getKey(i)) >= 0; i++);
            p = p->getChild(i);
        }

        // 确定key在节点中的索引
        int j = 0;
        for(; j < p->getSize() && p->getKey(j) != key; j++);

        if(j == p->getSize()){
            std::cerr << "Error: search failed: key '" << key << "' desn't exist!" << endl;
            return false;
        }
        p->setValue(j, value);
        return true;
    }

// 判断节点是第几个孩子
int child_index(BPlusNode *parent, BPlusNode *node)
{
    int i = 0;
    for(; i < parent->size+1 && node != parent->children[i]; i++);
    if(i < parent->size+1)
        return i;
    else    // node不是parent的孩子
        return -1;
}

// 往上改索引
void change_index(BPlusNode *current_node, vector<BPlusNode*> path)
{
    key_type key = current_node->keys[0];
    BPlusNode *parent = path.back();
    path.pop_back();
    int cindex = child_index(parent, current_node);
    while(parent != root && cindex == 0){
        current_node = parent;
        parent = path.back();
        path.pop_back();
        cindex = child_index(parent, current_node);
    }
    if(cindex > 0){ // 往上追溯到了根节点则不需要改索引，叶子是最左下叶子，否则更新索引
        parent->keys[cindex-1] = key;
    }
}

// 删除键值对
bool deleteKeyValue(const key_type &key) {
    if (getRoot() == nullptr) {
        std::cerr << "Error: delete failed: tree is empty!" << endl;
        return false;
    }

    // 查找要删除键所在的叶节点
    BPlusNode *current_node = root;
    //BPlusNode *parent = nullptr;
    vector<BPlusNode*> path;
    while (!current_node->isLeaf()) {
        int i = 0;
        for (; i < current_node->getSize() && cmpKeys(key, current_node->getKey(i)) >= 0; i++);
        path.push_back(current_node);
        //parent = current_node;
        current_node = current_node->getChild(i);
    }

    // 在叶节点中查找要删除的键的位置
    int index = 0;
    for (; index < current_node->getSize() && current_node->getKey(index) != key; index++);

    if (index == current_node->getSize()) {
        std::cerr << "Error: delete failed: key '" << key << "' doesn't exist!" << endl;
        return false;
    }

    // 从叶节点中删除键值对
    current_node->keys.erase(current_node->keys.begin() + index);
    current_node->values.erase(current_node->values.begin() + index);
    current_node->size--;

    ////// 要判断叶节点是否是根节点且被删空了
    if(current_node == getRoot()){
        if(current_node->getSize() == 0)
            root = nullptr;
    }
    ////// 不是根结点，要判断是否是最小key，需要改索引
    else{
        //cout << "判断是否是最小key，需要改索引" << endl;
        if(index == 0 ){ // 是最小key
            cout << "需要改索引" << endl;
            if(current_node->size > 0)
                change_index(current_node, path);
            //else   
                //cout << "当前叶节点删空了，暂时改不了索引" << endl;
        }
        // 触发下溢，调整叶节点
        if (current_node->size < leaf_min_degree-1) {
            adjustLeafNode(current_node, path);
        }
    } 

    return true;
}

// 调整叶节点
void adjustLeafNode(BPlusNode *node, vector<BPlusNode*> path) {
    BPlusNode *parent = path.back();
    //path.pop_back();
    // 找到 node 在 parent 中的索引
    int index = child_index(parent, node);

    // 尝试从左兄弟节点中借一个键值对
    if (index > 0 && parent->getChild(index - 1)->getSize() > leaf_min_degree-1) {
        BPlusNode *left_sibling = parent->getChild(index - 1);
        node->keys.insert(node->keys.begin(), left_sibling->getKey(left_sibling->getSize() - 1));
        node->values.insert(node->values.begin(), left_sibling->getValue(left_sibling->getSize() - 1));
        node->size++;   /////
        left_sibling->keys.pop_back();
        left_sibling->values.pop_back();
        left_sibling->size--;
        parent->keys[index - 1] = node->getKey(0);  // 更新索引
        //cout << "从左兄弟叶子中借key，更新索引" << endl;
    }
    // 尝试从右兄弟节点中借一个键值对
    else if (index < parent->getSize() && parent->getChild(index + 1)->getSize() > leaf_min_degree-1) {
        BPlusNode *right_sibling = parent->getChild(index + 1);
        node->keys.push_back(right_sibling->getKey(0));
        node->values.push_back(right_sibling->getValue(0));
        node->size++;   /////
        right_sibling->keys.erase(right_sibling->keys.begin());
        right_sibling->values.erase(right_sibling->values.begin());
        right_sibling->size--;
        parent->keys[index] = right_sibling->getKey(0); // 更新右兄弟索引
        //cout << "从右兄弟叶子中借key，更新右兄弟叶子的索引" << endl;
        //// 若借之前，节点为空，则还需更新当前节点的索引
        if(node->size == 1){
            change_index(node, path);
            //cout << "借之前为空，还需更新当前叶子的索引" << endl;
        }
            
    }
    // 无法借键值对，则考虑合并
    else {
        if (index > 0) {   ////// 有左兄弟
            //cout << "向左合并" << endl;
            BPlusNode *left_sibling = parent->getChild(index - 1);
            left_sibling->keys.insert(left_sibling->keys.end(), node->keys.begin(), node->keys.end());
            left_sibling->values.insert(left_sibling->values.end(), node->values.begin(), node->values.end());
            left_sibling->size += node->size;
            left_sibling->next_leaf = node->next_leaf;
            parent->keys.erase(parent->keys.begin() + index - 1);
            parent->children.erase(parent->children.begin() + index);
            parent->size--;
            delete node;
        } else {       ///// 没有左兄弟
            //cout << "向右合并" << endl;
            BPlusNode *right_sibling = parent->getChild(index + 1);
            node->keys.insert(node->keys.end(), right_sibling->keys.begin(), right_sibling->keys.end());
            node->values.insert(node->values.end(), right_sibling->values.begin(), right_sibling->values.end());
            node->size += right_sibling->size;
            node->next_leaf = right_sibling->next_leaf;
            parent->keys.erase(parent->keys.begin() + index);
            parent->children.erase(parent->children.begin() + index + 1);
            parent->size--;
            delete right_sibling;
            if(node->size == 1){
                //cout << "右边合并过来之前为空，则还需更新当前节点的索引" << endl;
                change_index(node, path);
            }
        }

        ////// parent是根节点，且被删空：更新根结点
        if(parent == root){
            if(parent->size == 0)
                root = node;
        }
        ///// parent不是根结点，且触发内部节点下溢
        else if (parent->size < nonleaf_min_degree-1) {
            //cout << "内部节点下溢，需要调整内部节点" << endl;
            adjustNonLeafNode(path);
        }
    }
}

// 调整内部节点
void adjustNonLeafNode(vector<BPlusNode*> path) {
    BPlusNode *node = path.back();  // 发生下溢的内部节点
    path.pop_back();
    BPlusNode *parent = path.back();    // 内部节点的父节点
    int index = child_index(parent, node);
    //cout << "内部节点index: " << index << endl;
    ///// for (; index < parent->getSize() + 1 && parent->getChild(index) != node; index++);

    // 尝试从左兄弟节点中借一个键 ///// 借键实际上借的是孩子和键的位置
    if (index > 0 && parent->getChild(index - 1)->getSize() > nonleaf_min_degree-1) {
        //cout << "从左兄弟借键" << endl;
        BPlusNode *left_sibling = parent->getChild(index - 1);
        ////// 借来的键的位置中要放的索引值是：借之前以该节点最左子树的最小值，可以在上一层索引找到
        node->keys.insert(node->keys.begin(), parent->getKey(index - 1));
        ////// 节点的上一层索引改为借来的孩子中的最小值，可在借的key的旧值找到
        parent->keys[index - 1] = left_sibling->getKey(left_sibling->getSize() - 1);
        left_sibling->keys.pop_back();
        node->children.insert(node->children.begin(), left_sibling->getChild(left_sibling->getSize()));
        left_sibling->children.pop_back();
        left_sibling->size--;
        node->size++; //////
    }
    // 尝试从右兄弟节点中借一个键
    else if (index < parent->getSize() && parent->getChild(index + 1)->getSize() > nonleaf_min_degree-1) {
        //cout << "从右兄弟借键" << endl;
        BPlusNode *right_sibling = parent->getChild(index + 1);
        ////// 同上：借过来的索引要改（改成借来的孩子中的最小值），右兄弟上一层索引也要改（改成右兄弟借出的key值）
        node->keys.push_back(parent->getKey(index));
        parent->keys[index] = right_sibling->getKey(0);
        right_sibling->keys.erase(right_sibling->keys.begin());
        node->children.push_back(right_sibling->getChild(0));
        right_sibling->children.erase(right_sibling->children.begin());
        right_sibling->size--;
        node->size++; /////
    }
    // 无法借键，则考虑合并
    else {
        //cout << "需要合并" << endl;
        if (index > 0) {
            BPlusNode *left_sibling = parent->getChild(index - 1);
            left_sibling->keys.push_back(parent->getKey(index - 1));
            left_sibling->keys.insert(left_sibling->keys.end(), node->keys.begin(), node->keys.end());
            left_sibling->children.insert(left_sibling->children.end(), node->children.begin(), node->children.end());
            left_sibling->size += node->size + 1;
            parent->keys.erase(parent->keys.begin() + index - 1);
            parent->children.erase(parent->children.begin() + index);
            parent->size--;
            delete node;
            node = left_sibling;    ///// 更新node为合并后的节点
        } else {
            BPlusNode *right_sibling = parent->getChild(index + 1);
            node->keys.push_back(parent->getKey(index));
            node->keys.insert(node->keys.end(), right_sibling->keys.begin(), right_sibling->keys.end());
            node->children.insert(node->children.end(), right_sibling->children.begin(), right_sibling->children.end());
            node->size += right_sibling->size + 1;
            parent->keys.erase(parent->keys.begin() + index);
            parent->children.erase(parent->children.begin() + index + 1);
            parent->size--;
            delete right_sibling;
        }

        ////// 判断parent是不是根节点，且被删空：更新根结点，这时树的层数-1
        if(parent == root){
            if(parent->size == 0)
                root = node;
        }
        // parent不是根结点，根据是否下溢决定是否递归调整内部节点
        else if (parent->size < nonleaf_min_degree-1) {
            adjustNonLeafNode(path);
        }
    }
}

// 查找节点的父节点
BPlusNode *findParent(BPlusNode *current_node, BPlusNode *node) {
    if (current_node->isLeaf() || current_node->getChild(0) == node) {
        return nullptr;
    }

    int i = 0;
    for (; i < current_node->getSize() + 1 && current_node->getChild(i) != node; i++);

    if (i <= current_node->getSize()) {
        BPlusNode *parent = findParent(current_node->getChild(i), node);
        if (parent != nullptr) {
            return parent;
        }
    }
    return current_node;
}

private:
    BPlusNode *root = nullptr;
};



// 层次遍历打印
void printBPT(BPlusNode* root)
{
    if(root == nullptr){
        cout << "Empty tree!" << endl;
        return;
    }

    std::queue<BPlusNode*> Q;
    BPlusNode *p = nullptr;
    Q.push(root);

    cout << "B+ Tree Content: " << endl;
    while(!Q.empty()){
        p = Q.front();
        Q.pop();

        for(auto key: p->getKeys())
            cout << key << " ";
        if(p->isLeaf())
            cout << "  |  " ;
        else{
            for(int i = 0; i < p->getSize()+1; i++)
                Q.push(p->getChild(i));
            cout << endl;
        }
    }
    cout << '\n' << endl;
}


bool test_insertion()
{
    std::vector<int> sequence;
    for (int i = 1; i <= 1000; ++i) {
        sequence.push_back(i);
    }

    // 使用随机数引擎和洗牌函数来随机排序序列
    std::random_device rd;
    std::mt19937 rng(rd());  // 随机数引擎
    std::shuffle(sequence.begin(), sequence.end(), rng);

    // 插入随机数生成B+树
    BPlusTree bpt;
    for(auto key : sequence)
        bpt.insertKeyValue(key, "000");
}

int main(int argc, char **argv)
{
    BPlusTree bpt;
    bpt.insertKeyValue(3, "30");
    bpt.insertKeyValue(5, "50");
    bpt.insertKeyValue(12, "120");
    bpt.insertKeyValue(7, "70");
    bpt.insertKeyValue(8, "80");

    bpt.insertKeyValue(31, "310");
    bpt.insertKeyValue(13, "130");
    bpt.insertKeyValue(22, "220");
    bpt.insertKeyValue(10, "100");
    bpt.insertKeyValue(19, "190");

    bpt.insertKeyValue(51, "510");
    bpt.insertKeyValue(17, "170");
    bpt.insertKeyValue(83, "830");

    //printBPT(bpt.getRoot());
    

    int opt;
    value_type v = "";
    while((opt = getopt(argc, argv, ":i:s:d:m:p")) != -1){
        switch(opt){        
            case 'i':
                bpt.insertKeyValue(std::stoi(optarg), argv[optind++]);
                break;
            case 's':
                if(bpt.searchKeyValue(std::stoi(optarg), v))
                    cout << "Value of key '" << optarg << "' is " << v << endl;
                break;
            case 'd':
                if(bpt.deleteKeyValue(std::stoi(optarg)))
                    cout << "Deletion done! (" << optarg << ")" << endl;
                else
                    cout << "Deletion failed! (" << optarg << ")" << endl;
                break;
            case 'p':
                printBPT(bpt.getRoot());
                break;
            case 'm':
                if(bpt.modifyKeyValue(std::stoi(optarg), argv[optind++]))
                    cout << "Modification done!" << endl;
                else
                    cout << "Modification failed" << endl;
                break;
            case ':':
                cout << "Some arguments missing!" << endl;
                break;
            case '?':
                cout << "Unknown option!" << endl;
                break;
        }
    }

    return 0;
}
