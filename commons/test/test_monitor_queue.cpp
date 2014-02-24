#include <thread>
#include <vector>
#include <iostream>

#include "monitor_queue.hpp"

using namespace std;
using namespace geryon::mt;

struct msg {
  int op;
  std::string message;

  msg() {}
  msg(int _op, const std::string & _message) : op(_op), message(_message) {}
};

std::mutex gmutex;
std::vector<std::string> results;

void t(int tid, MonitorQueue<msg> * const pq) {
    bool mustRun = true;
    while(mustRun) {
        msg m = pq->get();
        switch(m.op) {
            case 1:
                {
                    std::lock_guard<std::mutex> _(gmutex);
                    results.push_back(m.message);
                    std::cout << "tid " + std::to_string(tid) + " -- " + m.message << std::endl;
                }
                break;
            case 0:
                std::cout << "tid " + std::to_string(tid) + " -- EXIT!" << std::endl;
                mustRun = false;
                break;
        }
    }
}

void p(MonitorQueue<msg> * const pq) {
    pq->put(msg(1, "3rd msg"));
    std::cout << "one more" << std::endl;
}

void test1() {
    MonitorQueue<msg> mq(2);
    mq.put(msg(1, "message_1"));
    mq.put(msg(1, "message_2"));
    std::cout << "2 messages enqueued" << std::endl;

    std::thread t3(p, &mq);

    std::thread t1(t, 1, &mq);
    std::thread t2(t, 2, &mq);


    t3.join();

    mq.put(msg(0, "out-1"));
    mq.put(msg(0, "out-2"));

    t1.join();
    t2.join();


    if(results.size() != 3) {
        std::cout << "FAILED: size" << std::endl;
    } else {
        bool found = false;
        for(auto s : results) {
            if(s == "3rd msg") { found = true; break; }
        }
        if(!found) {
            std::cout << "FAILED: not found" << std::endl;
        }
    }
}

void test2() {
    MonitorQueue<msg> mq(2);
    mq.put(msg(1, "single_1"));
    std::cout << "1 message enqueued" << std::endl;

    std::thread t1(t, 1, &mq);
    std::thread t2(t, 2, &mq);

    mq.put(msg(0, "out-1"));
    mq.put(msg(0, "out-2"));

    t1.join();
    t2.join();
}

int main(int argn, const char * argv []) {

    test1();
    test2();

    return 0;
}
