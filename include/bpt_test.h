#ifndef __BPT_TEST_H__
#define __BPT_TEST_H__

#include "tree.h"

int test_insertion(BPlusTree &bpt, int numInsertions, bool is_random);
double test_search(BPlusTree &bpt, int num);
double test_deletion(BPlusTree &bpt, int num);
int test_serialization(BPlusTree &bpt);
int test_deserialization(BPlusTree &bpt, string file_name);
void test_bplustree(BPlusTree &bpt, int degree, bool clear);

#endif