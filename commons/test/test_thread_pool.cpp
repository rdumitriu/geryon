#include <thread>
#include <vector>
#include <iostream>

#include "thread_pool.hpp"

using namespace std;
using namespace geryon::mt;



void testFunct(int i) {
    std::cout << "bound function, i = " << i << " from thread:" << std::this_thread::get_id() << std::endl << std::flush;
}


int main(int argn, const char * argv []) {
    std::function<void(int)> f(testFunct);

    QueuedThreadPool<int> tp(f, 2, 4);

    for(int i = 0; i < 20; ++i) {
        if(i % 10 == 0) {
            this_thread::sleep_for(std::chrono::seconds(1));
        }
        tp.execute(i);
    }

    return 0;
}
