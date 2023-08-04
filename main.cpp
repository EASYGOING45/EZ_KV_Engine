/***
* Main file for the program
*/

#include<iostream>
#include"EZ_SkipList.h"
#define FILE_PATH "./store/dumpFile"

int main(){
    //键值对中的key用int型，如果使用其他类型，则需要自定义比较函数
    //而且如果修改key的类型，同事需要修改skipList.load_file函数
    SkipList<int,std::string> skipList(6);
    skipList.insert_element(1,"你好");
    skipList.insert_element(3,"这里是");
    skipList.insert_element(7,"EASYGOING45");
    skipList.insert_element(8,"的跳表");
    skipList.insert_element(9,"测试程序");
    skipList.insert_element(19,"LYX");
    skipList.insert_element(19,"C++");

    std::cout<<"skipList size:"<<skipList.size()<<std::endl;

    skipList.dump_file();

    //skipList.load_file();
    skipList.search_element(9);
    skipList.search_element(18);

    skipList.display_list();

    skipList.delete_element(3);
    skipList.delete_element(7);

    std::cout<<"skipList size:"<<skipList.size()<<std::endl;

    skipList.display_list();

}