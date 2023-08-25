#include "tree.h"
#include "bpt_test.h"

void test(BPlusTree &bpt)
{
    test_bplustree(bpt, 4, true);
    cout << "-------------------------------------------------------" << endl;

    for(int i = 10; i < 200; i += 10){
        test_bplustree(bpt, i, true);
        cout << "-------------------------------------------------------" << endl;
    }

    test_bplustree(bpt, 200, false);
}

int main(int argc, char **argv)
{
    BPlusTree bpt;
    bool updated = false;   // 记录有无更新
    
    bpt.build_tree_from("data.txt"); // 反序列化建树

    test_insertion(bpt, 1000000, true);
    test_deletion(bpt, 10000);

    if(updated)
        bpt.save_to_file();

    return 0;
}