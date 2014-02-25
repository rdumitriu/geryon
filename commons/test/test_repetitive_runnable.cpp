#include <thread>
#include <vector>
#include <iostream>

#include "repetitive_runnable.hpp"

using namespace std;
using namespace geryon::mt;


volatile int global_cycles;
volatile int second_global;


void testFunct(int i, int j) {
    std::cout << "some message, i = " << i << " j = " << j << std::endl;
    global_cycles++;
}

class Foo {
public:
    void operator()() {
        std::cout << "some message."<< std::endl;
        second_global++;
    }
};

int main(int argn, const char * argv []) {
    global_cycles = 0;
    second_global = 0;
    static const uint32_t cycle = 1;

    RepetitiveRunnable rr(cycle, testFunct, 12345, 54321);
    rr.start();
    cout << "Sleeping for 2 cycles + 1 sec" << endl;
    this_thread::sleep_for(std::chrono::seconds(cycle));
    {
        RepetitiveRunnable rr2(cycle, std::move(Foo()));
        rr2.start();
        this_thread::sleep_for(std::chrono::seconds(cycle + 1));
    }
    rr.stop();

    cout << "Global cycles:" << global_cycles << endl;
    if(global_cycles != 2 && global_cycles != 3) {
        cout << "FAILED ! (1)" << endl;
    }
    cout << "second_global = " << second_global << endl;
    if(second_global != 1) {
        cout << "FAILED ! (2)" << endl;
    }
    return 0;
}
