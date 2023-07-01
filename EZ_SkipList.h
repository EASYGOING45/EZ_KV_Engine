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

// 创建新节点
template <typename K, typename V>
Node<K, V> *SkipList<K, V>::create_node(const K k, const V v, int level)
{
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
}

// Insert given key and value in skip list
// return 1 means element exists
// return 0 means insert successfully
/*
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+

*/

template <typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value)
{
    mtx.lock();
    Node<K, V> *current = this->_header;

    // 创建新链表并初始化
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1)); // 初始化update数组为0

    // 从最高层开始查找要插入的位置
    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
        update[i] = current;
    }

    // 到达第0层并且右边的节点的key等于要插入的key
    current = current->forward[0];

    // 如果当前节点的key等于要插入的key，说明跳表中已经存在要插入的key，直接返回
    if (current != NULL && current->get_key() == key)
    {
        std::cout << "Key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }

    // if current is NULL that means we have reached to end of the level
    // if current's key is not equal to key that means we have to insert node between update[0] and current node
    if (current == NULL || current->get_key() != key)
    {

        // Generate a random level for node
        int random_level = get_random_level();

        // If random level is greater thar skip list's current level, initialize update value with pointer to header
        if (random_level > _skip_list_level)
        {
            for (int i = _skip_list_level + 1; i < random_level + 1; i++)
            {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        // create new node with random level generated
        Node<K, V> *inserted_node = create_node(key, value, random_level);

        // insert node
        for (int i = 0; i <= random_level; i++)
        {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        _element_count++;
    }
    mtx.unlock();
    return 0;
}

// Tag-1
// 打印跳表
template <typename K, typename V>
void SkipList<K, V>::display_list()
{
    std::cout << "\n*****Skip List*****"
              << "\n";
    for (int i = 0; i <= _skip_list_level; i++)
    {
        Node<K, V> *node = this->_header->forward[i];
        std::cout << "Level " << i << ": ";
        while (node != NULL)
        {
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

// Dump data in memory to file
template <typename K, typename V>
void SkipList<K, V>::dump_file()
{
    std::cout << "dump_file------------------" << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forwardd[0];

    while (node != NULL)
    {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }

    _file_writer.flush(); // 刷新缓冲区
    _file_writer.close(); // 关闭缓冲区
    return;
}

// 从磁盘加载数据
template <typename K, typename V>
void SkipList<K, V>::load_file()
{
    _file_reader.open(STORE_FILE);
    std::cout << "load_file-----------------" << std::endl;
    std::string line;
    std::string *key = new std::string();
    std::string *value = new std::string();
    while (getline(_file_reader, line))
    {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty())
        {
            std::cout << "key or value is empty" << std::endl;
            continue;
        }
        insert_element(*key, *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    _file_reader.close();
}

// 获取当前跳表的大小
template <typename K, typename V>
int SkipList<K, V>::size()
{
    return _element_count;
}

// 从字符串中获取key和value
template <typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string &str, std::string *key, std::string *value)
{
    if (!is_valid_string(str))
    {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

// 判断字符串是否合法
template <typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string &str)
{
    if (str.empty())
    {
        return false;
    }
    if (str.find(delimiter) == std::string::npos)
    {
        return false;
    }
    return true;
}