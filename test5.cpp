#include <iostream>
#include <cstdlib>
#include "thread.h"
#include "cpu.h"
#include "mutex.h"

using std::cout;
using std::endl;

int milk = 0;
mutex m;
cv c;
int count = 0;
void funcB(void *a) {
    m.lock();
    cout << "funcB lock\n";
    count++;
    m.unlock();
    cout << "funcB signaling\n";

    c.broadcast();
}

void funcA(void *a) {
    m.lock(); 
    while(count == 0) {
        cout << intptr_t(a) << " Waiting\n";
        c.wait(m);
    }
    cout << intptr_t(a) << " Func\n";
    m.unlock();
    cout << "count: " << count << endl;
}
void boot_func(void *) {
    for(int i = 0; i < 4; ++i){
        printf("%s", "FuncA called\n");
        thread tA(funcA, (void*)1);
    }
    printf("%s", "FuncB called\n");
    thread tC(funcB, (void*)3);
    tC.yield();
    assert_interrupts_enabled();

    tC.join();
    assert_interrupts_enabled();
}
int main()
{
    cpu::boot(1, (thread_startfunc_t) boot_func, (void *) 100, false, false, 0);
}
