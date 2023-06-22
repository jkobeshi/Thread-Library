#include <iostream>
#include <cstdlib>
#include "thread.h"
#include "cpu.h"
#include "mutex.h"

using std::cout;
using std::endl;

mutex m;


void funcA(void *a) {
    m.lock();
    //cout << "hello'\n";
    m.unlock();
}
void boot_func(void *) {
    cout << "T2\n";
    int i = 0;
    try
    {
        cout << "T3\n";
        while(i < 351){
                cout << "T4\n";
                thread tA(funcA, (void*)1);
                ++i;
        }
        if (i == 351) {
            thread tB(funcA, (void*)1);
        }
    }
    catch(const std::bad_alloc& ba)
    {
        cout << i << endl;
        assert_interrupts_enabled();
       cout << "caught\n";
    }
    cout << "finished\n";
}
int main()
{
    //throw std::bad_alloc();
    cout << "T1\n";
    cpu::boot(1, (thread_startfunc_t) boot_func, (void *) 100, false, false, 0);
}
