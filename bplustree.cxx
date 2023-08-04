#include <string>
#include <iostream>
#include <vector>


// 名字声明
using std::vector;
using std::string;
using std::cout;
using std::endl;

// B+树基本属性
constexpr int leaf_max_degree = 3;
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
    // 获取索引对应的value值
    value_type getValue(int index){return values[index];}

    // 获取节点大小，即存放的键的数目
    int getSize(){return size;}

    // 获取孩子指针
    BPlusNode *getChild(int index){ return children[index];}


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
    bool searchKeyValue(const key_type &key, value_type &value);

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

    // 按节点打印
    void print(BPlusNode *root){
        if(root == nullptr){
            cout << "The tree is empty!" << endl;
            return;
        }

        if(root->leaf)
            cout << "leaf: ";
        for(auto node : root->keys)
            cout << node << " ";
        cout << endl;

        if(!root->leaf)
            for(auto node : root->children)
                print(node);
    }

private:
    BPlusNode *root = nullptr;
};


bool BPlusTree::searchKeyValue(const key_type &key, value_type &value)
{
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

int main()
{
    BPlusTree bpt;
    bpt.insertKeyValue(3, "3");
    bpt.insertKeyValue(5, "5");
    bpt.insertKeyValue(12, "12");
    bpt.insertKeyValue(7, "7");
    bpt.insertKeyValue(8, "8");
    bpt.insertKeyValue(31, "31");
    bpt.insertKeyValue(13, "130");
    bpt.insertKeyValue(22, "22");
    bpt.insertKeyValue(10, "10");
    bpt.print(bpt.getRoot());
    
    value_type v;
    if(bpt.searchKeyValue(15, v))
        cout << "Value is" << v << endl;
    else
        cout << "not here" << endl;

    if(bpt.searchKeyValue(13, v))
        cout << "Value is " << v  << endl;
    else
        cout << "not here" << endl;

    return 0;
}
