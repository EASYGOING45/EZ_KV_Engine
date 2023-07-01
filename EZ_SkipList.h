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

    void set_value(V);

    // 线性数组保存指向不同级别的下一个节点的指针
    Node<K, V> **forward;

    int node_level;

private:
    K key;
    V value;
};

template <typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) // 构造函数
{
    // level是跳表的层数
    this->key = k;
    this->value = v;
    this->node_level = level;

    // level+1是因为0也是一层
    this->forward = new Node<K, V> *[level + 1];

    // 初始化forward数组为0
    memset(this->forward, 0, sizeof(Node<K, V> *) * (level + 1));
};

template <typename K, typename V> // 析构函数
Node<K, V>::~Node()
{
    delete[] forward; // 释放forward数组
};

template <typename K, typename V>
K Node<K, V>::get_key() const
{
    return this->key;
}

template <typename K, typename V>
V Node<K, V>::get_value() const
{
    return this->value;
}

template <typename K, typename V>
void Node<K, V>::set_value(V value)
{
    this->value = value;
}

// 实现跳表的类模板 Class template to implement skip list
template <typename K, typename V>
class SkipList
{
public:
    SkipList(int);
    ~SkipList();
    int get_random_level();             // 生成随机层数
    Node<K, V> *create_node(K, V, int); // 创建节点
    int insert_element(K, V);           // 插入节点
    void display_list();                // 打印跳表
    bool search_element(K, V);          // 查找节点
    void delete_element(K);             // 删除节点
    void dump_file();                   // 将跳表数据写入文件
    void load_file();                   // 从文件中读取跳表数据
    int size();                         // 返回跳表大小

private:
    void get_key_value_from_string(const std::string &str, std::string *key, std::string *value); // 从字符串中获取key和value
    bool is_valid_string(const std::string &str);                                                 // 判断字符串是否合法

private:
    // 跳表的最大层数
    int _max_level;

    // 跳表当前的层数
    int _skip_list_level;

    // 跳表的头指针
    Node<K, V> *_header;

    // 文件读写流
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    int _element_count; // 跳表元素个数
};