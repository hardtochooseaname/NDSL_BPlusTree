#include "tree.h"

BPlusTree::BPlusTree(int degree)
{
    root = nullptr;
    set_degree(degree);
}

BPlusTree::~BPlusTree()
{
    clear_tree();
}

int BPlusTree::getDegree()
{
    return leaf_max_degree;
}


// 获取根结点
BPlusNode *BPlusTree::getRoot(){ 
    return root; 
}

// 修改B+树度数
bool BPlusTree::set_degree(int degree)
{
    if(!root){
        leaf_max_degree = degree;
        leaf_min_degree = (leaf_max_degree+1)/2;
        nonleaf_max_degree = leaf_max_degree;
        nonleaf_min_degree = (nonleaf_max_degree+1)/2;  
        cout << "Successfully changed degree into: " << degree <<  endl;
        return true;     
    }
    else{
        cout << "Failed to change degree: The tree is not empty!" << endl;
        return false;
    }

}

/*******************    查找     *********************/
// 搜索key，并通过引用传递返回对应的value值，函数返回值表示是否成功查找到
bool BPlusTree::searchKeyValue(const key_type &key, value_type &value){
    if(this->getRoot() == nullptr){
        std::cerr << "Error: search failed: tree is empty!" << endl;
        return false;
    }

    // 确定key所在的节点
    BPlusNode *p = root;
    while(!p->isLeaf()){
        int i = 0;
        for(; i < p->getSize() && BPlusNode::cmpKeys(key, p->getKey(i)) >= 0; i++);
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
int BPlusTree::find_key_index(BPlusNode *node, const key_type &key){
    int j = 0;
    for(; j < node->getSize() && BPlusNode::cmpKeys(key, node->getKey(j)) > 0; j++);
    return j;
}

/*******************    插入     *********************/
// 插入数据到叶节点
void BPlusTree::insert_into_leaf(BPlusNode *leaf, const key_type &key, const value_type &value){
    int index = find_key_index(leaf, key);
    leaf->keys.insert(leaf->keys.begin()+index, key);
    leaf->values.insert(leaf->values.begin()+index, value);
    leaf->size++;
}

// 插入数据到内部节点
void BPlusTree::insert_into_nonleaf(BPlusNode *node, const key_type &key, BPlusNode *child){
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
key_type BPlusTree::split_leaf(BPlusNode *leaf){
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
key_type BPlusTree::split_nonleaf(BPlusNode *node, BPlusNode *&new_nonleaf){
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
bool BPlusTree::insertKeyValue(const key_type &key, const value_type &value){
    // 树为空：创建节点作为根节点
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
        for(; i < p->getSize() && BPlusNode::cmpKeys(key, p->getKey(i)) >= 0; i++);
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

/*******************    修改     *********************/
bool BPlusTree::modifyKeyValue(const key_type &key, const value_type &value){
    if(this->getRoot() == nullptr){
        std::cerr << "Error: search failed: tree is empty!" << endl;
        return false;
    }

    // 确定key所在的节点
    BPlusNode *p = root;
    while(!p->isLeaf()){
        int i = 0;
        for(; i < p->getSize() && BPlusNode::cmpKeys(key, p->getKey(i)) >= 0; i++);
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



/*******************    删除     *********************/
// 删除键值对
bool BPlusTree::deleteKeyValue(const key_type &key) {
    if (getRoot() == nullptr) {
        std::cerr << "Error: delete failed: tree is empty!" << endl;
        return false;
    }

    // 查找要删除键所在的叶节点
    BPlusNode *current_node = root;
    vector<BPlusNode*> path;    // 记录查找路径，便于之后查找父节点
    while (!current_node->isLeaf()) {
        int i = 0;
        for (; i < current_node->getSize() && BPlusNode::cmpKeys(key, current_node->getKey(i)) >= 0; i++);
        path.push_back(current_node);
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

    // 要判断叶节点是否是根节点且被删空了
    if(current_node == getRoot()){
        if(current_node->getSize() == 0)
            root = nullptr;
    }
    // 不是根结点，判断是否是最小key，需要改索引，先改索引再判断是否需要调整
    else{
        if(index == 0 ){ // 是最小key
            // 当前节点没有删空，才有最小的key作为当前节点新的索引，否则则需要等到借或合并后才能获取新的索引值
            if(current_node->size > 0) 
                change_index(current_node, path);
        }
        // 触发下溢，调整叶节点
        if (current_node->size < leaf_min_degree-1) {
            adjustLeafNode(current_node, path);
        }
    } 

    return true;
}

// 调整叶节点
void BPlusTree::adjustLeafNode(BPlusNode *node, vector<BPlusNode*> path) {
    BPlusNode *parent = path.back();
    int index = child_index(parent, node);

    /********* 首先尝试：借 ********/
    // 尝试从左兄弟节点中借一个键值对
    if (index > 0 && parent->getChild(index - 1)->getSize() > leaf_min_degree-1) {
        BPlusNode *left_sibling = parent->getChild(index - 1);
        node->keys.insert(node->keys.begin(), left_sibling->getKey(left_sibling->getSize() - 1));
        node->values.insert(node->values.begin(), left_sibling->getValue(left_sibling->getSize() - 1));
        node->size++;  
        left_sibling->keys.pop_back();
        left_sibling->values.pop_back();
        left_sibling->size--;
        parent->keys[index - 1] = node->getKey(0);  // 更新索引
    }
    // 尝试从右兄弟节点中借一个键值对
    else if (index < parent->getSize() && parent->getChild(index + 1)->getSize() > leaf_min_degree-1) {
        BPlusNode *right_sibling = parent->getChild(index + 1);
        node->keys.push_back(right_sibling->getKey(0));
        node->values.push_back(right_sibling->getValue(0));
        node->size++;   
        right_sibling->keys.erase(right_sibling->keys.begin());
        right_sibling->values.erase(right_sibling->values.begin());
        right_sibling->size--;
        parent->keys[index] = right_sibling->getKey(0); // 更新右兄弟索引
        // 若借之前，节点为空，则还需更新当前节点的索引
        if(node->size == 1){
            change_index(node, path);
        }
            
    }

    /********* 不能借则尝试：合并 ********/
    else {
        // 尝试合并到左兄弟
        if (index > 0) {  
            BPlusNode *left_sibling = parent->getChild(index - 1);
            left_sibling->keys.insert(left_sibling->keys.end(), node->keys.begin(), node->keys.end());
            left_sibling->values.insert(left_sibling->values.end(), node->values.begin(), node->values.end());
            left_sibling->size += node->size;
            left_sibling->next_leaf = node->next_leaf;
            parent->keys.erase(parent->keys.begin() + index - 1);
            parent->children.erase(parent->children.begin() + index);
            parent->size--;

            delete node;   // 释放内存
        } 
        // 尝试把右兄弟合并过来
        else {    
            // 标记当前节点在合并之前是否为空，若是，则合并后需要修改索引
            bool need_change_index;
            if(node->size == 0)
                need_change_index = true;
            else
                need_change_index = false;

            BPlusNode *right_sibling = parent->getChild(index + 1);
            node->keys.insert(node->keys.end(), right_sibling->keys.begin(), right_sibling->keys.end());
            node->values.insert(node->values.end(), right_sibling->values.begin(), right_sibling->values.end());
            node->size += right_sibling->size;
            node->next_leaf = right_sibling->next_leaf;
            parent->keys.erase(parent->keys.begin() + index);
            parent->children.erase(parent->children.begin() + index + 1);
            parent->size--;
            delete right_sibling;   // 释放内存

            if(need_change_index) {
                change_index(node, path);
            }
        }

        // parent是根节点，且被删空：更新根结点
        if(parent == root){
            if(parent->size == 0)
                root = node;
        }
        // parent不是根结点，且触发内部节点下溢
        else if (parent->size < nonleaf_min_degree-1) {
            adjustNonLeafNode(path);
        }
    }
}

// 调整内部节点
void BPlusTree::adjustNonLeafNode(vector<BPlusNode*> path) {
    BPlusNode *node = path.back();  // 发生下溢的内部节点
    path.pop_back();
    BPlusNode *parent = path.back();    // 内部节点的父节点
    int index = child_index(parent, node);

    /****************  首先尝试：借  ****************/
    // 对于内部节点，借键实际上是借一个键的位置（借来后再确定键中的索引值），以及目标键对应的孩子（子树）
    // 尝试从左兄弟节点中借一个键 
    if (index > 0 && parent->getChild(index - 1)->getSize() > nonleaf_min_degree-1) {
        BPlusNode *left_sibling = parent->getChild(index - 1);
        // 借来的键的位置中要放的索引值是：借之前以该节点最左子树的最小值，可以在上一层索引找到
        node->keys.insert(node->keys.begin(), parent->getKey(index - 1));
        // 节点的上一层索引改为借来的孩子中的最小值，可在借的key的旧值找到
        parent->keys[index - 1] = left_sibling->getKey(left_sibling->getSize() - 1);
        left_sibling->keys.pop_back();
        node->children.insert(node->children.begin(), left_sibling->getChild(left_sibling->getSize()));
        left_sibling->children.pop_back();
        left_sibling->size--;
        node->size++;
    }
    // 尝试从右兄弟节点中借一个键
    else if (index < parent->getSize() && parent->getChild(index + 1)->getSize() > nonleaf_min_degree-1) {
        BPlusNode *right_sibling = parent->getChild(index + 1);
        // 同上：借过来的索引要改（改成借来的孩子中的最小值），右兄弟上一层索引也要改（改成右兄弟借出的key值）
        node->keys.push_back(parent->getKey(index));
        parent->keys[index] = right_sibling->getKey(0);
        right_sibling->keys.erase(right_sibling->keys.begin());
        node->children.push_back(right_sibling->getChild(0));
        right_sibling->children.erase(right_sibling->children.begin());
        right_sibling->size--;
        node->size++; /////
    }
    /****************  不能借则尝试：合并  ****************/
    else {
        if (index > 0) {
            BPlusNode *left_sibling = parent->getChild(index - 1);
            left_sibling->keys.push_back(parent->getKey(index - 1));
            left_sibling->keys.insert(left_sibling->keys.end(), node->keys.begin(), node->keys.end());
            left_sibling->children.insert(left_sibling->children.end(), node->children.begin(), node->children.end());
            left_sibling->size += node->size + 1;
            parent->keys.erase(parent->keys.begin() + index - 1);
            parent->children.erase(parent->children.begin() + index);
            parent->size--;

            // 释放内存
            delete node;
            // 让node指针指向合并后的指针，因为过后若判断父节点为根结点且被删空，则需更新根结点为合并后的节点
            node = left_sibling;    
        } else {
            BPlusNode *right_sibling = parent->getChild(index + 1);
            node->keys.push_back(parent->getKey(index));
            node->keys.insert(node->keys.end(), right_sibling->keys.begin(), right_sibling->keys.end());
            node->children.insert(node->children.end(), right_sibling->children.begin(), right_sibling->children.end());
            node->size += right_sibling->size + 1;
            parent->keys.erase(parent->keys.begin() + index);
            parent->children.erase(parent->children.begin() + index + 1);
            parent->size--;

            // 释放内存
            delete right_sibling;
        }

        // 判断parent是不是根节点，且被删空：更新根结点，这时树的层数-1
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

// 判断节点是第几个孩子
int BPlusTree::child_index(BPlusNode *parent, BPlusNode *node)
{
    int i = 0;
    for(; i < parent->size+1 && node != parent->children[i]; i++);
    if(i < parent->size+1)
        return i;
    else    // node不是parent的孩子
        return -1;
}

// 往上改索引
void BPlusTree::change_index(BPlusNode *current_node, vector<BPlusNode*> path)
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


/*****************序列化与反序列化****************/
// 从文件读入数据建树
void BPlusTree::build_tree_from(string file_name)
{
    data_file = file_name;

    from_file.open(file_name, std::ios::in);

    // 文件存在
    if(from_file.is_open()){    
        // 文件不为空
        if(from_file.peek() != std::fstream::traits_type::eof()) {
            // 文件第一行为度数
            int degree;
            from_file >> degree;
            set_degree(degree);

            root = deserializeNodeFromFile();
            cout << "Successfully deserialized and built a B-plus tree from file: " << file_name << endl;  
        } 
        else
            cout << "Failed to deserialize: The file is empty." << endl;
    }
    else
        cout << "Failed to deserialize: The file does't exist." << endl;
        

    from_file.close();
    cout << "Degree of the tree: " << leaf_max_degree << endl;
}

void BPlusTree::save_to_file()
{
    to_file.open(data_file);

    // 树不为空
    if(root){
        // 文件第一行写入度
        to_file << leaf_max_degree << '\n';
        serializeNodeToFile(root);
    }
        

    cout << "Updation saved to file." << endl;
    to_file.close();
}

// 序列化B+树到文件
void BPlusTree::serializeNodeToFile(BPlusNode* node)
{
    // if(node == nullptr){
    //     cout << "function: serializeNodeToFile: failed, node is nullptr" << endl;
    //     return;
    // }

    // 第一行：是否叶子节点     第二行：节点大小
    int is_leaf = node->isLeaf() ? 1 : 0;
    to_file << is_leaf << '\n' << node->getSize() << '\n';
    // 第三行：keys 
    for(auto key : node->keys)
        to_file << key << " ";
    to_file << '\n';
    // 判断是否是叶子节点      
    if(is_leaf){    //  是，第四行写valus
        for(auto value : node->values)
            to_file << value << " ";
        to_file << '\n';
    }
    else{   // 不是：递归写入孩子节点
        for(auto child : node->children)
            serializeNodeToFile(child);
    }
}

// 从文件反序列化B+树到内存
BPlusNode* BPlusTree::deserializeNodeFromFile()
{
    BPlusNode *node = new BPlusNode();
    static BPlusNode *last_leaf = nullptr;

    int is_leaf, size;
    from_file >> is_leaf >> size;
    if(is_leaf)
        node->leaf = true;
    else    
        node->leaf = false;
    node->size = size;

    int key;
    for(int i = 0; i < size; i++){
        from_file >> key;
        node->keys.push_back(key);
    }

    // 是叶节点：读入values
    if(is_leaf){
        string value;
        for(int i = 0; i < size; i++){
            from_file >> value;
            node->values.push_back(value);
        }

        // 链入叶节点
        if(last_leaf){
            last_leaf->next_leaf = node;
        }
        last_leaf = node;
    }
    // 不是叶节点：递归读入各个孩子
    else{
        for(int i = 0; i <= size; i++)
            node->children.push_back(deserializeNodeFromFile());
    }

    return node;
}


/***************** 其他 ****************/
// 清空树，释放每一个节点的内存
void BPlusTree::clear_tree()
{
    if(!root)
        return;

    std::queue<BPlusNode*> Q;
    BPlusNode *p = nullptr;
    Q.push(root);

    while(!Q.empty()){
        p = Q.front();
        Q.pop();

        // 是内部节点，将孩子加入队列等待删除
        if(!p->isLeaf())
            for(int i = 0; i < p->getSize()+1; i++)
                Q.push(p->getChild(i));

        // 释放当前节点内存
        delete p;
    }

    root = nullptr;
    cout << "Deleted the whole tree and freed all the space." << endl;
}

// 验证当前树是否是B+树
bool BPlusTree::is_bplustree()
{
    if(!root)
        return true;

    BPlusNode *node = root;
    while(!node->isLeaf())
        node = node->getChild(0);

    int i = 0;
    while(node->next_leaf==nullptr || i < node->getSize()-1){
        if(i < node->getSize()-1){
            if(BPlusNode::cmpKeys(node->getKey(i), node->getKey(i+1)) >= 0)
                return false;
            i++;
        }
        else{
            if(BPlusNode::cmpKeys(node->getKey(i), node->next_leaf->getKey(0)) >= 0)
                return false;
            i = 0;
            node = node->next_leaf;
        }
    }

    return true;
}

void BPlusTree::verify()
{
    if(is_bplustree())
        cout << "Verification passed, it is a B+ tree. " << endl;
    else
        cout << "Verification failed, it is not a B+ tree. " << endl;
}





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

        if(p->isLeaf())
            cout << "|  " ;
        for(auto key: p->getKeys())
            cout << key << " ";
        if(p->isLeaf())
            cout << " |" ;
        else{
            for(int i = 0; i < p->getSize()+1; i++)
                Q.push(p->getChild(i));
            cout << endl;
        }
    }
    cout << '\n' << endl;
}