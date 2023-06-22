#include <iostream>
#include <cstdlib>
#include "thread.h"
using std::cout;
using std::endl;

mutex m;


void funcA(void* a) {
    m.lock();
    m.lock();
    cout << "hello'\n";
    m.unlock();
    cout << "He\n";
}
void boot_func(void*) {
    
    cout << "T4\n";
    thread tA(funcA, (void*)1);

}
int main()
{
    //throw std::bad_alloc();
    cout << "T1\n";
    cpu::boot(1, (thread_startfunc_t) boot_func, (void *) 100, false, false, 0);
}
