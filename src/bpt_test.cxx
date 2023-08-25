#include "bpt_test.h"


// 测试：插入
int test_insertion(BPlusTree &bpt, int numInsertions, bool is_random)
{
    auto startInsert = std::chrono::high_resolution_clock::now();
    if(is_random){
        std::vector<int> sequence;
        for (int i = 1; i <= numInsertions; ++i) {
            sequence.push_back(i);
        }

        // 使用随机数引擎和洗牌函数来随机排序序列
        std::random_device rd;
        std::mt19937 rng(rd());  // 随机数引擎
        std::shuffle(sequence.begin(), sequence.end(), rng);

        cout << "Test running: Insertion in random order: Size of " << numInsertions << endl;

        startInsert = std::chrono::high_resolution_clock::now();

        for(auto key : sequence)
            bpt.insertKeyValue(key, "V" + std::to_string(key));
    }
    else{
        cout << "Test running: Insertion in sequential order: Size of " << numInsertions << endl;
        
        startInsert = std::chrono::high_resolution_clock::now();

        for (int i = 1; i <= numInsertions; ++i) {
            bpt.insertKeyValue(i, "V" + std::to_string(i));
        }        
    }


    auto endInsert = std::chrono::high_resolution_clock::now();
    auto durationInsert = std::chrono::duration_cast<std::chrono::milliseconds>(endInsert - startInsert).count();
    std::cout << "Insertions of " << numInsertions << " records completed in: " << durationInsert << " ms" << std::endl;
    cout << "Average time consuming:" << (double)durationInsert/numInsertions << " ms" << endl;
    return durationInsert;
}

// 测试：查找
double test_search(BPlusTree &bpt, int num)
{
    cout << "Test running: Search: ";

    value_type v;
    auto startInsert = std::chrono::high_resolution_clock::now();
    
    for(int i = 1; i <= num; i++)
        bpt.searchKeyValue(i, v);

    auto endInsert = std::chrono::high_resolution_clock::now();
    auto durationInsert = std::chrono::duration_cast<std::chrono::milliseconds>(endInsert - startInsert).count();
    //std::cout << "Searching for " << num << " records completed in: " << durationInsert << " ms" << std::endl;
    cout << "Average time consuming:" << (double)durationInsert/num << " ms" << endl;
    return (double)durationInsert/num;
}

// 测试：删除
double test_deletion(BPlusTree &bpt, int num)
{
    cout << "Test running: Deletion: ";

    auto startInsert = std::chrono::high_resolution_clock::now();
    
    for(int i = 1; i <= num; i++)
        bpt.deleteKeyValue(i);

    auto endInsert = std::chrono::high_resolution_clock::now();
    auto durationInsert = std::chrono::duration_cast<std::chrono::milliseconds>(endInsert - startInsert).count();
    //std::cout << "Deletion of " << num << " records completed in: " << durationInsert << " ms" << std::endl;
    cout << "Average time consuming:" << (double)durationInsert/num << " ms" << endl;
    return (double)durationInsert/num;
}


// 测试：序列化
int test_serialization(BPlusTree &bpt)
{
    cout << "Test running: Serialization: ";
    auto startInsert = std::chrono::high_resolution_clock::now();

    bpt.save_to_file();

    auto endInsert = std::chrono::high_resolution_clock::now();
    auto durationInsert = std::chrono::duration_cast<std::chrono::milliseconds>(endInsert - startInsert).count();
    std::cout << "Time consumming: " << durationInsert << " ms" << std::endl;
    return durationInsert;
}

// 测试：反序列化
int test_deserialization(BPlusTree &bpt, string file_name)
{
    cout << "Test running: Deserialization: ";
    auto startInsert = std::chrono::high_resolution_clock::now();

    bpt.build_tree_from(file_name);

    auto endInsert = std::chrono::high_resolution_clock::now();
    auto durationInsert = std::chrono::duration_cast<std::chrono::milliseconds>(endInsert - startInsert).count();
    std::cout << "Time consumming: " << durationInsert << " ms" << std::endl;
    return durationInsert;
}


// 自动化测试
void test_bplustree(BPlusTree &bpt, int degree, bool clear)
{
    static vector<int> insert_time;
    static vector<double> search_time, delete_time;

    if(bpt.getDegree() != degree){
        //cout << "Tree's degree: " << bpt.getDegree() << "; To-be-set degree: " << degree << endl; 
        bpt.set_degree(degree);
        //cout << "Tree's degree: " << bpt.getDegree() << endl;
    }
        

    insert_time.push_back(test_insertion(bpt, 10000000, true));
    cout << '\n';
    search_time.push_back(test_search(bpt, 10000));
    delete_time.push_back(test_deletion(bpt, 10000));

    if(clear)
        bpt.clear_tree();
    else{
        int i = 0;
        for(auto t1 : insert_time){
            cout << insert_time[i] << " " << search_time[i] << " " << delete_time[i] << endl;
            i++;
        }
    }
    cout << '\n';
}