#include <iostream>
#include <string>

const int ORDER = 3; // B+树的阶数，这里假设为3

struct KeyValue {
    int key;
    std::string value;
};

// B+树节点类型
struct BPlusNode {
    bool is_leaf;
    int num_keys;
    KeyValue key_value[ORDER - 1];
    BPlusNode* children[ORDER];
    BPlusNode* next_leaf; // 指向下一个叶子节点的指针
};

class BPlusTree {
public:
    BPlusTree();
    ~BPlusTree();

    std::string search(int key);
    void insert(int key, const std::string& value);
    void remove(int key);
    void print();

private:
    BPlusNode* root; // B+树的根节点

    // 查找给定键的叶子节点
    BPlusNode* find_leaf_node(int key);

    // 从节点中查找给定键的索引位置
    int find_key_position(BPlusNode* node, int key);

    // 在叶子节点中插入键值对
    void insert_into_leaf(BPlusNode* leaf, int key, const std::string& value);

    // 在非叶子节点中插入索引项
    void insert_into_parent(BPlusNode* node, BPlusNode* new_child, int key);

    // 分裂叶子节点
    void split_leaf(BPlusNode* leaf);

    // 分裂非叶子节点
    void split_non_leaf(BPlusNode* node);

    // 从节点中删除给定键
    void remove_from_node(BPlusNode* node, int key);

    // 合并节点
    void merge_nodes(BPlusNode* left, BPlusNode* right);

    // 删除给定键的键值对
    void remove(int key, BPlusNode* node);
};

// 构造函数
BPlusTree::BPlusTree() {
    root = nullptr;
}

// 析构函数
BPlusTree::~BPlusTree() {
    // TODO: 释放树的所有节点内存
}

// 查找给定键的叶子节点
BPlusNode* BPlusTree::find_leaf_node(int key) {
    BPlusNode* current = root;
    while (!current->is_leaf) {
        int i = 0;
        while (i < current->num_keys && key >= current->key_value[i].key) {
            i++;
        }
        current = current->children[i];
    }
    return current;
}

// 从节点中查找给定键的索引位置
int BPlusTree::find_key_position(BPlusNode* node, int key) {
    int position = 0;
    while (position < node->num_keys && key >= node->key_value[position].key) {
        position++;
    }
    return position;
}

// 在叶子节点中插入键值对
void BPlusTree::insert_into_leaf(BPlusNode* leaf, int key, const std::string& value) {
    int position = find_key_position(leaf, key);
    for (int i = leaf->num_keys - 1; i >= position; i--) {
        leaf->key_value[i + 1] = leaf->key_value[i];
    }
    leaf->key_value[position] = {key, value};
    leaf->num_keys++;
}

// 在非叶子节点中插入索引项
void BPlusTree::insert_into_parent(BPlusNode* node, BPlusNode* new_child, int key) {
    int position = find_key_position(node, key);
    for (int i = node->num_keys - 1; i >= position; i--) {
        node->key_value[i + 1] = node->key_value[i];
    }
    node->key_value[position] = {key, ""};
    for (int i = node->num_keys; i >= position + 1; i--) {
        node->children[i + 1] = node->children[i];
    }
    node->children[position + 1] = new_child;
    node->num_keys++;
}

// 分裂叶子节点
void BPlusTree::split_leaf(BPlusNode* leaf) {
    BPlusNode* new_leaf = new BPlusNode;
    new_leaf->is_leaf = true;
    new_leaf->next_leaf = leaf->next_leaf;
    leaf->next_leaf = new_leaf;

    int split_point = (ORDER - 1) / 2;
    int j = 0;
    for (int i = split_point; i < ORDER - 1; i++) {
        new_leaf->key_value[j++] = leaf->key_value[i];
    }
    new_leaf->num_keys = j;
    leaf->num_keys = split_point;
}

// 分裂非叶子节点
void BPlusTree::split_non_leaf(BPlusNode* node) {
    BPlusNode* new_node = new BPlusNode;
    new_node->is_leaf = false;

    int split_point = ORDER / 2;
    int j = 0;
    for (int i = split_point + 1; i < ORDER; i++) {
        new_node->key_value[j++] = node->key_value[i];
        node->children[i]->is_leaf = false;
        new_node->children[j] = node->children[i];
    }
    node->num_keys = split_point;
    new_node->num_keys = j;
    new_node->children[j] = node->children[ORDER];

    BPlusNode* parent = node;
    node = new_node;
    int key = parent->key_value[split_point].key;
    insert_into_parent(parent, node, key);
}

// 插入键值对
void BPlusTree::insert(int key, const std::string& value) {
    if (root == nullptr) {
        root = new BPlusNode;
        root->is_leaf = true;
        root->num_keys = 1;
        root->key_value[0] = {key, value};
    } else {
        BPlusNode* leaf = find_leaf_node(key);
        if (leaf->num_keys < ORDER - 1) {
            insert_into_leaf(leaf, key, value);
        } else {
            split_leaf(leaf);
            insert_into_leaf(leaf->next_leaf, key, value);
        }
    }
}

// 删除给定键的键值对
void BPlusTree::remove(int key, BPlusNode* node) {
    if (node->is_leaf) {
        remove_from_leaf(node, key);
    } else {
        int position = find_key_position(node, key);
        BPlusNode* child = node->children[position];
        if (child->num_keys == ORDER - 1) {
            // TODO: 实现从非叶子节点借或合并键值对的逻辑
        }
        remove(key, child);
    }
}

// 从节点中删除给定键
void BPlusTree::remove_from_node(BPlusNode* node, int key) {
    int position = find_key_position(node, key);
    for (int i = position; i < node->num_keys - 1; i++) {
        node->key_value[i] = node->key_value[i + 1];
    }
    node->num_keys--;
}

// 合并节点
void BPlusTree::merge_nodes(BPlusNode* left, BPlusNode* right) {
    int position = find_key_position(left->children[0], right->key_value[0].key);
    left->key_value[left->num_keys] = right->key_value[0];
    left->num_keys++;
    for (int i = 0; i < right->num_keys - 1; i++) {
        left->key_value[left->num_keys + i] = right->key_value[i + 1];
    }
    left->num_keys += right->num_keys - 1;
    left->children[left->num_keys] = right->children[0];
    for (int i = 0; i < right->num_keys; i++) {
        right->children[i] = right->children[i + 1];
    }
    delete right;
}

// 删除给定键的键值对
void BPlusTree::remove(int key) {
    if (root == nullptr) {
        return;
    }

    if (root->is_leaf) {
        remove_from_leaf(root, key);
    } else {
        int position = find_key_position(root, key);
        BPlusNode* child = root->children[position];
        if (child->num_keys == ORDER - 1) {
            // TODO: 实现从非叶子节点借或合并键值对的逻辑
        }
        remove(key, child);
    }
}

// 打印树的内容（层次遍历）
void BPlusTree::print() {
    if (root == nullptr) {
        return;
    }

    std::cout << "B+ Tree Content:" << std::endl;

    std::queue<BPlusNode*> q;
    q.push(root);

    while (!q.empty()) {
        BPlusNode* node = q.front();
        q.pop();

        for (int i = 0; i < node->num_keys; i++) {
            std::cout << node->key_value[i].key << " ";
        }
        std::cout << "| ";

        if (!node->is_leaf) {
            for (int i = 0; i <= node->num_keys; i++) {
                q.push(node->children[i]);
            }
        }
    }

    std::cout << std::endl;
}

int main() {
    BPlusTree btree;

    // 插入键值对
    btree.insert(10, "Value1");
    btree.insert(20, "Value2");
    btree.insert(15, "Value3");
    btree.insert(30, "Value4");
    btree.insert(25, "Value5");
    btree.insert(5, "Value6");

    // 打印树的内容
    btree.print();

    // 查找键值对
    std::cout << "Search key 15: " << btree.search(15) << std::endl;
    std::cout << "Search key 50: " << btree.search(50) << std::endl;

    // 删除键值对
    btree.remove(20);

    // 打印树的内容
    btree.print();

    return 0;
}
