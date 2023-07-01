/**
 * EZ_SkipList.h  2023-07-01
 * 基于跳表SkipList的Key-Value存储引擎
 */

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

#define STORE_FILE "store/dumpFile"

std::mutex mtx;              // mutex 互斥锁
std::string delimiter = ":"; // 分隔符

// 实现节点的类模板 Class template to implement node
template <typename K, typename V>
class Node
{
public:
    Node() {}
    Node(K k, V v, int);
    ~Node();
    K get_key() const;
    V get_value() const;

    void set)value(V);

    // 线性数组保存指向不同级别的下一个节点的指针
    Node<K, V> **forward;

    int node_level;

private:
    K key;
    V value;
};