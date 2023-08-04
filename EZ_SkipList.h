/**
 * EZ_SkipList.h  2023-07-01
 2023-08-03 Restart
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

//Class Template for Skip list
//跳表类模板
template<typename K,typename V>
class SkipList{
public:
    SkipList(int);
    ~SkipList();
    int get_random_level();     //生成随机层数 用于跳表中的插入操作
    Node<K,V>* create_node(K,V,int);    //创建节点
    int insert_element(K,V);    //插入节点 用于跳表中的插入操作
    void display_list();        //显示跳表中的节点 用于跳表中的插入操作
    bool search_element(K);     //查找节点 用于跳表中的查找操作
    void delete_element(K);     //删除节点 用于跳表中的删除操作
    void dump_file();           //将跳表中的数据写入文件
    void load_file();           //从文件中读取数据到跳表中
    int size();                 //返回跳表中节点的个数 
private:
    void get_key_value_from_string(const std::string& str,std::string* key,std::string* value); //从字符串中获取key和value
    bool is_valid_string(const std::string& str); //判断字符串是否合法
private:
    //跳表的最大层数
    int _max_level;

    //跳表的当前层数 也就是最高层数为几
    int _skip_list_level;

    //跳表的头节点
    Node<K,V> *_header;

    //文件operator 
    std::ofstream _file_writer;     //写文件
    std::ifstream _file_reader;     //读文件

    //skiplist current element count 跳表中节点的个数
    int _element_count;
};


//create new node
template<typename K,typename V>
Node<K,V>* SkipList<K,V>::create_node(const K k,const V v,int level){
    Node<K,V> *n=new Node<K,V>(k,v,level);  //创建节点 level是跳表的层数
    return n;
}

//在跳表中插入给定的key和value
//return 1 表示 element exists 表示元素已经存在
//return 0 表示 element inserted successfully 表示元素插入成功

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

template<typename K,typename V>
int SkipList<K,V>::insert_element(const K key,const V value){
    mtx.lock();     //加锁 保证线程安全
    Node<K,V> *current=this->_header;   //从头节点开始

    //创建update数组并初始化为0 update数组用于保存要更新的节点
    //update is array which put node that the node->forward[i] should be operated later
    //update数组是一个指针数组，用于保存要更新的节点
    Node<K,V> *update[_max_level+1];    //update数组的大小为_max_level+1
    memset(update,0,sizeof(Node<K,V>*)*(_max_level+1)); //初始化update数组为0

    //从跳表的最高层开始查找要插入的位置
    for(int i=_skip_list_level;i>=0;i--){
        while(current->forward[i]!=NULL && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];  //向前查找 直到找到要插入的位置
        }
        update[i]=current;  //保存要更新的节点
    }

    //到达这里 current指向要插入的位置的前一个节点
    //reached level 0 and forward pointer to right node,which is desired to insert key
    current = current->forward[0];  //current指向要插入的位置的前一个节点的下一个节点

    //if current node have key equal to searched key,we get it
    //如果current节点的key等于要插入的key，那么就说明要插入的key已经存在
    if(current != NULL && current->get_key()==key){
        std::cout<<"key: "<<key<<",exists"<<std::endl;
        mtx.unlock();   //解锁
        return 1;
    }

    //if current is NULL that means we have reached to end of the level
    //如果current为NULL，说明要插入的key比跳表中的所有key都大，那么就将跳表的层数加1
    //if current's key is not equal to key that means we have to insert node between update[0] and current node 
    //如果current节点的key不等于要插入的key，那么就要插入一个新的节点
    if(current == NULL || current->get_key() != key){
        //Generate a random level for node 生成一个随机层数
        int random_level = get_random_level();

        //If random level is greater thar skip list's current level, initialize update value with pointer to header
        //如果随机层数大于跳表的当前层数，那么就将update数组中的值初始化为头节点
        if(random_level > _skip_list_level){
            for(int i=_skip_list_level+1;i<random_level+1;i++){
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        //create new node with random level generated 创建一个新的节点 该节点的层数为随机层数
        Node<K,V>* inserted_node = create_node(key,value,random_level); //创建一个新的节点

        //insert node by rearranging pointers 将新节点插入到跳表中
        for(int i=0;i<=random_level;i++){
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout<<"Successfully inserted key:"<<key<<", value:"<<value<<std::endl;
        _element_count++;   //跳表中节点的个数加1
    }
    mtx.unlock();   //解锁
    return 0;       //插入成功
}

//输出展示跳表 Display skip list
template<typename K,typename V>
void SkipList<K,V>::display_list(){
    std::cout<<"\n*****Skip List*****"<<"\n";
    for(int i=0;i<=_skip_list_level;i++){
        Node<K,V> *node=this->_header->forward[i];
        std::cout<<"Level "<<i<<": ";
        while(node != NULL){
            std::cout<<node->get_key()<<":"<<node->get_value()<<";";
            node = node->forward[i];
        }
        std::cout<<std::endl;
    }
}

//Dump data in memory to file 将内存中的数据写入到文件中
template<typename K,typename V>
void SkipList<K,V>::dump_file(){
    std::cout<<"dump_file-----------"<<std::endl;
    _file_writer.open(STORE_FILE);
    Node<K,V> *node = this->_header->forward[0];    //从头节点开始

    while(node != NULL){
        _file_writer<<node->get_key()<<":"<<node->get_value()<<"\n";
        std::cout<<node->get_key()<<":"<<node->get_value()<<";\n";
        node = node->forward[0];
    }

    _file_writer.flush();   //刷新缓冲区 flush buffer
    _file_writer.close();   //关闭文件
    return ;
}

//Load data from file to memory 从文件中加载数据到内存中
template<typename K,typename V>
void SkipList<K,V>::load_file(){
    _file_reader.open(STORE_FILE);
    std::cout<<"load_file--------------"<<std::endl;
    std::string line;
    std::string* key=new std::string();
    std::string* value=new std::string();
    while(getline(_file_reader,line)){
        get_key_value_from_string(line,key,value);
        if(key->empty() || value->empty()){
            continue;
        }
        insert_element(*key,*value);    //将数据插入到跳表中
        std::cout<<"key:"<<*key<<"value:"<<*value<<std::endl;
    }
    delete key;     //释放内存
    delete value;   //释放内存
    _file_reader.close();   //关闭文件
}

//Get current SkipList size 获取跳表的大小
template<typename K,typename V>
int SkipList<K,V>::size(){
    return _element_count;
}

template<typename K,typename V>
void SkipList<K,V>::get_key_value_from_string(const std::string& str,std::string* key,std::string* value){
    if(!is_valid_string(str)){
        return ;
    }
    *key = str.substr(0,str.find(delimiter));   //获取key
    *value=str.substr(str.find(delimiter)+1,str.length());  //获取value  substr(pos,n) 返回从pos开始的n个字符
}

//NEXT_DAY IS_VALID_STRING
template<typename K,typename V>
bool SkipList<K,V>::is_valid_string(const std::string& str){    //判断字符串是否为空
    if(str.empty()){
        return false;
    }
    if(str.find(delimiter) == std::string::npos){
        return false;       //没有找到分隔符 npos表示没有找到
    }
    return true;
}

//Delete element from skip list
template<typename K,typename V>
void SkipList<K,V>::delete_element(K key){
    mtx.lock();     //加锁
    Node<K,V> *current = this->_header; //从头节点开始
    Node<K,V> *update[_max_level+1];    //update数组
    memset(update,0,sizeof(Node<K,V>*)*(_max_level+1));   //初始化update数组

    //start from highest level of skip list 从跳表的最高层开始
    for(int i=_skip_list_level;i>=0;i--){
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
        update[i] = current;
    }

    curret = current->forward[0];   //当前节点
    if(current!=NULL && current->get_key()==key){
        //从最低层开始删除节点并重新连接指针
        for(int i=0;i<=_skip_list_level;i++){
            //如果在第i层中找到了key，那么就将第i层中的节点删除
            if(update[i]->forward[i]!=current){
                break;
            }

            update[i]->forward[i] = current->forward[i];
        }

        //删除跳表中的层数 Remove levels having no elements
        while(_skip_list_level > 0 && _header->forward[_skip_list_level]==0){
            _skip_list_level--;
        }

        std::cout<<"Successfully deleted key:"<<key<<std::endl;
        _element_count--;   //跳表中节点的个数减1
    }
    mtx.unlock();   //解锁
    return ;
}

//Search
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template<typename K,typename V>
bool SkipList<K,V>::search_element(K key){
    std::cout<<"search_element-----------"<<std::endl;
    Node<K,V> *current = _header;   //从头节点开始
    //start from highest level of skip list 从跳表的最高层开始
    for(int i=_skip_list_level;i>=0;i--){
        while(current->forward[i]&&current->forward[i]->get_key()<key){
            current = current->forward[i];  //向右移动
        }
    }

    //reached level 0 and advance pointer to right,which is possibly our desired node 到达第0层并且向右移动指针，这可能是我们想要的节点
    //到达第0层并且向右移动指针，这可能是我们想要的节点
    current = current->forward[0];

    //如果current节点的key等于要查找的key，那么就返回true
    if(current and current->get_key() == key){
        std::cout<<"Found key: "<<key<<", value: "<<current->get_value()<<std::endl;
        return true;
    }

    std::cout<<"Not Found Key:"<<key<<std::endl;
    return false;
}


//Construct skip list 初始化跳表
template<typename K,typename V>
SkipList<K,V>::SkipList(int max_level){
    this->max_level = max_level;    //跳表的最大层数
    this->_skip_list_level = 0;    //跳表的当前层数
    this->_element_count = 0;      //跳表中节点的个数

    //create header node and initialize key and value to null 创建头节点并将key和value初始化为null
    K k;
    V v;
    this->header = new Node<K,V>(k,v,_max_level);   //创建头节点
};


//析构函数
template<typename K,typename V>
SkipList<K,V>::~SkipList(){
    if (_file_writer.is_open()) {
        _file_writer.close();
    }
    if (_file_reader.is_open()) {
        _file_reader.close();
    }
    delete _header;
}


//NEXT=>get_random_level
template<typename K,typename V>
int SkipList<K,V>::get_random_level(){
    int k = 1;
    while(rand() % 2){
        k++;
    }

    k = (k < _max_level) ? k : _max_level;
    return k;
};