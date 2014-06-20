
#include <iostream>

#include "path_tree.hpp"
#include "log.hpp"


using namespace std;
using namespace geryon::server;

void test1() {
    PathTree<int> tree;

    tree.addNode("/app/test/node/1", 1);
    tree.addNode("/app/test/node/2", 2);
    tree.addNode("/app/test/node/another/3", 3);
    tree.addNode("/app/test/*", 4);
    tree.addNode("/app/test/node/*", 5);

    vector<int> ret = tree.getDataForPath("/app/test/node/another/3");
    if(ret.size() != 3) {
        LOG(geryon::util::Log::ERROR) << "FAILED: test1(3) size not correct!";
    } else {
        if(ret.at(0) != 4 && ret.at(1) != 5 && ret.at(2) != 3) {
            LOG(geryon::util::Log::ERROR) << "FAILED: test1(3) result not correct!";
        }
    }
    for(auto i : ret) {
        cout << "test1(3) value:" << i << endl;
    }
    ret = tree.getDataForPath("/app/test/node/somevalue"); //4, 5
    if(ret.size() != 2) {
        LOG(geryon::util::Log::ERROR) << "FAILED: test1(test/somevalue) size not correct!";
    } else {
        if(ret.at(0) != 4 && ret.at(1) != 5) {
            LOG(geryon::util::Log::ERROR) << "FAILED: test1(test/somevalue) result not correct!";
        }
    }
    for(auto i : ret) {
        cout << "test1(test/somevalue) value:" << i << endl;
    }
    ret = tree.getDataForPath("/app/test/somevalue"); //4
    if(ret.size() != 1) {
        LOG(geryon::util::Log::ERROR) << "FAILED: test1(somevalue) size not correct!";
    } else {
        if(ret.at(0) != 4) {
            LOG(geryon::util::Log::ERROR) << "FAILED: test1(somevalue) result not correct!";
        }
    }
    for(auto i : ret) {
        cout << "test1(somevalue) value:" << i << endl;
    }
}

int main(int argn, const char * argv []) {
    test1();
}
